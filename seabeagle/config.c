#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sqlite3.h>
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

 
int config_callback(void *cvoid, int argc, char **argv, char **column){
  config_t * config = (config_t *) cvoid;
  if(argc == 0) {
    // not configured yet, go into registration TODO
    // for the moment, just read off the config file.

    return 1;
  }
  fprintf(stderr, "argc is %d\n", argc);
  config->identity = strdup(argv[0]);
  config->broker_endpoint = strdup(argv[1]);
  config->portwatcher_endpoint = strdup(argv[2]);
  return 0;
}

int get_config(config_t * config) {
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  memset(config, '\0', sizeof(config_t));
  rc = sqlite3_open("seabeagle.db", &db);
  if(rc){
    zclock_log("can't open base db");
    sqlite3_close(db);
    return 1;
  }
  rc = sqlite3_exec(db, "create table if not exists config (identity string, broker string, portwatcher string);\
                         select identity,broker,portwatcher from config", config_callback, config, &zErrMsg);
  if(rc!=0) {
    zclock_log("couldn't write to db, giving up: errno %d, msg %s", rc, zErrMsg);
    return 1;
  }
  if(!config->identity) {
    fprintf(stderr, "nothing in db, let's have a go from file\n");
    if(!parse_config(config)) {
      fprintf(stderr, "bad config\n");
      exit(1); 
    }
    // put it back in!
    char sqlbuf[2048];
    sprintf(sqlbuf, "insert into config (identity, broker, portwatcher) values ('%s', '%s', '%s');",
            config->identity, config->broker_endpoint, config->portwatcher_endpoint);
    printf("%s\n", sqlbuf);
    rc = sqlite3_exec(db, sqlbuf, NULL, NULL, &zErrMsg);
    if(rc!=0) {
      zclock_log("couldn't write to db, giving up: errno %d, msg %s", rc, zErrMsg);
      return 1;
    }
  }

  fprintf(stderr, "rc is %d\n", rc);
  printf("identity is |%s|\n", config->identity);
  printf("broker is  |%s|\n", config->broker_endpoint);
  printf("port watcher is at |%s|\n", config->portwatcher_endpoint);
  return 0;
}
