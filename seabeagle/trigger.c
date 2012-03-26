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



/* int trigger_fired(triggermemory_t *m, triggertype_t * ttype, char * value){ */
  
/* } */

// what do we expect to get out of addins?
//   line_id is mandatory
//   trigger_level is optional
//   reset_level is optional
//   anything else is ignored.
int parse_addins(msgpack_object * addins_obj, triggermemory_t * target) {
  msgpack_object_print(stdout, *addins_obj);
  if (addins_obj->type != MSGPACK_OBJECT_MAP) {
    zclock_log("expected a hash at the top level, got %d",
               addins_obj->type);
    return 0;
  }
  
  msgpack_object_map addins_hash = addins_obj->via.map;
  int i;
  int success = 0;
  for(i=0; i<addins_hash.size; i++) {
    msgpack_object key = addins_hash.ptr[i].key;
    msgpack_object obj = addins_hash.ptr[i].val;
    if (key.type != MSGPACK_OBJECT_RAW) {
      printf("key expected raw, got %d\n", key.type);
      return 0;
    }
    if(obj.type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
      printf("obj expected positive integer, got %d\n", obj.type);
      continue;
    }
    int size = key.via.raw.size;
    const char * kstr = key.via.raw.ptr;
    int64_t o = obj.via.i64;
    if        (strncmp(kstr, "line", size) == 0) {
      target->line_id = o;
      printf("found line\n");
      success = 1;
    } else if (strncmp(kstr, "trigger_level", size) == 0) {
      target->trigger_level = o;
    } else if (strncmp(kstr, "reset_level", size) == 0) {
      target->reset_level = o;
    }
  }
  return success;
}


void send_trigger(mdcli_t * client, char * target_worker, char * rule_id,  int ival, char * user_id) {
  zclock_log("activating trigger\ntarget=%s\nvalue=%d\nuser=%s",
             target_worker, ival, user_id);
  struct timeval tval;
  gettimeofday(&tval, NULL);
  // make a messagepack hash
  msgpack_sbuffer * buffer =  msgpack_sbuffer_new();
  msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);
  // value chunk
  msgpack_pack_map(pk, 3);
  // key
  msgpack_pack_raw(pk, 5);
  msgpack_pack_raw_body(pk, "value", 5);
  // value
  msgpack_pack_int(pk, ival);

  //time chunk

  // key
  msgpack_pack_raw(pk, 5);
  msgpack_pack_raw_body(pk, "epoch", 5);
  // time
  msgpack_pack_int(pk, tval.tv_sec);

  msgpack_pack_raw(pk, 6);
  msgpack_pack_raw_body(pk, "micros", 6);
  // time
  msgpack_pack_int(pk, tval.tv_usec);


  zmsg_t * msg = zmsg_new();
  // really, the user_id should be being added by a
  // gatekeeper, not the block itself, or it's a security
  // hole. will do for now FIX
  
  zmsg_pushstr(msg, user_id);
  // zmsg_pushmem(msg, &trigger.line_id, sizeof(int));

  zmsg_pushmem(msg, buffer->data, buffer->size);
  zmsg_pushstr(msg, rule_id);
  zmsg_pushstr(msg, "DoAction");
  mdcli_send(client, target_worker, &msg);
}




int falls_below(triggermemory_t * mem, int value) {
  zclock_log("falls below called: reset=%d, trigger=%d, value=%d", 
             mem->reset_level, mem->trigger_level, value);
  if (mem->ready) {
    if (value  <= mem->trigger_level) {
      zclock_log("trigger fires, deactivate");
      mem->ready = 0;
      return 1;
    } else {
      return 0;
    }
  } else {
    if (value >= mem->reset_level) {
      zclock_log("trigger reactivates!");
      mem->ready = 1;
    }
    return 0;
  }

}

int rises_above(triggermemory_t * mem, int value) {

  if (mem->ready) {
    if (value  >= mem->trigger_level) {
      zclock_log("rises above trigger fires: reset=%d, trigger=%d, value=%d", 
                 mem->reset_level, mem->trigger_level, value);
      mem->ready = 0;
      return 1;
    } else {
      return 0;
    }
  } else {
    if (value <= mem->reset_level) {
      zclock_log("rises above trigger resets: reset=%d, trigger=%d, value=%d", 
                 mem->reset_level, mem->trigger_level, value);
      mem->ready = 1;
    }
    return 0;
  }

}

void dump_trigger(triggermemory_t * mem) {
  zclock_log("line=%d,reset=%d, trigger=%d, ready=%d\n",
             mem->line_id, mem->reset_level, mem->trigger_level,mem->ready);
}


triggerfunction find_trigger(char * channel, char * triggername){
  if (strcmp("light", channel) == 0) {
    if (strcmp("light_level_falls_below", triggername) == 0) {
      return &falls_below;
    }
    if (strcmp("light_level_rises_above", triggername) == 0) {
      return &rises_above;
    }
  }
  return NULL;
}

int create_triggerconfig(triggerconfig_t * conf,
                         zmsg_t * rule_details,
                         char * channel,
                         char * rule_id) {
  if(zmsg_size(rule_details) !=4) {
    zclock_log("bad rule");
    zmsg_dump(rule_details);
    return 1;
  }

  conf->rule_id = rule_id;
  conf->channel = channel;

  conf->trigger_name = zmsg_popstr(rule_details);
  conf->target_worker = zmsg_popstr(rule_details);
  conf->auth = zmsg_pop(rule_details); // msgpack packed.
  conf->addins = zmsg_pop(rule_details); // msgpack packed
  return 0;
}

void trigger(void *cvoid, 
             zctx_t * context, 
             void * control) {
  triggerconfig_t * c = (triggerconfig_t*) cvoid;
  //set up msgpack stuff
  zclock_log("watch_port started!");
  msgpack_zone mempool;
  msgpack_zone_init(&mempool, 2048);

  // TODO
  char * user_id = "17"; 
  // TODO get broker in somehow
  char * broker = "tcp://au.ninjablocks.com:5773";

  mdcli_t * client = mdcli_new(broker, 1); //VERBOSE

  triggermemory_t trigger_memory;
  msgpack_object * addins_obj = parse_msgpack(&mempool, c->addins);

  if(!parse_addins(addins_obj, &trigger_memory)) {
    //bad message
    zclock_log("bad trigger definition");
    msgpack_object_print(stdout, *addins_obj);
    send_sync("bad trigger", control);
    return;
  }
  zclock_log("Creating trigger: target %s, rule_id %s, name %s", 
             c->target_worker, c->rule_id, c->trigger_name);
  dump_trigger(&trigger_memory);
  triggerfunction trigger_func;
  if(!(trigger_func = find_trigger(c->channel, c->trigger_name))) {

    zclock_log("no trigger found for channel %s, trigger %s",
               c->channel, c->trigger_name);
    send_sync("no such trigger", control);
    return;
  }

  void * line = zsocket_new(context, ZMQ_SUB);


  // what line are we on?
  // this comes in the addins. 
  char * linesocket = to_linesocket(trigger_memory.line_id);
  zclock_log("trigger is listening on %s", linesocket);
  zsocket_connect(line, linesocket);

  zsockopt_set_unsubscribe(line, "");
  zsockopt_set_subscribe(line, "VALUE");
  recv_sync("ping", control);
  send_sync("pong", control);
  
  zmq_pollitem_t items [] = {
    { line, 0, ZMQ_POLLIN, 0 },
    { control, 0, ZMQ_POLLIN, 0 }
  };
  while(1) {
    // listen on control and line
    zmq_poll (items, 2, -1);
    if (items[1].revents & ZMQ_POLLIN) {
      zclock_log("rule %s received message on control pipe", c->rule_id);
      // control message
      // really only expecting DESTROY
      zmsg_t * msg = zmsg_recv(control);
      char * str = zmsg_popstr(msg);
      zmsg_destroy(&msg);
      
      if (strcmp("Destroy", str) == 0) {
        zclock_log("rule %s will quit on request", c->rule_id);
        free(str);
        send_sync("ok", control);
        zclock_log("rule %s quitting on request", c->rule_id);
        break;
      } else  {
        zclock_log("unexpected command %s for rule %s", str, c->rule_id);
        free(str);
        send_sync("ok", control);
      }
    }

    if (items[0].revents & ZMQ_POLLIN) {
      // serial update
      zmsg_t * msg = zmsg_recv(line);
      zframe_t * cmd = zmsg_pop(msg);
      if(zframe_streq(cmd, "CHANNEL_CHANGE")) {
        // TODO
        // must have been dormant to have gotten this
        char * new_channel = zmsg_popstr(msg);

        if(strcmp(c->channel, new_channel) == 0) {
        // oh, happy day! We're relevant again.
        // reactivate and start looking at reset levels.
          zclock_log("line %d: changed channel from %s to %s: trigger coming back to life", trigger_memory.line_id, c->channel, new_channel);
          zsockopt_set_subscribe(line, "VALUE");
          zsockopt_set_unsubscribe(line, "CHANNEL_CHANGE");
        }
        free(new_channel);
      } else if (zframe_streq(cmd, "VALUE")) {
        zframe_t * vframe = zmsg_pop(msg);
        int value;
        memcpy(&value, zframe_data(vframe), sizeof(int));
        char * update_channel = zmsg_popstr(msg);

        if(strcmp(c->channel, update_channel) != 0) {
          // channel changed,  go dormant
          // this is legit according to my tests at
          // https://gist.github.com/2042350

          zclock_log("line %d: changed channel from %s to %s: trigger going dormant", trigger_memory.line_id, c->channel, update_channel);
          zsockopt_set_subscribe(line, "CHANNEL_CHANGE");
          zsockopt_set_unsubscribe(line, "VALUE");
        } 
        
        else if(trigger_func(&trigger_memory, value)) {
          send_trigger(client, c->target_worker, c->rule_id, value, user_id);
        }           

        free(update_channel);
      } else {
        // shouldn't ever happen.
        zclock_log("shouldn't have received command %s\n", zframe_strdup(cmd));
      }
      zmsg_destroy(&msg);
      zframe_destroy(&cmd);
    }
    
  }

  msgpack_zone_destroy(&mempool);
}
