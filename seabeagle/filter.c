#include "line.h"
#include "cJSON.h"
#include "zhelpers.h"
#include "filter.h"

void setup_line(line_t * lines, int i, char * identity, char * remote, void * context){
  // do we need to realloc lines? let''s assume not - set MAXLINES for
  // now.
  
  // some temporary buffer space
  char * endpoint = "inproc://linexxxx";
  sprintf(endpoint, "inproc://line%4d", i);
  // set up structure for that line
  line_t * line = &lines[i];
  int line_pid = fork();

  if (line_pid != 0) {
    line->pid = line_pid;
    // what's the protocol here? does the line ever talk back?
    // say no for the moment...
    line->socket = zmq_socket(context, ZMQ_PUB); 
    zmq_connect(line->socket, endpoint);
    // and returns.
  } else {
    void * line_sock = zmq_socket(context, ZMQ_SUB);
    zmq_bind(line_sock, endpoint);
    line_listener(i, line_sock, identity, remote);
    // only returns on shutdown. not sure why it would, but
    // let's be safe.
    zmq_close(line_sock);
    exit(0);
  }
}

int MAXLINES=20;

void filter(void* context, void * serial, char * identity, char * remote) {
  // zmq_pollitem_t  * items = malloc(1 * sizeof(zmq_pollitem_t));
  zmq_setsockopt (serial, ZMQ_SUBSCRIBE, "", 1);
  zmq_msg_t msg;
  zmq_msg_init(&msg);
  int i;
  line_t lines[MAXLINES];
  memset(lines, 0, MAXLINES); // should that be '\0'?
  while(1) {
    // this will probably have to be a poll.
    // we need to listen to the serial port, at least,
    // and then we need to be able to tell the line to turn on and
    // off.
    
    // alternatively: include a message_type and just let the worker
    // handle connecting to the line processes directly...

    zmq_recv(serial, &msg, 0);
    // ephemeral storage, but we'll copy what we want out of
    // the json sturcture.

    cJSON * root = cJSON_Parse(zmq_msg_data(&msg));
    cJSON * port = cJSON_GetObjectItem(root, "ports")->child;
    for(i=0; port; i++) {
      char * type = cJSON_GetObjectItem(port, "type")->valuestring;
      char * value = cJSON_GetObjectItem(port, "value")->valuestring;
      // this is pretty sketchy, but how else do we indicate that
      // the line hasn't been initialised yet?
      if (lines[i].pid==0){
        setup_line(lines, i, identity, remote, context);
      }
      void * sock = lines[i].socket;
      s_sendmore(sock, SERIAL_UPDATE);
      s_sendmore(sock, type);
      s_send(sock, value);
      port = port->next;
    }
    cJSON_Delete(root);
  }
}
