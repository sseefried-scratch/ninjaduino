#include "line.h"
#include "cJSON.h"
#include "line_dispatcher.h"
#include "config.h"
#include <czmq.h>
#include "utils.h"

/* line dispatcher pulls in json and pulls it apart,
   dispatching to a pub socket with an appropriate topic (lineXXXX) */

void line_dispatcher(void * cvoid, zctx_t * context, void * pipe) {
  // zmq_pollitem_t  * items = malloc(1 * sizeof(zmq_pollitem_t));
  // config_t * config = (config_t*) cvoid;
  //  assert(cvoid==NULL);
  zclock_log("line dispatcher in");
  void * serial = zsocket_new(context, ZMQ_SUB);
  zsocket_connect(serial, "inproc://raw_serial");
  zclock_log("line dispatcher connected");
  // apparently we still need to subscribe to the empty string...
  
  zsockopt_set_subscribe(serial, "");
  int i;
  int lines = 0;

  void * events = zsocket_new(context, ZMQ_PUB);
  zsocket_bind(events, "inproc://line");
  zclock_log("child handshake...");
  child_handshake(pipe);
  zclock_log("child handshake");
  zmq_pollitem_t items [] = {
    { pipe, 0, ZMQ_POLLIN, 0 },
    { serial, 0, ZMQ_POLLIN, 0 }
  };


  while(1) {
    zclock_log("child waiting for data");
    int no_messages = zmq_poll(items, 2, -1);
    // let's assume for the moment that we have only one message
    // waiting. i _think_ this is incorrect, but not sure how to use
    // zmq_poll correctly.
    if (items[0].revents & ZMQ_POLLIN) {
      char * cmd = zstr_recv(pipe);
      if (strcmp(cmd, "DESTROY") == 0) {
        zclock_log("shutting down line dispatcher");
        // should also shut down clients
        zstr_send(events, "DESTROY");
        zstr_send(pipe, "OK");
        free(cmd);
        break;
      } else {
        zclock_log("Don't understand command %s, ignoring", cmd);
      }
    }
    if (items[1].revents & ZMQ_POLLIN) {
      zmsg_t *msg = zmsg_recv(serial);
      zclock_log("child got data");
      if(!msg) {
        // interrupted. or something.
        zclock_log("filter quitting!");
        return;
      }
      assert(zmsg_size(msg) == 1);
      char * data = zmsg_popstr(msg);
      zclock_log(data);
      // ephemeral storage, so we'll copy what we want out of
      // the json structure
      cJSON * root = cJSON_Parse(data);
      
      if (!root) {
        // our feed is not clean json.
        zclock_log("Bad json, ignoring:|%s|", data);
        free(data);
        zmsg_destroy(&msg);
        continue;
      }
      free(data);
      cJSON * port = cJSON_GetObjectItem(root, "ports")->child;
      
      for(i=1; port; i++) {
        char * channel = cJSON_GetObjectItem(port, "type")->valuestring;
        char * tmp = channel;
        while(*tmp) {
          *tmp = tolower(*tmp);
          tmp++;
        }
        //zclock_log("filter\ntype is %s\n", type);
        assert(channel);
      
        // this may be dodgy: some values may be structured.
        int value = cJSON_GetObjectItem(port, "value")->valueint;
      // zclock_log("filter\nvalue is %d\n", value);
        assert(value >= 0);
        assert(value <= 1024);
        
        zmsg_t * out = zmsg_new();
      // value needs to be an int here FIX
        int * vcopy = malloc(sizeof(int));
        *vcopy = value;
        zmsg_pushmem(out, vcopy, sizeof(int)); 
        zmsg_pushstr(out, channel);
        char * line = to_line(i);
        zmsg_pushstr(out,line);
        zclock_log("sending...");
        zmsg_dump(out);
        zmsg_send(&out, events);

        port = port->next;
      }
      cJSON_Delete(root);
      zmsg_destroy(&msg);
    }
  }
}
