#include "serial_relay.h"
#include <zmq.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
  int serial_pid, filter_pid;

  void *context = zmq_init (1);

  // yes we need a better way of handling this.
  char * identity = "n:1234"; 

  // same for this
  char * remote = "tcp://launch.ninjablocks.com:5773";
  /*  The serial thread just reads from /dev/ttyO1 and publishes to an
   *  inproc socket. This could be done inline, but now we can test
   *  these separately
   */
     
  if(0 == (serial_pid = fork())){
    // child
    puts("serial reporting\n");
    void *publisher = zmq_socket (context, ZMQ_PUB);
    zmq_bind (publisher, "inproc://read_serial");

    read_serial(publisher, stdin);

    // probably won't get to here.
    exit(0);
  }

  /* The filter thread is responsible for distributing messages to the
   * correct place. it also starts up a line thread when necessary.
   */
  if(0 == (filter_pid = fork())){
    // filter child    
    puts("filter reporting\n");
    void * serial_reader = zmq_socket(context, ZMQ_SUB);
    zmq_connect (serial_reader, "inproc://read_serial");
    filter(identity, serial_reader, remote);
    // probably won't get to here.
    exit(0);
  }

  waitpid(serial_pid, NULL, 0);
  waitpid(filter_pid, NULL, 0);
  exit(0);
}
