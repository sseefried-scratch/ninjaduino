#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <czmq.h>

typedef struct {
  char * identity;
  char * broker_endpoint;
  char * portwatcher_endpoint;
  char * registration_endpoint;
  // void * context;
  
} config_t;

int get_config(zctx_t * context, config_t * config);
#endif
