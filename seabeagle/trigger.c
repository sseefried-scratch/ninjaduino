#include "monitor.h"
#include "config.h"
#include "czmq.h"
#include "trigger.h"
#include "utils.h"
#include "msgpack_wrapper.h"
#include "mdcliapi2.h"

/* Message format

send to "service_name"
   ___________________
   | "DoAction"      |
   | rule_id         |
   | addins:msgpack  |
   | user_id         |
   |_________________|

*/

typedef struct {
  int ready;
  int trigger_level;
} triggermemory_t;

typedef struct 
{
  int (*comparator)( int, int);
} triggertype_t;


typedef struct {
  int line_id;
  int trigger_level;
  int reset_level;
} trigger_t;

int (*pt2Function) (float, char, char); // C
int trigger_fired(triggermemory_t *m, triggertype_t * ttype, char * value){
  
}

// what do we expect to get out of addins?
//   line_id is mandatory
//   trigger_level is optional
//   reset_level is optional
//   anything else is ignored.
int parse_trigger(msgpack_object * addins_obj, trigger_t * target) {
  assert(addins_obj->type == MSGPACK_OBJECT_MAP);

  msgpack_object_map addins_hash = addins_obj->via.map;
  int i;
  for(i=0; i<addins_hash.size; i++) {
    msgpack_object key = addins_hash.ptr[i].key;
    msgpack_object obj = addins_hash.ptr[i].val;
    if (key.type != MSGPACK_OBJECT_RAW) {
      printf("key expected raw, got %d\n", key.type);
      return 0;
    }
    if(obj.type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
      printf("obj expected raw, got %d\n", obj.type);
      return 0; // can't be fooling with these silly data structures
    }
    int size = key.via.raw.size;
    const char * kstr = key.via.raw.ptr;
    int64_t o = obj.via.i64;
    if        (strncmp(kstr, "line_id", size) == 0) {
      target->line_id = o;
    } else if (strncmp(kstr, "trigger_level", size) == 0) {
      target->trigger_level = o;
    } else if (strncmp(kstr, "reset_level", size) == 0) {
      target->reset_level = o;
    }
  }
  return 1;
}


void send_trigger(mdcli_t * client, char * target_worker, char * raw_value, char * user_id) {
  zmsg_t * msg = zmsg_new();
  // really, the user_id should be being added by a
  // gatekeeper, not the block itself, or it's a security
  // hole. will do for now FIX
  
  zmsg_pushstr(msg, user_id);
  // zmsg_pushmem(msg, &trigger.line_id, sizeof(int));
  zmsg_pushstr(msg, raw_value);
  mdcli_send(client, target_worker, &msg);
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
  // TODO
  char * user_id = "1"; 
  // TODO get broker in somehow
  char * broker = "tcp://au.ninjablocks.com:5773";
  // TODO channel from somewhere
  char * channel;
  mdcli_t * client = mdcli_new(broker, 1); //VERBOSE
  triggertype_t trigger_type;
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
  
  zframe_t * addins = zmsg_pop(rule_details); // msgpack packed
  zmsg_destroy(&rule_details);
  msgpack_object * addins_obj = parse_msgpack(&mempool, addins);
  if(!parse_trigger(addins_obj, &trigger)) {
    //bad message
    zclock_log("bad trigger definition");
    return;
  }

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
        char * new_channel = zmsg_popstr(msg);

        if(strcmp(channel, new_channel) == 0) {
        // oh, happy day! We're relevant again.
        // reactivate and start looking at reset levels.
          zsockopt_set_subscribe(line, "VALUE");
          zsockopt_set_unsubscribe(line, "CHANNEL_CHANGE");
        }
        free(new_channel);
      } else if (zframe_streq(cmd, "VALUE")) {
        char * value = zmsg_popstr(msg);
        char * update_channel = zmsg_popstr(msg);
        if(strcmp(channel, update_channel) != 0) {
          // channel changed,  go dormant
          // this is legit according to my tests at https://gist.github.com/2042350
          zsockopt_set_subscribe(line, "CHANNEL_CHANGE");
          zsockopt_set_unsubscribe(line, "VALUE");
        } 
        else if(trigger_fired(&trigger_memory, &trigger_type, value)) {
          send_trigger(client, target_worker, value, user_id);
        }           
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
