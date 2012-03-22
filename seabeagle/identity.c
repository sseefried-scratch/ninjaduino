#include <czmq.h>
#include "config.h"

/* this function connects to the externally visible endpoints
 * and hangs until we get a response.
 */

char * get_identity(zctx_t * context, config_t* config) {
  void * socket = zmq_socket(context, ZMQ_REQ);
  zmq_connect(socket, config->registration_endpoint);
  zstr_send(socket, "REGISTER");
  
  // block here until we get a sensible response, dammit.
  char * result = zstr_recv(socket);
  return result;
}
