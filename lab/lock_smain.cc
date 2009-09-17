#include "rpc.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include "lock_server.h"

// Main loop of lock_server

int
main(int argc, char *argv[])
{
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  srandom(getpid());

  if(argc != 2){
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(1);
  }
  


#ifndef RSM
  lock_server ls;
  rpcs server(htons(atoi(argv[1])));
  server.reg(lock_protocol::stat, &ls, &lock_server::stat);
#endif


  while(1)
    sleep(1000);
}
