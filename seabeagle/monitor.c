#include "monitor.h"
#include "config.h"
#include "czmq.h"
#include "utils.h"
/*

Incoming messages from linein
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

Outgoing messages to portwatcher
__________________
| source_worker  |
|________________|
| channel        |
|________________|
| line_id        |
|________________|
| value:msgpack  |
|________________|

*/

void dump_monitorconfig(monitorconfig_t * c) {
  zclock_log("source: %s\nline:%d\nlistener:%s\nout:%s\nchannel:%s",
          c->source_worker, c->line_id, c->listen_socket, c->out_socket, c->channel);
}

void watch_port(void *cvoid, 
                zctx_t * context, 
                void * pipe ) {
  zclock_log("watch_port started!");
  monitorconfig_t * config = (monitorconfig_t*) cvoid;
  dump_monitorconfig(config);

  void * linein = zsocket_new(context, ZMQ_SUB);
  zsocket_connect(linein, config->listen_socket);
  zsockopt_set_unsubscribe(linein, "");
  zsockopt_set_subscribe(linein, "CLEAR_MONITORS");
  zsockopt_set_subscribe(linein, "VALUE");
  // have set up subscription, can signal parent that we're ok.
  child_handshake(pipe);
  zsocket_destroy(context, pipe); // no longer require pipe

  void * lineout = zsocket_new(context, ZMQ_PUB);
  zsocket_connect(lineout, config->out_socket);

  while(1) {
    zmsg_t * msg = zmsg_recv(linein);
    if(!msg) {
      zclock_log("monitor quitting!");
      return;
    }
    zframe_t * cmd = zmsg_pop(msg);
    if(zframe_streq(cmd, "CLEAR_MONITORS")) {
      zclock_log("ephemeral monitor quitting");
      zmsg_destroy(&msg);
      zframe_destroy(&cmd);
      break;
    } else if (zframe_streq(cmd, "VALUE")) {
      // TODO perhaps some rate limiting necessary
      assert(zmsg_size(msg) == 2);
      
      zframe_t * value = zmsg_pop(msg);
      char * new_channel = zmsg_popstr(msg);
      if(strcmp(new_channel, config->channel)!=0) {
        zclock_log("channel change, monitor quitting");
        zmsg_destroy(&msg);
        zframe_destroy(&cmd);
        break;
      }

      zmsg_t * to_send = zmsg_new();
      zmsg_push(to_send, value);
      zmsg_pushstr(to_send, config->channel);
      zmsg_pushstr(to_send, config->source_worker);      
      zmsg_send(&to_send, lineout);
      // don't destroy value frame, now owned by zmsg
    }
    // else ignore
    zmsg_destroy(&msg);
    zframe_destroy(&cmd);
  }
  //cleanup
  zsocket_destroy(context, &linein);
  zsocket_destroy(context, &lineout);

}
