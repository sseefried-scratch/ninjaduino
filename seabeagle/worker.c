#include <czmq.h>
#include <sqlite3.h>
#include "mdwrkapi.h"
#include "utils.h"
#include "trigger.h"
#include "monitor.h"
#include "worker_config.h"
/*

  This is the worker that listens for addition & deletion of rules.
  each rule is a thread unto itself, we just turn them on and off as
  directed by the user.

  additionally, each rule should monitor its line and only listen for
  CHANNEL_CHANGE messages when dormant.


 */

/*

   Message formats.

   There are a number of different message formats that this worker
   can handle. The format of the message is distinguished by the first
   part of the multipart message.

   In the ASCII diagrams below anything expected verbatim is in quotes
   (e.g. "AddMonitor"). Everything else is an a arbitrary field name.
   Naturally every message part in 0MQ is a binary blob, but each
   each field is annotated with the type we are expecting

   - strings are not null terminated
   - integers are just numeric strings
   - msgpack`s are defined at http://msgpack.org/

   ________________
   | "AddMonitor" |
   |______________|
   ___________________
   | "RemoveMonitor" |
   |_________________|

   ___________________________
   | "AddAction"             |
   | rule_id: integer        |
   | action_name: string     |
   | auth: msgpack           |
   | addins: msgpack         |
   |-------------------------|

   ___________________________
   | "AddTrigger"            |
   | rule_id: integer        |
   | trigger_name: string    |
   | target_service: string  |
   | auth: msgpack           |
   | addins: msgpack         |
   |-------------------------|
     - addins must contain "line_id";
     - "trigger_level" and "reset_level" are
       respected.
     - auth is ignored.

    We don't have RemoveTrigger and RemoveAction, because RemoveRule can
    remove either Triggers or Actions
   ____________________
   | "RemoveRule"     |
   | rule_id: integer |
   |__________________|


 */

/*

  Also, each worker has its own sqlite database. Thus, it can slurp up
  the rules when it starts up again.

  in the fullness of time, it should also check with the back end and
  reload anything it doesn't know about. This is a useful hack for
  now, though.

*/


sqlite3 * init_db(char * servicename) {
  int rc;
  sqlite3 * db;
  char *zErrMsg = 0;
  char * buf = malloc(strlen(servicename) + 4);
  sprintf(buf, "%s.db", servicename);
  rc = sqlite3_open(buf, &db);
  free(buf);
  if(rc!=0) {
    zclock_log("ERROR: couldn't write to %s db: %s", servicename, zErrMsg);
    return NULL;
  }
  // create table if it doesn't exist
  rc = sqlite3_exec(db, "create table if not exists rules (\
rule_id string,\
trigger_name string,\
target_worker string,\
auth string,\
addins string);", NULL, NULL, &zErrMsg);

  return db;
}

typedef struct {
  zhash_t * rules;
  zctx_t * context;
} rulepackage_t;




// somewhat counterintuitively, this returns NULL if everything's ok.
char * create_trigger(zhash_t * rules, 
                      char * rule_id, 
                      zctx_t * context,
                      triggerconfig_t * tconf) {
  void * rule_pipe = zthread_fork(context, trigger, tconf);
  send_sync("ping", rule_pipe);
  char * pipe_resp = zstr_recv(rule_pipe);
  

  if(strcmp(pipe_resp, "pong") == 0) {
    zhash_insert(rules, rule_id, rule_pipe);
    free(pipe_resp);
    return NULL;
  } else {
    zclock_log("something went wrong creating a trigger: %s", pipe_resp);
    return pipe_resp;
  }
}

int load_rule(void *rvoid, int argc, char ** argv,  char ** column) {
  int i;
  for(i=0;i<argc;i++) {
    printf("Argument %d is %s\n", i, argv[i]);
  }
  rulepackage_t * rpkg = (rulepackage_t *) rvoid;
  triggerconfig_t * tconf = malloc(sizeof(triggerconfig_t));
  tconf->rule_id       = strdup(argv[0]);
  tconf->trigger_name  = strdup(argv[1]);
  tconf->target_worker = strdup(argv[2]);
  // FIXME auth and addins are blobs. how do we pull them out?

  create_trigger(rpkg->rules, argv[0], rpkg->context, tconf);
}

void reload_rules(zctx_t * context, sqlite3 * db, char * servicename, zhash_t * rules) {

  char *zErrMsg = 0;
  int rc;
  rulepackage_t rpkg = {rules, context};
  rc=sqlite3_exec(db, "select rule_id,trigger_name,target_worker,auth,addins from rules", load_rule, &rpkg, &zErrMsg);
  if(rc!=0) {
    // not necessarily a problem - if we've never written to the db,
    // the rules table doesn't exist, which is fine.
    zclock_log("INFO: couldn't read from %s db: %s", servicename, zErrMsg);
    return;
  }

}


void generic_worker(void * cvoid, zctx_t * context, void * pipe) {
  workerconfig_t *config = (workerconfig_t*) cvoid;
  zhash_t * rules = zhash_new();

  child_handshake(pipe);
  zmsg_t *reply = NULL;
  void * rule_pipe = NULL;
  char * ninja = config->base_config->identity;
  char * channel = config->channel;

  char * servicename = malloc(strlen(ninja) +
                              strlen(channel) + 2);
  sprintf(servicename, "%s:%s", ninja,channel);

  // sqlite stuff
  int rc;
  char * tail = NULL;
  sqlite3 *db = init_db(servicename);
  sqlite3_stmt * insert_stmt;
  zclock_log("%s worker preparing rules...", servicename);
  sqlite3_prepare_v2(db, "insert into rules VALUES (@RULEID, @TRIGGER_NAME, @TARGET_WORKER, @AUTH, @ADDINS);", 512, &insert_stmt, NULL);
  zclock_log("%s worker reloading rules...", servicename);
  reload_rules(context, db, servicename, rules);
  zclock_log("%s worker connecting...", servicename);
  mdwrk_t *session = mdwrk_new (config->base_config->broker_endpoint, servicename, 0);
  zclock_log("%s worker connected!", servicename);

  while (1) {
    zclock_log("%s worker waiting for work.", servicename);
    zmsg_t *request = mdwrk_recv (session, &reply);

    if (request == NULL)
      break;              //  Worker was interrupted

    char * command = zmsg_popstr(request);
    char * rule_id = zmsg_popstr(request);

    zclock_log("%s worker servicing request %s for rule %s", servicename,command,rule_id);
    reply = zmsg_new();
    if (strcmp(command, "AddTrigger") == 0) {
      zclock_log("new trigger!");
      if (zhash_lookup(rules, rule_id)) {
        // already have a rule with that id! WTF??
        // FIXME should probably delete this and reinstate
        zclock_log("Received duplicate rule %s, ignoring", rule_id);
        zmsg_destroy(&request);

        zmsg_pushstr(reply, "duplicate");
      } else {
        triggerconfig_t * tconf = malloc(sizeof(triggerconfig_t));
        create_triggerconfig(tconf, request, channel, rule_id);
        char * created = create_trigger(rules, rule_id, context, tconf);
        if(NULL == created) {
          // happy path, so add to db
          sqlite3_bind_text(insert_stmt, 1, tconf->rule_id, -1, SQLITE_TRANSIENT);
          sqlite3_bind_text(insert_stmt, 2, tconf->trigger_name, -1, SQLITE_TRANSIENT);
          sqlite3_bind_text(insert_stmt, 3, tconf->target_worker, -1, SQLITE_TRANSIENT);
          sqlite3_bind_blob(insert_stmt, 4, zframe_data(tconf->auth), zframe_size(tconf->auth), SQLITE_TRANSIENT);
          sqlite3_bind_blob(insert_stmt, 5, zframe_data(tconf->addins), zframe_size(tconf->addins), SQLITE_TRANSIENT);
          sqlite3_step(insert_stmt);
          sqlite3_clear_bindings(insert_stmt);
          sqlite3_reset(insert_stmt);

          zmsg_pushstr(reply, "ok");
        } else {
          zclock_log("create_trigger failed: %s", created);
          zmsg_pushstr(reply, created);
        }
        free(created);

      }
    } else if (strcmp(command,"RemoveRule") == 0) {

      if (rule_pipe=zhash_lookup(rules, rule_id)) {
        // found it
        zclock_log("rule %s exists, removing.", rule_id);
        send_sync("Destroy",rule_pipe);
        zclock_log("rule %s waiting for OK from pipe", rule_id);
        recv_sync("ok", rule_pipe);
        zsocket_destroy(context, rule_pipe);
        zhash_delete(rules, rule_id);
        zmsg_pushstr(reply, "ok");
        zclock_log("rule %s completely destroyed", rule_id);
      } else {
        // not there!
        zclock_log("Received delete trigger request for nonexistent rule %s, ignoring", rule_id);
        zmsg_pushstr(reply, "rule not found");
      }
    } else if (strcmp(command, "AddMonitor")==0) {
      // unconditionally fork a monitor for each line
      // they'll die when they get a channel change
      int i;
      for(i=1; i<4; i++) { 
        monitorconfig_t * mconf = malloc(sizeof(monitorconfig_t));
        mconf->line_id = i;
        mconf->source_worker = servicename;
        mconf->out_socket = config->base_config->portwatcher_endpoint;
        mconf->channel = channel;
        void * monitor_pipe = zthread_fork(context, watch_port, (void*)mconf);
        send_sync("ping", monitor_pipe);
        recv_sync("pong", monitor_pipe);
        zsocket_destroy(context, monitor_pipe);
      }
      zmsg_pushstr(reply, "ok");
    } else {
      zclock_log("Can't handle command %s: ignoring", command);
    }
    zmsg_destroy(&request);
  }
  mdwrk_destroy (&session);
  return;
}
