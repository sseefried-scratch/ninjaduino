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

int LIMIT = 1;
typedef struct {
  char * current_channel;
  char * next_channel;
  int confirmed_rounds;
} channel_memory_t;

int port_changed(char * channel, channel_memory_t * m) {
  // zclock_log("line\nchannel: %s\nchan_memory:%s\nnext_chan:%s",
  //            channel, m->current_channel, m->next_channel);
  if (strcmp(channel, m->current_channel)==0) {
    return 0;
  }
  if(m->next_channel && strcmp(channel, m->next_channel)==0) {
    if(m->confirmed_rounds++ > LIMIT) {
      // time to change
      zclock_log("channel change! %s to %s", m->current_channel, channel);
      
      free(m->next_channel);
      free(m->current_channel);
      m->next_channel = NULL;
      m->confirmed_rounds=0;
      m->current_channel = strdup(channel);
      return 1;
    } else {
      zclock_log("tick %s:%s", m->current_channel, channel);
      return 0;
    }
  }

  // new channel that we weren't expecting.
  free(m->next_channel);
  m->next_channel = strdup(channel);
  return 0;
}

void dump_lineconfig(lineconfig_t * c) {
  zclock_log("LINE CONFIG\n%d", c->line_id);
}

void line_listener(void * cvoid, zctx_t * context, void * pipe) {
  lineconfig_t * config = (lineconfig_t*)  cvoid;
  // atm, topic == outpipe, but this is coincidental...
  zmsg_t * msg;
  dump_lineconfig(config);
  channel_memory_t channel_memory = { strdup("unknown"),
                                      strdup("unknown"),
                                      0 };
  // void * monitor_controller = zsocket_new(config->context, ZMQ_PUB);
  //  zsocket_bind(monitor_controller, "inproc://monitor_controller");
  //  int trigger_capacity = 1;
  
  void * lineout = zsocket_new(context, ZMQ_PUB);
  char * outpipe = to_linesocket(config->line_id);
  zclock_log("binding line |%s|", outpipe);
  zsocket_bind(lineout, outpipe);

  void * subscriber = zsocket_new(context, ZMQ_SUB);
  zclock_log("subscribing to line |%d|", config->line_id);
  zsockopt_set_unsubscribe(subscriber, "");

  char * topic = to_line(config->line_id);

  zsockopt_set_subscribe(subscriber, topic);
  zsockopt_set_subscribe(subscriber, "DESTROY"); // listen for
                                                 // shutdown message.
  zclock_log("subscribing to literal line |%s|", topic);
  zsocket_connect(subscriber, "inproc://line");

  child_handshake(pipe);
  zsocket_destroy(context, pipe);

  while(1) {
    msg = zmsg_recv(subscriber);
    if(!msg) {
      zclock_log("line quitting!");
      return;
    }
        
    // zmsg_dump(msg);
    char * recv_topic = zmsg_popstr(msg);
    if (strcmp("DESTROY", recv_topic) == 0) {
      // shutdown
      
      // TODO free memory.
      return;
    }

    // zclock_log("line got topic\nreceived: %s\nexpected: %s\n", recv_topic, config->topic);
    //fflush(stdout);
    assert(strcmp(recv_topic, topic)==0);
    free(recv_topic);
    assert(zmsg_size(msg) == 2);
      
    char * channel = zmsg_popstr(msg);
    zmsg_t * out = zmsg_new();
    // originally, I thought it was a neat trick to not mention the
    // channel in every message. unfortunately, this screws up the
    // case where a new trigger gets introduced: at the beginning, it
    // has no idea what the channel currently is.

    // rather than trying to micro-optimise, let's just keep repeating
    // the channel in the value update too.

    if (port_changed(channel, &channel_memory)) {
      zmsg_pushstr(out, channel_memory.current_channel);
      zmsg_pushstr(out, "CHANNEL_CHANGE");
      zmsg_send(&out, lineout);
    }
    // only send a value if we're all settled down
    if(strcmp(channel, channel_memory.current_channel)==0) {
      out = zmsg_new();
      zmsg_pushstr(out, channel_memory.current_channel);
      zmsg_push(out, zmsg_pop(msg));
      zmsg_pushstr(out, "VALUE");
      zmsg_send(&out, lineout);
    }
    free(channel);
    zmsg_destroy(&msg);
    
  }
  free(config);
}
