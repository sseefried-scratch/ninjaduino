#include <czmq.h>
#include "mdwrkapi.h"
#include "utils.h"

int camera_connected() {
  return 1;
}



void camera(void * cvoid, zctx_t * context, void * pipe) {
  zclock_log("worker trying to connect!");
  child_handshake(pipe);
  mdwrk_t * session = NULL;
  while(1) {
    // loop * sleep until  camera is connected
    while(!camera_connected()) {
      if(session) {
        mdwrk_destroy(&session);
        session = NULL;
      }
      sleep(1);
    }
    if(session==NULL)
      session = mdwrk_new ("tcp://10.10.50.60:5555", "n:1234:camera", 1);
    zclock_log("worker connected!");
    
    zmsg_t *reply = NULL;
    while (1) {
      zmsg_t *request = mdwrk_recv (session, &reply);
      if (request == NULL)
        break;              //  Worker was interrupted
      
      char * command = zmsg_popstr(request);
      if (strcmp(command, "NewTrigger") == 0) {
        zclock_log("new trigger!");
      } else {
        zclock_log("Can't handle command %s: ignoring", command);
      }
      reply = zmsg_new();
      zmsg_pushstr(reply, "ok");
      zmsg_destroy(&request);
      
    }
  }
  mdwrk_destroy (&session);
  return;
}
