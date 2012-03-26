#ifndef __CONFIG_H__
#define __CONFIG_H__
typedef struct {
  char * identity;
  char * broker_endpoint;
  char * portwatcher_endpoint;
  char * registration_endpoint;
  // void * context;
  
} config_t;

int get_config(config_t * config);
#endif
