#ifndef __LINE_H__
#define __LINE_H__
#include "config.h"
typedef struct {
  void * socket;
  int pid;
} line_t;


typedef enum  { SERIAL_UPDATE, 
                MONITOR_ON, MONITOR_OFF,
                TRIGGER_ON, TRIGGER_OFF
} line_message_type_t;


void line_listener(int line_id, void * subscriber, config_t * config);
#endif
