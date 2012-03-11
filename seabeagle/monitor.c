#include "zhelpers.h"
#include "monitor.h"
#include "config.h"
#include "czmq.h"

/*
typedef struct {
  int next_send;
  int deadline;
  char * channel;
} monitor_t;
*/

void watch_port(void *cvoid, 
                zctx_t * context, void * pipe ) {
  zclock_log("watch_port started!");
  config_t * config = (config_t*) cvoid;
  // sort out comms with the overlord first
  char * channel = s_recv(pipe);
  
  zclock_log(channel);
  s_send(pipe, "ok");

  void * portwatcher = zsocket_new(config->context, ZMQ_PUB);
  zmq_connect(portwatcher, config->portwatcher_endpoint);
  void * control = zsocket_new(config->context, ZMQ_SUB);
  // think it should be possible to automatically ignore
  // everything we don't know about.
  zsockopt_set_subscribe(control, "CHANNEL_CHANGE");
  zsockopt_set_subscribe(control, "VALUE");
  zsocket_connect(context, "inproc://monitor_controller");
  while(1) {
    zmsg_t * msg = zmsg_recv(control);
    zframe_t * cmd = zmsg_pop(msg);
    if(zframe_streq(cmd, "CHANNEL_CHANGE") || 
       zframe_streq(cmd, "CLEAR_MONITORS")) {
      assert(zmsg_size(msg) == 0);
      // don't really care what we've changed to...
      return;
    } else if (zframe_streq(cmd, "VALUE")) {
      assert(zmsg_size(msg) == 1);
      char * value = zmsg_popstr(msg);
      // let's just send everything for now. TODO, maybe
      //      if (fire_monitor(monitor)) {
      s_sendmore(portwatcher, config->identity);
      s_sendmore(portwatcher, channel);
      s_send(portwatcher, value);
    }
    // else ignore
    zmsg_destroy(&msg);
    zframe_destroy(&cmd);
  }
}
