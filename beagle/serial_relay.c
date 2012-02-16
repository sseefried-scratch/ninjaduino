//
//  file read server 
//  Binds PUB socket to tcp://*:5556
//
#include "zhelpers.h"

int main (void)
{
    char * buf;
    int nbytes=2047;
    int linger = 0; 
    //  Prepare our context and publisher
    void *context = zmq_init (1);
    void *publisher = zmq_socket (context, ZMQ_PUB);
    zmq_setsockopt(publisher, ZMQ_LINGER, &linger, sizeof(int));
    zmq_bind (publisher, "tcp://*:5556");
    buf = (char *) malloc(nbytes+1) ;
    fprintf(stderr, "bound\n");
    // first line is likely to be garbage
    getline(&buf, &nbytes, stdin);
    
    fprintf(stderr, "first garbage line %s\n", buf);
    while ( getline(&buf, &nbytes, stdin) != -1 ) {
//        puts(buf);
        s_send (publisher, buf);
    }
    // fprintf(stderr, "error reading from stdin\n");
    zmq_close (publisher);
    zmq_term (context);
    return 0;
}
