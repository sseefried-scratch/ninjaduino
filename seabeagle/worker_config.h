#ifndef __WORKER_CONFIG_H__
#define __WORKER_CONFIG_H__
#include "config.h"
typedef struct {
  config_t * base_config;
  char * channel;
} workerconfig_t;
#endif
