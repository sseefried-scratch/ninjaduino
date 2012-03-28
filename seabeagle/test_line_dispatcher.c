#include "serial_relay.h"
#include <czmq.h>
#include <zmq.h>
#include <stdlib.h>
#include "line_dispatcher.h"

int test_line_dispatcher() {
  int serial_pid, filter_pid;

  void *context = zctx_new();

  void * serial_in = zsocket_new(context, ZMQ_PUB);
  zsocket_bind (serial_in, "inproc://raw_serial");
  // filter(identity, serial_reader, remote, context);

  zclock_log("pre-fork");
  void * pipe = zthread_fork(context, line_dispatcher, NULL);
  // temp
  sleep(1);
  zclock_log("post-fork");
  zclock_log("parent handshake...");
  parent_handshake(pipe);
  zclock_log("parent handshake");

  // should be ok to receive events now.
  void * line_out = zsocket_new(context, ZMQ_SUB);
  zsockopt_set_subscribe(line_out, "line0001");
  zsocket_connect(line_out, "inproc://line");
  
  char * sample = "{\"ports\": [{\
                          \"type\": \"DISTANCE\",   \
                          \"value\": 12,\
                          \"port\": 1\
                          }]}";
  zclock_log("sending %s", sample);
  zstr_send(serial_in, sample);
  zclock_log("testing parent sent - waiting for output on lineout");
  zmsg_t * output = zmsg_recv(line_out);
  zclock_log("lineout response!");
  assert(zmsg_size(output) == 3);
  assert(strcmp(zmsg_popstr(output), "line0001")==0);
  assert(strcmp(zmsg_popstr(output), "distance")==0);
  zframe_t * val = zmsg_pop(output);
  assert(zframe_size(val) == sizeof(int));
  int v = *(int*)zframe_data(val);
  assert(v==12);
  zclock_log("test all done, now testing shutdown protocol");
  // exit(0);
  // FIX shut it down properly
  zstr_send(pipe, "DESTROY");
  // really want a poll here, and to fail if it doesn't quit within a
  // few seconds.
  char * response = zstr_recv(pipe);
  zclock_log(response);
  assert(strcmp("OK", response)==0);
  
}
