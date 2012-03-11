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
  void * portwatcher = NULL; // don't connect the socket until we need it.
  // realloc when you get more.
  int monitor_size = 0;
  //monitor_t * monitors = NULL;
  int confirmed_rounds = 0;
  channel_memory_t channel_memory = { NULL, NULL, 0 };
  void * monitor_controller;
  //  int trigger_capacity = 1;
  // monitor_t * monitors = malloc(1*sizeof(monitor_t));

  while(1) {
    int i;
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
        s_send(monitor_controller, "CHANNELCHANGE");
      } else {
        char * value = zmsg_popstr(msg);
        s_send_more(monitor_controller, "VALUE");
        s_send_more(monitor_controller, channel_memory.current_channel);
        s_send(monitor_controller, value);

        // send to triggers too? or can they listen on the same port?
      }
        // check we have a connection
        if (monitor_size > 0) {
          if (!portwatcher) {
            portwatcher = zsocket_new(config->context, ZMQ_PUB);
            zmq_connect(portwatcher, config->portwatcher_endpoint);
          }
          char * channel_name = zframe_strdup(channel);

          // fire all existing monitors
          for(i=0;i<monitor_size;i++) {
            if (fire_monitor(&monitors[i])) {
              zmsg_t * notification = zmsg_new();
              zmsg_pushstr(notification, config->identity);
              zmsg_pushstr(notification, channel_name);
              zmsg_pushstr(notification, value);
              zmsg_send(&notification, portwatcher); // also destroys msg
            }
          }
          free(value);
          free(channel_name);
        }
      }
      zframe_destroy(&channel);
      zmsg_destroy(&msg);
      break;
    case MONITOR_ON:
      // what's in a monitor? hrm. TODO
    case MONITOR_OFF: break; // not used yet
    case TRIGGER_ON: // TODO
    case TRIGGER_OFF: // TODO
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
