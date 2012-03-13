#include "line.h"
#include "czmq.h"
#include "monitor.h"
#include "trigger.h"
#include "utils.h"


/* 
outgoing messages to monitors
__________________
| CHANNEL_CHANGE |
|________________|
| new channel    |
|________________|
________________
| VALUE        |
|______________|
| actual value |
|______________|
*/

int LIMIT = 3;
typedef struct {
  char * current_channel;
  char * next_channel;
  int confirmed_rounds;
} channel_memory_t;

int port_changed(zframe_t * channel, channel_memory_t * m) {
  if (zframe_streq(channel, m->current_channel)) {
    return 0;
  }
  if (zframe_streq(channel, m->next_channel)) {
    if(m->confirmed_rounds++ > LIMIT) {
      // time to change
      free(m->next_channel);
      free(m->current_channel);
      m->next_channel = NULL;
      m->confirmed_rounds=0;
      m->current_channel = zframe_strdup(channel);
      return 1;
    }
    return 0;
  }
  // new channel that we weren't expecting.
  free(m->next_channel);
  m->next_channel = zframe_strdup(channel);
  return 0;
}


void line_listener(void * cvoid, zctx_t * context, void * pipe) {
  lineconfig_t * config = (lineconfig_t*)  cvoid;
  zmsg_t * msg;

  channel_memory_t channel_memory = { NULL, NULL, 0 };
  // void * monitor_controller = zsocket_new(config->context, ZMQ_PUB);
  //  zsocket_bind(monitor_controller, "inproc://monitor_controller");
  //  int trigger_capacity = 1;
  
  void * lineout = zsocket_new(context, ZMQ_PUB);
  zsocket_bind(lineout, config->outpipe);

  void * subscriber = zsocket_new(context, ZMQ_SUB);
  zsocket_connect(subscriber, "inproc://serial_events");
  zsockopt_set_subscribe(subscriber, config->topic);
  child_handshake(pipe);
  zsocket_destroy(context, pipe);
  while(1) {
    msg = zmsg_recv(subscriber);
    zframe_t * recv_topic = zmsg_pop(msg);
    assert(zframe_streq(recv_topic, config->topic));
    zframe_destroy(&recv_topic);
    /* zframe_t * cmd = zmsg_pop(msg); */

    /* // this is probably a bit over-keen - we could just send a string */
    /* // rather than an enum. */
    /* switch((line_message_type_t)zframe_data(cmd)) { */
    /*   /\* on an update, we check the monitors and triggers * */
    /*    * a trigger may fire, a monitor may be on.         * */
    /*    * we may also have received an update of our       * */
    /*    * accessory.                                       *\/ */
    /* case SERIAL_UPDATE: */
    /*   /\* expect to be sent two messages: value and type *\/ */
    assert(zmsg_size(msg) == 2);
      
    zframe_t * channel = zmsg_pop(msg);
    zmsg_t * out = zmsg_new();
    if (port_changed(channel, &channel_memory)) {
      zmsg_pushstr(out, channel_memory.current_channel);
      zmsg_pushstr(out, "CHANNEL_CHANGE");
      zmsg_send(&out, lineout);
    } else {
      zmsg_push(out, zmsg_pop(msg));
      zmsg_pushstr(out, "VALUE");

    }
    zframe_destroy(&channel);
    zmsg_destroy(&msg);
    
  }
  free(config);

    /*   break; */
    /*   // TODO this isn't the job of the filter */
    /* case MONITOR_ON: */
    /*   // create a pthread, wait till we've synchronised.  */
    /*   // delaying main thread while syncing is overhead but */
    /*   // acceptable, and necessary for correctness. */
      
    /*   chan = zmsg_pop(msg); */
    /*   if (zframe_streq(chan, channel_memory.current_channel)) { */
    /*     void * pipe = zthread_fork(context, watch_port, (void*)config); */
  // make sure you send the full line endpoint here.

    /*     s_send(pipe, channel_memory.current_channel); */
    /*     char * ok = s_recv(pipe); */
    /*     assert(strcmp(ok, "ok") == 0); */
    /*     free(ok); */
    /*     zsocket_destroy(context, pipe); */
    /*   } else { */
    /*     zclock_log("ignoring request for monitor: wrong channel requested"); */
    /*   } */
      
    /*   break; */
    /*   // TODO this isn't the job of the filter */
    /* case MONITOR_OFF:  */
    /*   s_send(monitor_controller, "CLEAR_MONITORS"); */
    /*   break; */
    /*   // TODO this isn't the job of the filter */
    /* case TRIGGER_ON:  */
    /*   // create a pthread, wait till we've synchronised, */
    /*   // pass it whatever it needs. TODO */
    /*   { */
    /*     void * pipe = zthread_fork(context, trigger, (void*)config); */
    /*     s_send(pipe, channel_memory.current_channel); */
    /*     // TODO what else does a channel need? */
    /*     char * ok = s_recv(pipe); */
    /*     assert(strcmp(ok, "ok") == 0); */
    /*     free(ok); */
    /*     zsocket_destroy(context, pipe); */
    /*     break; */
    /*   } */
    /*   // TODO this isn't the job of the filter */
    /* case TRIGGER_OFF: */
    /*   rule_id = zmsg_popstr(msg); */
    /*   s_sendmore(monitor_controller, "CLEAR_TRIGGER"); */
    /*   s_send(monitor_controller, rule_id); */
    /*   free(rule_id); */
    /*   break; */
    /* default: */
    /*   str = zframe_strdup(cmd); */
    /*   fprintf(stderr, "don't understand message %s; dropping\n", str); */
    /*   free(str); */
    /* } */
  
}
