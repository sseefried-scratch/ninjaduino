#ifndef __TRIGGER_H__
#define __TRIGGER_H__
#include <czmq.h>

typedef struct {
  char * channel;
} triggerconfig_t;

void trigger(void *config, 
             zctx_t * context, 
             void * pipe );
#endif
