#include <czmq.h>
#include "utils.h"

void child_handshake(void*pipe) {
  recv_sync("ping", pipe);
  send_sync("pong", pipe);
}


void parent_handshake(void*pipe) {
  send_sync("ping", pipe);
  recv_sync("pong", pipe);
}

void recv_sync(char * expected, void * pipe) {
  assert(pipe);
  assert(expected);
  zmsg_t * in = zmsg_recv(pipe);
  if(!in) {
    zclock_log("quitting in recv_sync handshake!");
    exit(1);
  }
  assert(zmsg_size(in) == 1);
  zframe_t* ping = zmsg_pop(in);
  assert(zframe_streq(ping, expected));
  zmsg_destroy(&in);
}

void send_sync(char * msg, void * pipe) {
  assert(msg);
  assert(pipe);
  zstr_send(pipe, msg);
}

char * to_linesocket(int l) {
  char * socketname = malloc(20);
  sprintf(socketname, "inproc://line%04d", l);
  return socketname;
}

char * to_line(int l) {
  char * topic = malloc(9);
  sprintf(topic, "line%04d", l);
  return topic;
}

