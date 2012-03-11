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

void trigger(config_t *config, 
             zctx_t * context, 
             void * pipe ) {
  zclock_log("watch_port started!");
  // sort out comms with the overlord first
  char * channel = s_recv(pipe);
  //TODO receive all other necessary information
  zclock_log(channel);
  s_send(pipe, "ok");

  void * control = zsocket_new(config->context, ZMQ_SUB);
  // think it should be possible to automatically ignore
  // everything we don't know about.
  zsockopt_set_subscribe(control, "CHANNEL_CHANGE");
  zsockopt_set_subscribe(control, "VALUE");
  zsocket_connect(context, "inproc://monitor_controller");

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
