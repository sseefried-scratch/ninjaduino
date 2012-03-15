#include "monitor.h"
#include "config.h"
#include "czmq.h"
#include "trigger.h"
#include "utils.h"
#include "msgpack_wrapper.h"
#include "mdcliapi2.h"

typedef struct {
  
} triggermemory_t;

int trigger_fired(triggermemory_t *m, char * value){
  return 1;
}

typedef struct {
  int line_id;
  int trigger_level;
  int reset_level;
} trigger_t;

int parse_trigger(msgpack_object * addins_obj, trigger_t * target) {
  assert(addins_obj->type == MSGPACK_OBJECT_MAP);
  msgpack_object_map addins_hash = addins_obj->via.map;
  int i;
  for(i=0; i<addins_hash.size; i++) {
    msgpack_object key = addins_hash.ptr[i].key;
    msgpack_object obj = addins_hash.ptr[i].val;
    if (key.type != MSGPACK_OBJECT_RAW ||
        obj.type != MSGPACK_OBJECT_RAW)
      return 0; // can't be fooling with these silly data structures
    int size = key.via.raw.size;
    const char * str = key.via.raw.ptr;
    if        (strncmp(str, "line_id", size) == 0) {
      // set!
    } else if (strncmp(str, "trigger_level", size) == 0) {
      // set trigger
    } else if (strncmp(str, "reset_level", size) == 0) {
      // set reset
    }
  }
  return 1;
}

void trigger(void *cvoid, 
             zctx_t * context, 
             void * control) {

  // triggerconfig_t * config = (triggerconfig_t*) cvoid;
  //set up msgpack stuff
  zclock_log("watch_port started!");
  msgpack_zone mempool;
  msgpack_zone_init(&mempool, 2048);
  trigger_t trigger;
  // TODO get broker in somehow
  mdcli_t * client = mdcli_new(broker, 1); //VERBOSE
  
  // sort out comms with the overlord
  zmsg_t * rule_details = zmsg_recv(control);
  assert(zmsg_size(rule_details) == 5);
  char * rule_id = zmsg_popstr(rule_details);
  char * trigger_name = zmsg_popstr(rule_details);
  triggermemory_t trigger_memory;
  // thisi might be a ninjablock+service combo, as in "n:1234:relay",
  // or a software service, as in "twitter"
  char * target_worker = zmsg_popstr(rule_details);
  char * auth = zmsg_popstr(rule_details); // msgpack packed.
  
  /*          zmsg_pushstr(msg, workername);
          zmsg_pushstr(msg, user_id);
          zmsg_pushstr(msg, line_id);
          zmsg_pushstr(msg, value);
          mdcli_send(client, target_service, msg);
  */
  zframe_t * addins = zmsg_pop(rule_details); // msgpack packed
  zmsg_destroy(&rule_details);
  msgpack_object * addins_obj = parse_msgpack(&mempool, addins);
  if(!parse_trigger(addins_obj, &trigger)) {
    //bad message
    zclock_log("bad trigger definition");
    return;
  }

  // what do we expect to get out of addins?
  //   line_id is mandatory
  //   trigger_level is optional
  //   reset_level is optional
  //   anything else is ignored.


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
        if(trigger_fired(&trigger_memory, value)) {
          // go!
          zmsg_t * msg = zmsg_new();
          zmsg_pushstr(msg, workername);
          zmsg_pushstr(msg, user_id);
          zmsg_pushmem(msg, &trigger.line_id, sizeof(int));
          zmsg_pushstr(msg, value);
          mdcli_send(client, target_worker, msg);
          
        }
           
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
