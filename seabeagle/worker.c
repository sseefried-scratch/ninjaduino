#include <czmq.h>
#include "mdwrkapi.h"
#include "utils.h"
#include "trigger.h"
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
   | "AddTrigger"            |
   | rule_id: integer        |
   | trigger_name: string    |
   | target_service: string  |
   | auth: msgpack           |
   | addins: msgpack         |
   |-------------------------|
     - addins must contain "rule_id";
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


void generic_worker(void * cvoid, zctx_t * context, void * pipe) {
  zhash_t * rules = zhash_new();
  zclock_log("worker trying to connect!");
  mdwrk_t *session = mdwrk_new ("tcp://10.10.50.60:5555", "echo", 1);
  zclock_log("worker connected!");
  child_handshake(pipe);
  zmsg_t *reply = NULL;
  void * rule_pipe = NULL;
  while (1) {
    zmsg_t *request = mdwrk_recv (session, &reply);
    if (request == NULL)
      break;              //  Worker was interrupted

    char * command = zmsg_popstr(request);
    char * rule_id = zmsg_popstr(request);
    if (strcmp(command, "AddTrigger") == 0) {
      zclock_log("new trigger!");
      if (zhash_lookup(rules, rule_id)) {
        // already have a rule with that id! WTF??
        // FIXME should probably delete this and reinstate
        zclock_log("Received duplicate rule %s, ignoring", rule_id);
        zmsg_destroy(&request);
        reply = zmsg_new();
        zmsg_pushstr(reply, "duplicate");
      } else {
        // start a new rule thread
        zmsg_pushstr(request, rule_id);
        void * pipe = zthread_fork(context, trigger, NULL);
        zmsg_send(pipe, request);
        recv_sync("ok", pipe);
        zhash_insert(rules, rule_id, pipe);
        zmsg_t * pipe_resp = zmsg_recv(pipe);
        zmsg_destroy(&pipe_resp);
        zmsg_destroy(&request);

      }
    } else if (strcmp(command,"DeleteTrigger") == 0) {
      if (rule_pipe=zhash_lookup(rules, rule_id)) {
        // found it
        send_sync("DESTROY",pipe);
        recv_sync("OK", pipe);
        zsocket_destroy(context, pipe);
        zhash_delete(rules, rule_id);
        zmsg_destroy(&request);
      } else {
        // not there!
        zclock_log("Received delete trigger request for nonexistent rule %s, ignoring", rule_id);
        reply = zmsg_new();
        zmsg_pushstr(reply, "rule not found");
        zmsg_destroy(&request);
      }
    } else if (strcmp(command, "AddMonitor")==0) {

    }






    } else {
      zclock_log("Can't handle command %s: ignoring", command);
    }
    reply = zmsg_new();
    zmsg_pushstr(reply, "ok");
    zmsg_destroy(&request);

  }
  mdwrk_destroy (&session);
  return;
}
