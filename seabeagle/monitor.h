#ifndef __MONITOR_H__
#define __MONITOR_H__
#include "czmq.h"

typedef struct {
  char * source_worker; // is this n:1234 or n:1234:$channeL?
  int line_id;
  char * listen_socket;
  char * out_socket;
  char * channel;
} monitorconfig_t;

void watch_port(void *config, 
                zctx_t * context, 
                void * pipe ) ;
#endif
