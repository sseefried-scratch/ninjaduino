#ifndef __LINE_H__
#define __LINE_H__
enum line_message_type { SERIAL_UPDATE, 
                         MONITOR_ON, MONITOR_OFF,
                         TRIGGER_ON, TRIGGER_OFF
};

typedef struct {
  void * socket;
  int pid;
} line_t;

void line_listener(int line_id, void * subscriber, char * identity, char * endpoint);
#endif
