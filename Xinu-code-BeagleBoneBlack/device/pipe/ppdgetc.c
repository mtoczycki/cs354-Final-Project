
#include <xinu.h>
devcall ppdgetc(struct dentry *devptr)
{
      int i;
struct pipe * piptr;
   piptr = &pipes[devptr->dvminor];
  wait(piptr-> Consumer);
  wait(piptr-> Mutex);
  //kprintf("IN GET");
  if (piptr -> pstatus == PIPE_RW || piptr -> pstatus == PIPE_R) {
    if ((piptr -> buff[piptr -> rindex] & 0xffff) == EOF_INDICATOR) {
      signal(piptr-> Mutex);
      signal(piptr-> Consumer);
     
        return EOF;
     
    } else {
      int32 result = (piptr -> buff[piptr -> rindex] & 0x000000ff);
      piptr -> rindex += 1;
      piptr -> rindex = piptr -> rindex % PIPE_LEN;
      signal(piptr-> Mutex);
      signal(piptr-> Producer);
      //kprintf("result: %x", piptr -> buff[piptr -> rindex-1]);
      return result;
    }
  } else {
    return SYSERR;
  }

}