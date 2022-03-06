#include <xinu.h>
devcall ppdputc(struct dentry *devptr, char ch){
  int i;
struct pipe * piptr;
  piptr = &pipes[devptr->dvminor];
  //kprintf("minor:%d",devptr->dvminor );
  wait(piptr-> Producer);
  wait(piptr-> Mutex);
 // //kprintf("put: %x",c);
    if (piptr -> pstatus != PIPE_RW) {
      signal(piptr-> Mutex);
      signal(piptr-> Producer);
      return SYSERR;
    } else {
      /*for (write ; write < PIPE_LEN; write++) {
        if (piptr -> buff[write] == 0) {
          piptr -> buff[write] = c & 0xff;
          for (write; write < PIPE_LEN; write++) {
            piptr -> buff[write] = 0;
          }
          break;
        }*/

      //piptr -> buff[write] = c & 0xff;
      piptr -> buff[piptr -> windex] = ch & 0xff;
      piptr -> windex += 1;
      piptr -> windex = piptr -> windex % PIPE_LEN;
      //kprintf("put: %x",piptr -> buff[piptr -> windex-1] );
      signal(piptr-> Mutex);
      signal(piptr-> Consumer);
      
      return OK;
    }


}