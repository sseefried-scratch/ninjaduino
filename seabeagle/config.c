#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sqlite3.h>
#include <czmq.h>

char * after_colon(char * buf) {
  while(*buf) {
    if(*buf == ':')  {
      buf++;
      while(*buf == ' ') buf++;
      char * tmpbuf = buf;
      while(*tmpbuf && *tmpbuf!='\n') tmpbuf++;
      
      *tmpbuf='\0';
      return buf;
    }
    buf++;
  };
  return NULL;
}

int parse_config(config_t * config, FILE * c) {
  char *buf=NULL;
  ssize_t nbytes=2047;
  //  FILE * c = fopen("/etc/seabeagle.conf", "r");
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

int get_identity_from_broker(zctx_t * context, config_t * config) {
  return 1;
}

int config_callback(void *cvoid, int argc, char **argv, char **column){
  config_t * config = (config_t *) cvoid;
  if(argc == 0) {
    // don't want to abort the call, just return.
    return 0;
  }
  fprintf(stderr, "argc is %d\n", argc);
  config->identity = strdup(argv[0]);
  config->broker_endpoint = strdup(argv[1]);
  config->portwatcher_endpoint = strdup(argv[2]);
  return 0;
}

int retrieve_config(sqlite3 *db, config_t * config) {
  int rc;
  char * zErrMsg;
  rc = sqlite3_exec(db, "select identity,broker,portwatcher from config", config_callback, config, &zErrMsg);
  if(rc!=0) {
    zclock_log("couldn't write to db, giving up: errno %d, msg %s", rc, zErrMsg);
    return 1;
  }
  return 0;
}
int record_config(sqlite3 * db, config_t * config) {
    // put it back in!
    char sqlbuf[2048];
    char * zErrMsg;
    int rc;
    sprintf(sqlbuf, "insert into config (identity, broker, portwatcher) values ('%s', '%s', '%s');",
            config->identity, config->broker_endpoint, config->portwatcher_endpoint);
    printf("%s\n", sqlbuf);
    rc = sqlite3_exec(db, sqlbuf, NULL, NULL, &zErrMsg);
    if(rc!=0) {
      zclock_log("couldn't write to db, giving up: errno %d, msg %s", rc, zErrMsg);
      return 1;
    }
}

int create_db(sqlite3 * db) {
  char * zErrMsg;
  int rc = sqlite3_exec(db, "create table if not exists config (identity string, broker string, portwatcher string);", NULL, NULL, &zErrMsg);
  if (rc!=0) {
    zclock_log("couldn't create db tables, giving up: errno %d, msg %s", rc, zErrMsg);
  } else {
    return 0;
  }
}

int get_config(zctx_t * context, config_t * config) {
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
  create_db(db);
  retrieve_config(db, config);

  // Our only piece of local configuration outside the database is the 
  // block registrar. First thing we do is to connect to it, possibly
  // with the identity pulled from the db, and confirm/get a new id.

  // this should also mean that it will be possible to write
  // all-encompassing testing code.

  if(!config->identity) {
    // not configured yet, go into registration TODO
    //  this is a bit awkward: we need to have the endpoint we're
    //  going to ask for, at the very least. perhaps this is the only
    //  thing that should live in the file config.
   
    // also, even for registered ninjablocks, we should still get the
    // broker/portwatcher/whatever endpoints from the registrar.
    // that way, if the broker falls out of communication for a long
    // time, we can ask the registrar for new details.
    
    
    if(0!=get_identity_from_broker(context, config)) {
      fprintf(stderr, "failed to get an identity from the broker");
      exit(1);
    }
    // for the moment, just read off the config file.
    fprintf(stderr, "nothing in db, let's have a go from file\n");
    FILE * c = fopen("/etc/seabeagle.conf", "r");
    if(!parse_config(config, c)) {
      fprintf(stderr, "bad config\n");
      exit(1); 
    }
    record_config(db, config);
  }

  fprintf(stderr, "rc is %d\n", rc);
  printf("identity is |%s|\n", config->identity);
  printf("broker is  |%s|\n", config->broker_endpoint);
  printf("port watcher is at |%s|\n", config->portwatcher_endpoint);
  return 0;
}
