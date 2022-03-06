#include <xinu.h>
devcall ppdclose(struct dentry *devptr){
  int rv = 0;
  struct pipe * piptr;
  piptr = & pipes[devptr->dvminor];
  if(piptr -> pstatus == PIPE_FREE){
    rv= SYSERR;
  }
  else{
    intmask mask = disable();
    rv = OK;
     if (piptr -> pstatus == PIPE_RW) {
       wait(piptr-> Producer);
  wait(piptr-> Mutex);
  piptr -> pstatus = PIPE_R;
   piptr -> buff[piptr->windex] = EOF_INDICATOR;
   signal(piptr-> Mutex);
            signal(piptr-> Consumer);
     }
    else{
      piptr->pstatus = PIPE_FREE;
      semreset(piptr -> Producer, PIPE_LEN);
      semreset(piptr -> Consumer, 0);
      semreset(piptr -> Mutex,1);
    }
    restore(mask);
  }
  return rv;
}