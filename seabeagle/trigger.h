
#ifndef __TRIGGER_H__
#define __TRIGGER_H__
#include <czmq.h>

typedef struct {
  char * channel;
  char * rule_id;
  char * trigger_name;
  char * target_worker;
  //experiment
  char * auth;  char * addins;

  //zframe_t * auth;  zframe_t * addins;
  
} triggerconfig_t;
typedef struct {
  int ready;
  int trigger_level;
  int reset_level;
  int line_id;
} triggermemory_t;

int create_triggerconfig(triggerconfig_t * conf,
                         zmsg_t * rule_details,
                         char * channel,
                         char * rule_id);

/* typedef struct { */
/*   char * trigger_name; */
/*   int (*trigger)( triggermemory_t*, int);  */

/*   int trigger_level; */
/*   int reset_level; */
/* } trigger_t; */

// a trigger function is one that takes a trigger_memory and a new value,
// then returns a bool to say whether or not it fired. may edit the
// trigger_memory in place.


typedef int (*triggerfunction) (triggermemory_t *, int value); // C
triggerfunction find_trigger(char * channel, char * triggername);

void trigger(void *config, 
             zctx_t * context, 
             void * pipe );

#endif
