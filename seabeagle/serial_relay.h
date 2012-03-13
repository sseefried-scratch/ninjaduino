
#ifndef __SERIAL_RELAY_H__
#define __SERIAL_RELAY_H__
#include <czmq.h>

typedef struct {
  FILE * in;
} serialconfig_t;
void read_serial(void * in, zctx_t * context, void * pipe) ;
#endif
