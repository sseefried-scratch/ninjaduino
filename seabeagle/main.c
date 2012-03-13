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

int main() {

  config_t config;
  // yes we need a better way of handling this.
  config.identity = "n:1234";

  // same for this
  config.broker_endpoint = "tcp://launch.ninjablocks.com:5773";
  config.portwatcher_endpoint = "tcp://launch.ninjablocks.com:5774";
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
  void * worker_pipe = zthread_fork(context, worker, (void *) &config);
  parent_handshake(worker_pipe);

  // FIX do we have to wait for the child threads to finish?
  sleep(100);
  exit(0);
}
