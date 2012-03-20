#include <czmq.h>
#include "mdwrkapi.h"
#include "utils.h"
#include "config.h"

int camera_connected() {
  return 1;
}



void camera(void * cvoid, zctx_t * context, void * pipe) {

  child_handshake(pipe);
  config_t * config = (config_t*) cvoid;
  char workername[512];
  void * data = malloc(2000000); // 2 mb enough?
  sprintf(workername, "%s:camera", config->identity);
  zclock_log("worker %s trying to connect!", workername);
  mdwrk_t * session = mdwrk_new (config->broker_endpoint, workername, 0);
  zmsg_t *reply = NULL;
  while(1) {
    zmsg_t *request = mdwrk_recv (session, &reply);
    if (request == NULL)
      break;              //  Worker was interrupted
    reply = zmsg_new();
    if(camera_connected()) {
      char * command = zmsg_popstr(request);

      if (strcmp(command, "TakePicture") == 0) {
        // if (!system("rm snap.jpg; uvccapture")) {
        if (0!=system("rm snap.jpg; uvccapture")) {
          zclock_log("error taking photo");
        } else {
          
        }
        
      } else {
        zclock_log("Can't handle command %s: ignoring", command);
      }
      FILE * f = fopen("snap.jpg", "r");
      int bytes_read = fread(data, 1, 2000000, f);
      zmsg_pushmem(reply, data, bytes_read);
      zmsg_destroy(&request);
    } else {
      zclock_log("camera was unplugged while we were waiting for a request");
      zmsg_pushstr(reply, "no camera");
    }
    zmsg_destroy(&request);

  }
  mdwrk_destroy (&session);
  
  return;
}
