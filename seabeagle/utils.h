
#ifndef __UTILS_H__
#define __UTILS_H__
void parent_handshake(void*pipe);
void child_handshake(void*pipe);
void send_sync(char * msg, void * pipe);
void recv_sync(char * msg, void * pipe);

#endif
