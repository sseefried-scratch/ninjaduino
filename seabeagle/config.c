#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char * after_colon(char * buf) {
  while(*buf) {
    if(*buf == ':')  {
      buf++;
      while(*buf == ' ') buf++;
      char * tmpbuf = buf;
      while(*tmpbuf!='\n') tmpbuf++;
      *tmpbuf='\0';
      return buf;
    }
    buf++;
  };
  return NULL;
}

int parse_config(config_t * config) {
  char *buf=NULL;
  ssize_t nbytes=2047;
  FILE * c = fopen("/etc/seabeagle.conf", "r");
  assert(c);

  config->broker_endpoint=NULL;
  config->portwatcher_endpoint=NULL;
  config->identity=NULL;

  while(-1 != getline(&buf, &nbytes, c)) {
    printf("read |%s|\n", buf);
    if (strncmp("broker:", buf, 7)==0) {
      config->broker_endpoint = after_colon(buf);
    } else if  (strncmp("portwatcher:", buf, 12)==0) {
      config->portwatcher_endpoint = strdup(after_colon(buf));
    } else if  (strncmp("portwatcher:", buf, 12)==0) {
      config->portwatcher_endpoint = strdup(after_colon(buf));
    } else if (strncmp("identity:", buf, 9) == 0) {
      config->identity = strdup(after_colon(buf));

    } else {
      fprintf(stderr, "bad line: %s\n", buf);
      free(buf);
    }
    buf=NULL;
  }
  return (config->identity && config->broker_endpoint && config->portwatcher_endpoint);
}
