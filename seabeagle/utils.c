#include <czmq.h>

void child_handshake(void*pipe) {
  zmsg_t * in = zmsg_recv(pipe);
  if(!in) {
    zclock_log("quitting in child handshake!");
    exit(1);
  }
  assert(zmsg_size(in) == 1);
  zframe_t* ping = zmsg_pop(in);
  assert(zframe_streq(ping, "ping"));
  zmsg_destroy(&in);
  zframe_destroy(&ping);
  zmsg_t * out = zmsg_new();
  zmsg_pushstr(out, "pong");
  zmsg_send(&out, pipe);
}


void parent_handshake(void*pipe) {
  assert(pipe);
  zmsg_t * out = zmsg_new();
  zmsg_pushstr(out, "ping");
  zmsg_send(&out, pipe);

  zmsg_t * in = zmsg_recv(pipe);
  if(!in) {
    zclock_log("quitting in parent handshake!");
    exit(1);
  }
  assert(zmsg_size(in) == 1);
  zframe_t* pong = zmsg_pop(in);
  assert(zframe_streq(pong, "pong"));
  zmsg_destroy(&in);
  zframe_destroy(&pong);

}


