
#ifndef __TRIGGER_H__
#define __TRIGGER_H__
#include <czmq.h>

typedef struct {
  char * channel;
} triggerconfig_t;
typedef struct {
  int ready;
  int trigger_level;
  int reset_level;
} triggermemory_t;



typedef struct {
  char * trigger_name;
  int (*trigger)( triggermemory_t*, int); 
  int line_id;
  int trigger_level;
  int reset_level;
} trigger_t;

// a trigger function is one that takes a trigger_memory and a new value,
// then returns a bool to say whether or not it fired. may edit the
// trigger_memory in place.


typedef int (*triggerfunction) (triggermemory_t *, int value); // C
triggerfunction find_trigger(char * channel, char * triggername);

void trigger(void *config, 
             zctx_t * context, 
             void * pipe );
#endif
