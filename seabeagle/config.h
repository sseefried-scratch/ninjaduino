#ifndef __CONFIG_H__
#define __CONFIG_H__
typedef struct {
  char * identity;
  char * broker_endpoint;
  char * portwatcher_endpoint;
  void * context;

} config_t;
#endif
