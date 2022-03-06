#include <xinu.h>
devcall ppdwrite(struct dentry *devptr, char *buff, int32 count){
    int i;
  int counts = 0;
  int32 returned = 0;
  for (i = 0; i < count; i++) {
    returned = ppdputc(devptr, buff[i]);
    if ((returned == SYSERR)) {
      break;
    }
    counts++;

  }
  if(i==0){
    return returned;
  }
  return counts;
}