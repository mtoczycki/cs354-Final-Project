#include <xinu.h>
devcall ppdread(struct dentry *devptr, char *buff, int32 count){
    int i;
  int counts = 0;
   int32 returned;
  for (i = 0; i < count; i++) {
    //kprintf("\ni:%d",i);
    returned = ppdgetc(devptr);
   //kprintf("\nreturned:%x",returned);
    if ((returned == EOF || returned == SYSERR)) {
      break;
    } else {
      buff[i] = returned;
      counts++;
    }
  }
  if(i == 0){
    return returned;
  }
  return counts;

}
