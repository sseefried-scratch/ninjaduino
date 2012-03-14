#include "monitor.h"
#include "config.h"
#include "czmq.h"
#include "trigger.h"
#include "utils.h"
#include "msgpack_wrapper.h"

void trigger(void *cvoid, 
             zctx_t * context, 
             void * control) {
  zclock_log("watch_port started!");
  triggerconfig_t * config = (triggerconfig_t*) cvoid;
  //set up msgpack stuff
  msgpack_zone mempool;
  msgpack_zone_init(&mempool, 2048);
  
  // sort out comms with the overlord
  zmsg_t * rule_details = zmsg_recv(control);
  assert(zmsg_size(rule_details) == 5);
  char * rule_id = zmsg_popstr(rule_details);
  char * trigger_name = zmsg_popstr(rule_details);
  // thisi might be a ninjablock+service combo, as in "n:1234:relay",
  // or a software service, as in "twitter"
  char * target_worker = zmsg_popstr(rule_details);
  char * auth = zmsg_popstr(rule_details); // msgpack packed.
  zframe_t * addins = zmsg_pop(rule_details); // msgpack packed
  zmsg_destroy(&rule_details);
  msgpack_object * addins_obj = parse_msgpack(&mempool, addins);
  
  void * line = zsocket_new(context, ZMQ_SUB);
  zsockopt_set_unsubscribe(line, "");
  zsockopt_set_subscribe(line, "VALUE");

  // what line are we on?
  // this comes in the addins. 
  zsocket_connect(line, "TODO");
  send_sync("ok", control);

  zmq_pollitem_t items [] = {
    { line, 0, ZMQ_POLLIN, 0 },
    { control, 0, ZMQ_POLLIN, 0 }
  };
  while(1) {
    // listen on control and line
    zmq_poll (items, 2, -1);
    if (items[0].revents & ZMQ_POLLIN) {
      // serial update
      zmsg_t * msg = zmsg_recv(line);
      zframe_t * cmd = zmsg_pop(msg);
      if(zframe_streq(cmd, "CHANNEL_CHANGE")) {
        // TODO
        // must have been dormant to have gotten this
        // if we've changed to our channel, oh, happy day!
        // reactivate and start looking at reset levels again.
      } else if (zframe_streq(cmd, "VALUE")) {
        char * value = zmsg_popstr(msg);
        // TODO what does a trigger do?
        // first, check the channel is correct, and go dormant if it's
        // not.
        // then, handle the trigger/reset logic.
        // go dormant -> zsockopt_set_subscribe(line, "CHANNEL_CHANGE");
        //            -> zsockopt_set_unsubscribe(line, "VALUE");
      } else {
        // shouldn't ever happen.
        zclock_log("shouldn't have received command %s\n", zframe_strdup(cmd));
      }
      zmsg_destroy(&msg);
      zframe_destroy(&cmd);
    }
    if (items[1].revents & ZMQ_POLLIN) {
      // control message
      // really only expecting DESTROY
      zmsg_t * msg = zmsg_recv(control);
      char * str = zmsg_popstr(msg);
      zmsg_destroy(&msg);
      
      if (strcmp("DESTROY", str) == 0) {
        free(str);
        send_sync("ok", control);
        zclock_log("rule %s quitting on request", rule_id);
        break;
      } else  {
        zclock_log("unexpected command %s for rule %s", str, rule_id);
        free(str);
      }
    }
    
  }

  msgpack_zone_destroy(&mempool);
}
