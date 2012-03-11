#ifndef __TRIGGER_H__
#define __TRIGGER_H__
#include <czmq.h>
void trigger(void *config, 
             zctx_t * context, 
             void * pipe );
#endif
