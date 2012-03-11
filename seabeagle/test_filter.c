#include "serial_relay.h"
#include <zmq.h>
#include <stdlib.h>
#include "filter.h"

int main() {
  int serial_pid, filter_pid;

  void *context = zmq_init (1);

  // yes we need a better way of handling this.
  char * identity = "n:1234"; 

  // same for this
  char * remote = "tcp://launch.ninjablocks.com:5773";
  
  puts("filter reporting\n");
  void * serial_reader = zmq_socket(context, ZMQ_SUB);
  zmq_connect (serial_reader, "inproc://read_serial");
  // filter(identity, serial_reader, remote, context);

}
