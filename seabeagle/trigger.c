#include "monitor.h"
#include "config.h"
#include "czmq.h"
#include "trigger.h"
#include "utils.h"
/*
typedef struct {
  int next_send;
  int deadline;
  char * channel;
} monitor_t;
*/

void trigger(void *cvoid, 
             zctx_t * context, 
             void * pipe ) {
  zclock_log("watch_port started!");
  triggerconfig_t * config = (triggerconfig_t*) cvoid;

  void * control = zsocket_new(context, ZMQ_SUB);
  // think it should be possible to automatically ignore
  // everything we don't know about.
  zsockopt_set_unsubscribe(control, "");
  zsockopt_set_subscribe(control, "CHANNEL_CHANGE");
  zsockopt_set_subscribe(control, "VALUE");
  zsocket_connect(context, "inproc://monitor_controller");
  // sort out comms with the overlord
  child_handshake(pipe);
  zsocket_destroy(context, pipe);

  while(1) {
    zmsg_t * msg = zmsg_recv(control);
    zframe_t * cmd = zmsg_pop(msg);
    if(zframe_streq(cmd, "CHANNEL_CHANGE")) {
      // TODO
    } else if (zframe_streq(cmd, "VALUE")) {
      char * value = zmsg_popstr(msg);
      // TODO what does a trigger do?

    }
    // else ignore
    zmsg_destroy(&msg);
    zframe_destroy(&cmd);
  }
}
