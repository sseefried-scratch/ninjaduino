#ifndef __IDENTITY_H__
#define __IDENTITY_H__
#include <czmq.h>
#include "config.h"
char * get_identity(zctx_t *, config_t *);

#endif
