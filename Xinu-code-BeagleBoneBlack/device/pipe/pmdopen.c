#include <xinu.h>
devcall pmdopen(struct dentry *devptr, char *name, char *mode){
int i =0;
struct pipe * piptr;
for(i = 0; i< NPIPES; i++){
piptr = & pipes[i];
piptr -> rindex = 0;
  piptr -> windex = 0;
if(piptr -> pstatus == PIPE_FREE){
    piptr -> pstatus = PIPE_RW;
    return i+29;
}
}
return SYSERR;
}