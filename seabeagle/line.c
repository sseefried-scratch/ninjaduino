#include "line.h"
#include "zhelpers.h"
#include "czmq.h"
#include "monitor.h"

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


void line_listener(int line_id, void * subscriber, config_t* config) {
  char * str;
  zmsg_t * msg;
  // realloc when you get more.
  int monitor_size = 0;
  int confirmed_rounds = 0;
  channel_memory_t channel_memory = { NULL, NULL, 0 };
  void * monitor_controller = zsocket_new(config->context, ZMQ_PUB);
  zsocket_bind(monitor_controller, "inproc://monitor_controller");
  //  int trigger_capacity = 1;


  while(1) {
    int i;
    zframe_t * chan;
    char * rule_id;
    msg = zmsg_recv(subscriber);
    zframe_t * cmd = zmsg_pop(msg);

    switch((line_message_type_t)zframe_data(cmd)) {
      /* on an update, we check the monitors and triggers *
       * a trigger may fire, a monitor may be on.         *
       * we may also have received an update of our       *
       * accessory.                                       */
    case SERIAL_UPDATE:
      /* expect to be sent two messages: value and type */
      assert(zmsg_size(msg) == 2);
      
      zframe_t * channel = zmsg_pop(msg);
      if (port_changed(channel, &channel_memory)) {
        s_sendmore(monitor_controller, "CHANNEL_CHANGE");
        s_send(monitor_controller, channel_memory.current_channel);
      } else {
        char * value = zmsg_popstr(msg);
        s_sendmore(monitor_controller, "VALUE");
        s_send(monitor_controller, value);
        free(value);
      }
      zframe_destroy(&channel);
      zmsg_destroy(&msg);
      break;
    case MONITOR_ON:
      // what's in a monitor? hrm. TODO
      // create a pthread, wait till we've synchronised
      // while it's a bit annoying to be paused, if we don't then we
      // might miss a channel change event, and the monitor will
      // keep blithely sending garbage.
      
      chan = zmsg_pop(msg);
      if (zframe_streq(chan, channel_memory.current_channel)) {
        void * pipe = zthread_fork(config->context, watch_port, (void*)config);
        s_send(pipe, channel_memory.current_channel);
        char * ok = s_recv(pipe);
        assert(strcmp(ok, "ok") == 0);
        free(ok);
      } else {
        zclock_log("ignoring request for monitor: wrong channel requested");
      }
      
      
    case MONITOR_OFF: 
      s_send(monitor_controller, "CLEAR_MONITORS");
      break;
    case TRIGGER_ON: 
      // create a pthread, wait till we've synchronised,
      // pass it whatever it needs. TODO

    case TRIGGER_OFF:
      rule_id = zmsg_popstr(msg);
      s_sendmore(monitor_controller, "CLEAR_TRIGGER");
      s_send(monitor_controller, rule_id);
      free(rule_id);
      break;
    default:
      str = zframe_strdup(cmd);
      fprintf(stderr, "don't understand message %s; dropping\n", str);
      free(str);
    }
  }
  // encapsulate logic behind lines
  fprintf(stderr, "not implemented!");
  exit(1);

}
