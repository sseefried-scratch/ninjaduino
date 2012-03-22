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
| line_id        |
|________________|
| value:msgpack  |
|________________|

*/

void dump_monitorconfig(monitorconfig_t * c) {
  zclock_log("source: %s\nline:%d\nchannel:%s",
          c->source_worker, c->line_id, c->channel);
}

void watch_port(void *cvoid, 
                zctx_t * context, 
                void * pipe ) {
  zclock_log("watch_port started!");
  monitorconfig_t * config = (monitorconfig_t*) cvoid;
  dump_monitorconfig(config);

  void * linein = zsocket_new(context, ZMQ_SUB);
  char * listen_socket = to_linesocket(config->line_id);
  char line_id[16];
  sprintf(line_id, "%d", config->line_id);
  zsocket_connect(linein, listen_socket);
  zsockopt_set_unsubscribe(linein, "");
  zsockopt_set_subscribe(linein, "CLEAR_MONITORS");
  zsockopt_set_subscribe(linein, "VALUE");
  // have set up subscription, can signal parent that we're ok.
  child_handshake(pipe);
  zsocket_destroy(context, pipe); // no longer require pipe

  void * lineout = zsocket_new(context, ZMQ_PUB);
  zsocket_connect(lineout, config->out_socket);
  time_t until = time(NULL) + 60;
  while(time(NULL)<until) {
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
      int res = *(int*)zframe_data(value);
      zclock_log("res is %d", res);
      char * new_channel = zmsg_popstr(msg);

      if(strcmp(new_channel, config->channel)!=0) {
        zclock_log("monitor on %d: listening for %s, channel changed to %s quitting",
                   config->line_id, config->channel, new_channel);
        zmsg_destroy(&msg);
        zframe_destroy(&cmd);
        break;
      }

      zmsg_t * to_send = zmsg_new();

      char buf[1024];
      sprintf(buf, "%d", res);
      zmsg_pushstr(to_send, buf);
      zmsg_pushstr(to_send, line_id);
      zmsg_pushstr(to_send, config->source_worker);

      zmsg_dump(to_send);
      zmsg_send(&to_send, lineout);
      // don't destroy value frame, now owned by zmsg
    }
    // else ignore
    zmsg_destroy(&msg);
    zframe_destroy(&cmd);
  }
  //cleanup
  zsocket_destroy(context, linein);
  zsocket_destroy(context, lineout);

}
