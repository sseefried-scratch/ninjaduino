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

int main() {

  config_t config;
  // yes we need a better way of handling this.
  config.identity = "n:1234";

  // same for this
  config.broker_endpoint = "tcp://au.ninjablocks.com:5773";
  config.portwatcher_endpoint = "tcp://au.ninjablocks.com:5775";
  zctx_t * context = zctx_new();
  /*  The serial thread just reads from /dev/ttyO1 and publishes to an
   *  inproc socket. This could be done inline, but now we can test
   *  these separately
   */
  serialconfig_t t = { stdin };
  void * serial_pipe = zthread_fork(context, &read_serial, &t);
  parent_handshake(serial_pipe);
  void * filter_pipe = zthread_fork(context, line_dispatcher, NULL);
  parent_handshake(filter_pipe);

  // separate worker threads
  zclock_log("starting workers");

  zthread_attached_fn * independent_workers[2] = {camera};

  int i;
  
  for(i = 0; i<1; i++ ) {
    parent_handshake(zthread_fork(context, independent_workers[i], (void*) &config));
  }

  // we also start workers for the analog lines. These are easy to
  // monitor, we can just pass a channel name.
  char * internal_channels[3] = { "distance", "light", "button" };
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


