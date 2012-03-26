#include "serial_relay.h"
#include <zmq.h>
#include <czmq.h>
#include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <stdio.h>
#include "config.h"
#include "filter.h"
#include "utils.h"
#include "worker.h"
#include "worker_config.h"
#include "camera.h"
#include "identity.h"


int main() {
  config_t config;

  zctx_t * context = zctx_new();
  // let's assume the magical code fairies Do The Right Thing
  // in terms of talking to the server.
  get_config(&config);
  // we don't want the camera sharing any thread info, because it'll
  // be running system etc, and that seems to screw with the threads.
  if (!fork()) {
    camera(&config);
    return 0;
  }

  /*  The serial thread just reads from /dev/ttyO1 and publishes to an
   *  inproc socket. This could be done inline, but now we can test
   *  these separately
   */
  serialconfig_t t = { stdin };
  zclock_log("Starting serial relay");
  void * serial_pipe = zthread_fork(context, &read_serial, &t);
  parent_handshake(serial_pipe);
  zclock_log("Started serial relay!");

  zclock_log("Starting line dispatcher");
  void * filter_pipe = zthread_fork(context, line_dispatcher, NULL);
  parent_handshake(filter_pipe);
  zclock_log("Started line dispatcher");
  // separate worker threads
  zclock_log("starting workers");



  // we also start workers for the analog lines. These are easy to
  // monitor, we can just pass a channel name.
  char * internal_channels[3] = { "distance", "light", "button" };
  int i;
  for(i=0;i<3;i++) {
    workerconfig_t * wconf = malloc(sizeof(workerconfig_t));
    wconf->base_config = &config;
    wconf->channel = internal_channels[i];
    parent_handshake(zthread_fork(context, generic_worker, (void*) wconf));    
  }

  // FIX We have to wait for the child threads to finish, but this
  // can't be the best way.
  while(1) sleep(100);
  exit(0);
}


