#ifndef __LINE_H__
#define __LINE_H__
#include "config.h"
#include <czmq.h>
/* typedef struct { */
/*   void * socket; */
/*   int pid; */
/* } line_t; */


typedef struct {
  char * inpipe;
  char * outpipe;
} lineconfig_t;

typedef enum  { SERIAL_UPDATE, 
                MONITOR_ON, MONITOR_OFF,
                TRIGGER_ON, TRIGGER_OFF
} line_message_type_t;

void line_listener(void * cvoid, zctx_t * context, void * pipe);
#endif
