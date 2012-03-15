/* the purpose of this driver is to allow a haskell app to start
 * up a zeromq socket on an arbitrary port and send shit back and
 * forth. it's assumed that we link it with an object "thread_under_test"
 */
#include <czmq.h>

zthread_attached_fn thread_under_test;

void main(int argc, char ** argv) {
  assert(argc==4);

  char * port = argv[1];
  char * port_type = argv[2];
  char * bind = argv[3];



}
