#ifndef __MONITOR_H__
#define __MONITOR_H__
#include "czmq.h"

typedef struct {
  char * source_worker; // n:1234:distance
  int line_id;
  char * channel;       // distance
  char * out_socket;    // tcp://au.ninjablocks.com:5773
} monitorconfig_t;

void watch_port(void *config, 
                zctx_t * context, 
                void * pipe ) ;
#endif
