//
//  file read server 
//  Binds PUB socket to tcp://*:5556
//
#include "zhelpers.h"

void read_serial(void * socket, FILE * in) {
    char * buf;
    size_t nbytes=2047;
    //  Prepare our context and publisher
    buf = (char *) malloc(nbytes+1) ;
    fprintf(stderr, "bound\n");
    // first line is always garbage
    getline(&buf, &nbytes, in);
    while ( getline(&buf, &nbytes, in) != -1 ) {
#ifdef DEBUG
      puts(buf);
#endif
      s_send (socket, buf);
    }
    fprintf(stderr, "error reading from stdin\n");
}
