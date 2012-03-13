#include <czmq.h>
#include "mdwrkapi.h"
#include "utils.h"

void worker(void * cvoid, zctx_t * context, void * pipe) {

  mdwrk_t *session = mdwrk_new ("tcp://10.10.50.60:5555", "echo", 1);
  zclock_log("worker connected!");
  child_handshake(pipe);
  zmsg_t *reply = NULL;
  while (1) {
    zmsg_t *request = mdwrk_recv (session, &reply);
    if (request == NULL)
      break;              //  Worker was interrupted
    reply = request;        //  Echo is complex… :-)
  }
  mdwrk_destroy (&session);
  return;
}
