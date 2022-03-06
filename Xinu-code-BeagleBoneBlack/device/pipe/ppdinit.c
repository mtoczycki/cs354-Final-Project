#include <xinu.h>
devcall ppdinit(struct dentry *devptr){
     int i;
struct pipe * piptr;
    piptr = & pipes[devptr->dvminor];
    piptr -> rindex = 0;
    piptr -> windex = 0;
    int j = 0;
    for (j=0; j < PIPE_LEN; j++) {
      piptr -> buff[j] = 0;
    }
    piptr -> pstatus = PIPE_FREE;
    piptr -> Producer = semcreate(PIPE_LEN);
    piptr -> Consumer = semcreate(0);
    piptr -> Mutex = semcreate(1);
  }