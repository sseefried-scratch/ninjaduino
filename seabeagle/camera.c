#include <czmq.h>
#include "mdwrkapi.h"
#include "utils.h"
#include "config.h"

int camera_connected() {
  struct stat   buffer;   
  return (stat ("/dev/video0", &buffer) == 0);
}



void camera(config_t * config) {

  char workername[512];
  void * data = malloc(2000000); // 2 mb enough?
  sprintf(workername, "%s:camera", config->identity);
  zclock_log("worker %s trying to connect!", workername);

  zmsg_t *reply = NULL;
  while(1) {

    while(!camera_connected()) {
      sleep(1);
    }
    // camera is now connected
    mdwrk_t * session = mdwrk_new (config->broker_endpoint, workername, 0);    

    while(camera_connected()) {
      
      zmsg_t *request = mdwrk_recv (session, &reply);
      if (request == NULL) {
        mdwrk_destroy(&session);
        break;
      }

      reply = zmsg_new();
      char * command = zmsg_popstr(request);
      if (strcmp(command, "TakePicture") == 0) {
        // still a race condition here. would be lovely if uvccapture
        // didn't segfault. FIX when we have time
        if (0!=system("rm snap.jpg; [ -e /dev/video0 ] &&  uvccapture")) {
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
    }
    mdwrk_destroy (&session);
    zclock_log("camera became unavailable");
    zmsg_pushstr(reply, "no camera");
  }
}
