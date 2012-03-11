#ifndef __MONITOR_H__
#define __MONITOR_H__
#include "czmq.h"

void watch_port(void *config, 
                zctx_t * context, 
                void * pipe ) ;
#endif
