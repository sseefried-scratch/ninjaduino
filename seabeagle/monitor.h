#ifndef __MONITOR_H__
#define __MONITOR_H__
#include "czmq.h"
#include "config.h"


void watch_port(config_t *config, 
                zctx_t * context, void * pipe ) ;
#endif
