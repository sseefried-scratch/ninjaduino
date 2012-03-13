#include "line.h"
#include "cJSON.h"
#include "filter.h"
#include "config.h"
#include <czmq.h>
#include "utils.h"



void line_dispatcher(void * cvoid, zctx_t * context, void * pipe) {
  // zmq_pollitem_t  * items = malloc(1 * sizeof(zmq_pollitem_t));
  // config_t * config = (config_t*) cvoid;
  void * serial = zsocket_new(context, ZMQ_SUB);
  zsocket_connect(serial, "inproc://raw_serial");
  // unnecessary: we start with an empty subscription.
  // zmq_setsockopt (serial, ZMQ_SUBSCRIBE, "", 1);
  zmq_msg_t msg;
  zmq_msg_init(&msg);
  int i;
  int lines = 0;

  void * events = zsocket_new(context, ZMQ_PUB);
  zsocket_bind(events, "inproc://serial_events");
  while(1) {
    zmq_recv(serial, &msg, 0);
    // ephemeral storage, but we'll copy what we want out of
    // the json sturcture.
    cJSON * root = cJSON_Parse(zmq_msg_data(&msg));
    if (!root) {
      // our feed is not clean json.
      continue;
    }
    cJSON * port = cJSON_GetObjectItem(root, "ports")->child;

    for(i=1; port; i++) {
      char line[16];
      char * type = cJSON_GetObjectItem(port, "type")->valuestring;
      zclock_log("filter\ntype is %s\n", type);
      assert(type);
      // this may be dodgy: some values may be structured.
      int value = cJSON_GetObjectItem(port, "value")->valueint;
      zclock_log("filter\nvalue is %d\n", value);
      assert(value >= 0);
      assert(value <= 1024);
      // this is pretty sketchy, but how else do we indicate that
      // the line hasn't been initialised yet?
      sprintf(line, "line%04d", i);
      if ((lines & (0x1 << (i-1))) == 0) {
        // line listener owns the config
        lineconfig_t * lineconfig = malloc(sizeof(lineconfig_t));
        lineconfig->inpipe = "line";
        lineconfig->outpipe = strdup(line);
        lineconfig->topic = lineconfig->outpipe; // I GUESS? TODO
        void * pipe = zthread_fork(context, line_listener, (void*)lineconfig);
        parent_handshake(pipe);
        zsocket_destroy(context, pipe);
        lines |=  0x1<<(i-1) ;
      }
      zmsg_t * out = zmsg_new();
      // value needs to be an int here FIX
      int * vcopy = malloc(sizeof(int));
      *vcopy = value;
      zmsg_pushmem(out, vcopy, sizeof(int)); 
      zmsg_pushstr(out, type);
      zmsg_pushstr(out,line);
      zmsg_send(&out, events);

      port = port->next;
    }
    cJSON_Delete(root);
  }
}
