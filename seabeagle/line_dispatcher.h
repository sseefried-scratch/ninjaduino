#ifndef __FILTER_H__
#define __FILTER_H__

#include <czmq.h>
void line_dispatcher(void * cvoid, zctx_t * context, void * pipe);
#endif
