/* shell.c  -  shell */

#include <xinu.h>

#include <stdio.h>

#include "shprototypes.h"


/************************************************************************/
/* Table of Xinu shell commands and the function associated with each	*/
/************************************************************************/
const struct cmdent cmdtab[] = {
  {
    "argecho",
    TRUE,
    xsh_argecho
  },
  {
    "arp",
    FALSE,
    xsh_arp
  },
  {
    "cat",
    FALSE,
    xsh_cat
  },
  {
    "clear",
    TRUE,
    xsh_clear
  },
  {
    "date",
    FALSE,
    xsh_date
  },
  {
    "devdump",
    FALSE,
    xsh_devdump
  },
  {
    "echo",
    FALSE,
    xsh_echo
  },
  {
    "exit",
    TRUE,
    xsh_exit
  },
  {
    "help",
    FALSE,
    xsh_help
  },
  {
    "kill",
    TRUE,
    xsh_kill
  },
  {
    "memdump",
    FALSE,
    xsh_memdump
  },
  {
    "memstat",
    FALSE,
    xsh_memstat
  },
  {
    "netinfo",
    FALSE,
    xsh_netinfo
  },
  {
    "ping",
    FALSE,
    xsh_ping
  },
  {
    "ps",
    FALSE,
    xsh_ps
  },
  {
    "sleep",
    FALSE,
    xsh_sleep
  },
  {
    "udp",
    FALSE,
    xsh_udpdump
  },
  {
    "udpecho",
    FALSE,
    xsh_udpecho
  },
  {
    "udpeserver",
    FALSE,
    xsh_udpeserver
  },
  {
    "uptime",
    FALSE,
    xsh_uptime
  },
  {
    "?",
    FALSE,
    xsh_help
  }

};

uint32 ncmd = sizeof(cmdtab) / sizeof(struct cmdent);

/************************************************************************/
/* shell  -  Provide an interactive user interface that executes	*/
/*	     commands.  Each command begins with a command name, has	*/
/*	     a set of optional arguments, has optional input or		*/
/*	     output redirection, and an optional specification for	*/
/*	     background execution (ampersand).  The syntax is:		*/
/*									*/
/*		   command_name [args*] [redirection] [&]		*/
/*									*/
/*	     Redirection is either or both of:				*/
/*									*/
/*				< input_file				*/
/*			or						*/
/*				> output_file				*/
/*									*/
/************************************************************************/

process shell(
    did32 dev /* ID of tty device from which	*/
    ) /*   to accept commands		*/ {
  char buf[SHELL_BUFLEN]; /* Input line (large enough for	*/
  /*   one line from a tty device	*/
  int32 len; /* Length of line read		*/
  char tokbuf[SHELL_BUFLEN + /* Buffer to hold a set of	*/
    SHELL_MAXTOK]; /* Contiguous null-terminated	*/
  /* Strings of tokens		*/
  int32 tlen; /* Current length of all data	*/
  /*   in array tokbuf		*/
  int32 tok[SHELL_MAXTOK]; /* Index of each token in	*/
  /*   array tokbuf		*/
  int32 toktyp[SHELL_MAXTOK]; /* Type of each token in tokbuf	*/
  int32 ntok; /* Number of tokens on line	*/
  pid32 child; /* Process ID of spawned child	*/
  bool8 backgnd; /* Run command in background?	*/
  char * outname, * inname; /* Pointers to strings for file	*/
  /*   names that follow > and <	*/
  did32 stdinput, stdoutput; /* Descriptors for redirected	*/
  /*   input and output		*/
  int32 i; /* Index into array of tokens	*/
  int32 j; /* Index into array of commands	*/
  int32 z; /* Temporary variable */
  int32 p; /* Index into array of pipIDs */
  int32 msg; /* Message from receive() for	*/
  /*   child termination		*/
  int32 tmparg; /* Address of this var is used	*/
  /*   when first creating child	*/
  /*   process, but is replaced	*/
  char * src, * cmp; /* Pointers used during name	*/
  /*   comparison			*/
  bool8 diff; /* Was difference found during	*/
  /*   comparison			*/
  char * args[SHELL_MAXTOK]; /* Argument vector passed to	*/
  /*   builtin commands		*/
  did32 pipIDs[NPIPES]; /* Array for opened pipe devices IDs */
  int32 npip; /* Number of pipes in pipe command */
  int32 retval; /* Return value from a function */

  /* Print shell banner and startup message */

  fprintf(dev, "\n\n%s%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
      SHELL_BAN0, SHELL_BAN1, SHELL_BAN2, SHELL_BAN3, SHELL_BAN4,
      SHELL_BAN5, SHELL_BAN6, SHELL_BAN7, SHELL_BAN8, SHELL_BAN9);

  fprintf(dev, "%s\n\n", SHELL_STRTMSG);

  /* Set the cwd as the root of RAMDISK0 */

  //memcpy((char *)&cwd, (char *)&NAMESPACE.rt0, sizeof(struct inode));

  /* Continually prompt the user, read input, and execute command	*/

  while (TRUE) {

    npip = 0;

    /* Display prompt */

    /* Display prompt */

    fprintf(dev, SHELL_PROMPT);

    /* Read a command */

    len = read(dev, buf, sizeof(buf));
   

    /* Exit gracefully on end-of-file */

    if (len == EOF) {
      break;
    }

    /* If line contains only NEWLINE, go to next line */

    if (len <= 1) {
      continue;
    }

    buf[len] = SH_NEWLINE; /* terminate line */

    /* Parse input line and divide into tokens */

    ntok = lexan(buf, len, tokbuf, & tlen, tok, toktyp);
    /* Handle parsing error */

    if (ntok == SYSERR) {
      fprintf(dev, "%s\n", SHELL_SYNERRMSG);
      continue;
    }

    /* If line is empty, go to next input line */

    if (ntok == 0) {
      fprintf(dev, "\n");
      continue;
    }

    /* If last token is '&', set background */

    if (toktyp[ntok - 1] == SH_TOK_AMPER) {
      ntok--;
      tlen -= 2;
      backgnd = TRUE;
    } else {
      backgnd = FALSE;
    }

    /* Check for input/output redirection (default is none) */

    outname = inname = NULL;
    //kprintf ( " \nbefore : ntok : %d tlen :%d",ntok, tlen);
    if ((ntok >= 3) && ((toktyp[ntok - 2] == SH_TOK_LESS) ||
          (toktyp[ntok - 2] == SH_TOK_GREATER))) {
      if (toktyp[ntok - 1] != SH_TOK_OTHER) {
        fprintf(dev, "1 %s\n", SHELL_SYNERRMSG);
        continue;
      }
      if (toktyp[ntok - 2] == SH_TOK_LESS) {
        inname = & tokbuf[tok[ntok - 1]];
      } else {
        outname = & tokbuf[tok[ntok - 1]];
      }
      ntok -= 2;
      tlen = tok[ntok];
    }

    if ((ntok >= 3) && ((toktyp[ntok - 2] == SH_TOK_LESS) ||
          (toktyp[ntok - 2] == SH_TOK_GREATER))) {
      if (toktyp[ntok - 1] != SH_TOK_OTHER) {
        fprintf(dev, "2 %s\n", SHELL_SYNERRMSG);
        continue;
      }
      if (toktyp[ntok - 2] == SH_TOK_LESS) {
        if (inname != NULL) {
          fprintf(dev, "3 %s\n", SHELL_SYNERRMSG);
          continue;
        }
        inname = & tokbuf[tok[ntok - 1]];
      } else {
        if (outname != NULL) {
          fprintf(dev, "4 %s\n", SHELL_SYNERRMSG);
          continue;
        }
        outname = & tokbuf[tok[ntok - 1]];
      }
      ntok -= 2;
      tlen = tok[ntok];
    }
//kprintf ( " \nafter : ntok : %d tlen :%d",ntok, tlen);
    /* Branch into a normal command and a pipe command */

    if (commtype == SH_COMMNORM) {

      /* Verify remaining tokens are type "other" */

      for (i = 0; i < ntok; i++) {
        if (toktyp[i] != SH_TOK_OTHER) {
          break;
        }
      }
      if ((ntok == 0) || (i < ntok)) {
        fprintf(dev, SHELL_SYNERRMSG);
        continue;
      }

      stdinput = stdoutput = dev;

      /* Lookup first token in the command table */

      for (j = 0; j < ncmd; j++) {
        src = cmdtab[j].cname;
        cmp = tokbuf;
        diff = FALSE;
        while ( * src != NULLCH) {
          if ( * cmp != * src) {
            diff = TRUE;
            break;
          }
          src++;
          cmp++;
        }
        if (diff || ( * cmp != NULLCH)) {
          continue;
        } else {
          break;
        }
      }

      /* Handle command not found */

      if (j >= ncmd) {
        fprintf(dev, "command %s not found\n", tokbuf);
        continue;
      }

      /* Handle built-in command */

      if (cmdtab[j].cbuiltin) {
        /* No background or redirect. */
        if (inname != NULL || outname != NULL || backgnd) {
          fprintf(dev, SHELL_BGERRMSG);
          continue;
        } else {
          /* Set up arg vector for call */

          for (i = 0; i < ntok; i++) {
            args[i] = & tokbuf[tok[i]];
          }

          /* Call builtin shell function */

          if (( * cmdtab[j].cfunc)(ntok, args) ==
              SHELL_EXIT) {
            break;
          }
        }
        continue;
      }

      /* Open files and redirect I/O if specified */
      //kprintf("\n INNAME: %s", inname);
      //kprintf("\n OUTNAME: %s", outname);
      if (inname != NULL) {
        stdinput = open(RFILESYS, inname, "ro");
        if (stdinput == SYSERR) {
          fprintf(dev, SHELL_INERRMSG, inname);
          continue;
        }
      }
      if (outname != NULL) {
        stdoutput = open(RFILESYS, outname, "w");
        if (stdoutput == SYSERR) {
          fprintf(dev, SHELL_OUTERRMSG, outname);
          continue;
        } else {
          control(stdoutput, F_CTL_TRUNC, 0, 0);
        }
      }

      /* Spawn child thread for non-built-in commands */
      //kprintf("cmd name: %s , j: %d args: %d", cmdtab[j].cname, j, ntok);
      
      child = create(cmdtab[j].cfunc,
          SHELL_CMDSTK, SHELL_CMDPRIO,
          cmdtab[j].cname, 2, ntok, & tmparg);

      /* If creation or argument copy fails, report error */
//kprintf("addargs: ntok%d , tlen %d", ntok, tlen);
      if ((child == SYSERR) ||
          (addargs(child, ntok, tok, tlen, tokbuf, & tmparg) ==
           SYSERR)) {
        fprintf(dev, SHELL_CREATMSG);
        continue;
      }

      /* Set stdinput and stdoutput in child to redirect I/O */

      proctab[child].prdesc[0] = stdinput;
      proctab[child].prdesc[1] = stdoutput;

      msg = recvclr();
      resume(child);
      if (!backgnd) {
        msg = receive();
        while (msg != child) {
          msg = receive();
        }
      }





    } 
    
    
    
    
    else {
      
      /* Verify remaining tokens are type "other" or "pipe" */
      i = 0;
      bool8 found = FALSE;

      while (i < ntok) {
        if(toktyp[i] == SH_TOK_LESS && i == 1){
          if(toktyp[i+1] != SH_TOK_OTHER){
            break;
          }
          if(toktyp[i] == SH_TOK_LESS && inname == NULL){
            //kprintf("BE HERE");
            //inname = & tokbuf[tok[i+1]];
            inname = & tokbuf[tok[i+1]];
            found = TRUE;
           /* ntok -= 2;
            tlen = tok[ntok];*/
          }

        }
        else if ((toktyp[i] != SH_TOK_OTHER) && (toktyp[i] != SH_TOK_PIPE)) {
          //kprintf("BREAKING");
          break;
        }
        if ((toktyp[i] == SH_TOK_PIPE) || (i == 0)) {
          if (toktyp[i - 1] == SH_TOK_PIPE) {
            break;
          }
          if (toktyp[i] == SH_TOK_PIPE) {
            i++;
            npip++;
          }
          /* Lookup Command token in the command table */
          for (j = 0; j < ncmd; j++) {
            src = cmdtab[j].cname;
            cmp = & tokbuf[tok[i]];
            diff = FALSE;
            while ( * src != NULLCH) {
              if ( * cmp != * src) {
                diff = TRUE;
                break;
              }
              src++;
              cmp++;
            }
            if (diff || ( * cmp != NULLCH)) {
              continue;
            } else {
              break;
            }
          }
          /* Handle command not found */

          if (j >= ncmd) {
            fprintf(dev, "command %s not found\n", & tokbuf[tok[i]]);
            break;
          }


          /* Check for a built-in command */

          if (cmdtab[j].cbuiltin) {
            fprintf(dev, SHELL_BPERRMSG);
            break;
          }


          /*if(tokbuf[i+1] != SH_TOK_OTHER)
            {
            fprintf(dev, SHELL_SYNERRMSG);in
            ntok == 0;
            break;
            }*/

        }

        i++;
      }
      if(found == TRUE){
        // tokbuf[tok[1]] = NULLCH;
        //tokbuf[tok[2]] = NULLCH;
         ntok -= 2;
         tlen = tok[ntok];
        /*
        int32 *temp[SHELL_MAXTOK];
        int h = 0;
        int tindex = 0;
        for(h = 0 ; h < ntok ; h++){

          if(h == 1){
            tindex += 2;
          }
          temp[h] = &tok[tindex];
          tindex ++;

        }
            ntok -= 2;
            for(h = 0; h < ntok; h++){
              tok[h] = temp [h];
            }
            tlen = tok[ntok];*/
      }
      if ((ntok == 0) || (i < ntok) || (npip > NPIPES) || (toktyp[i - 1] == SH_TOK_PIPE)) {
        //kprintf("HERE");
        fprintf(dev, SHELL_SYNERRMSG);
        continue;
      }

      stdinput = stdoutput = dev;

      /* Open files and redirect I/O if specified */
      //kprintf("\n INNAME: %s", inname);
      //kprintf("\n OUTNAME: %s", outname);
      if (inname != NULL) {
        stdinput = open(RFILESYS, inname, "ro");
        if (stdinput == SYSERR) {
          fprintf(dev, SHELL_INERRMSG, inname);
          continue;
        }
      }
      if (outname != NULL) {
        stdoutput = open(RFILESYS, outname, "w");
        if (stdoutput == SYSERR) {
          fprintf(dev, SHELL_OUTERRMSG, outname);
          continue;
        } else {
          control(stdoutput, F_CTL_TRUNC, 0, 0);
        }
      }
      /* Open pipe devices */

      for (i = 0; i < npip; i++) {
        pipIDs[i] = open(PIPE, "", "");
        proctab[currpid].prdesc[proctab[currpid].pprdesc] = -1;
      }

      /* Create child processes to execute the commands */

      i = 0;
      p = 0;

      while (i < ntok) {

        if (toktyp[i] == SH_TOK_PIPE) {
          i++;
        }

        /* Lookup Command token in the command table */

        for (j = 0; j < ncmd; j++) {
          src = cmdtab[j].cname;
          cmp = & tokbuf[tok[i]];
          diff = FALSE;
          while ( * src != NULLCH) {
            if ( * cmp != * src) {
              diff = TRUE;
              break;
            }
            src++;
            cmp++;
          }
          if (diff || ( * cmp != NULLCH)) {
            continue;
          } else {
            break;
          }
        }

        z = i;
        if(toktyp[i+1] == SH_TOK_LESS){
          //kprintf("HERE toktyp 1 : %d  toktyp 2 : %d  toktyp 3 : %d   toktyp 4 : %d", toktyp[z] , toktyp[i] , toktyp[i+1], toktyp[i+2]);
          //inname = & tokbuf[tok[i+2]];
         //i+= 2;
          //z=i;
        }
        
        if(toktyp[i] == SH_TOK_LESS)
        {
          //kprintf(" SH_TOK_LESS");
        }
        while ((i < ntok) && (toktyp[i] != SH_TOK_PIPE)) {
          i++;
        }
        i--;

        /* Spawn child thread for non-built-in commands */
        //kprintf("cmd name: %s , j: %d args: %d", cmdtab[j].cname,j, (i - z + 1));
       child = create(cmdtab[j].cfunc,
            SHELL_CMDSTK, SHELL_CMDPRIO,
            cmdtab[j].cname, 2, (i-z + 1), &tmparg);
            /*int d = 0;
            for( d = 0 ; d < teln; d++){
              //kprintf("tok[%d] :%d",d,tok[d]);
            }*/

        /* If creation or argument copy fails, report error */
        //addargs(child, ntok, tok, tlen, tokbuf, & tmparg)
        //kprintf( "TRUE ntok : %d tlen: %d", ntok,tlen);
        //kprintf( "FALSE ntok : %d tlen: %d", (i - z + 1),tok[i + 1] - tok[z]);
        if (i == (ntok - 1) && p != 0 ) {
          //kprintf("\n ONE \n");
          retval = addargs(child, (i - z + 1), tok + z, tlen - tok[z], tokbuf + tok[z], &tmparg);
        } else if (p == 0 && found == TRUE){
          //kprintf("\n TWO \n");
          child = create(cmdtab[j].cfunc,
            SHELL_CMDSTK, SHELL_CMDPRIO,
            cmdtab[j].cname, 2, 1, &tmparg);
            //kprintf( "TRUE ntok : %d tlen: %d", ntok,tlen);
            int before = i;
            i=z;
        //kprintf( "FALSE ntok : %d tlen: %d", (i - z + 1),tok[i + 1] - tok[z]);
         retval = addargs(child, 2, tok + z, tok[i+1] - tok[z], tokbuf + tok[z] , &tmparg);
          i = before;
        }
        else {
          //kprintf("\n THREE \n");
          retval = addargs(child, (i - z + 1), tok + z, tok[i + 1] - tok[z], tokbuf + tok[z], &tmparg);
        }

        if ((child == SYSERR) ||
            (retval == SYSERR)) {
          fprintf(dev, SHELL_CREATMSG);
          break;
        }

        /* Set stdinput and stdoutput in child to redirect I/O */
        //proctab[child].prdesc[0] = stdinput;
        // proctab[child].prdesc[1] = stdoutput;
        //kprintf("HERE");
        if (z == 0) {
          	//kprintf("\n Process stdin: %d", p);
          proctab[child].prdesc[0] = stdinput;
        } else {
            //kprintf("\n change in: %d", p);
          proctab[child].prdesc[0] = pipIDs[p - 1];
        }

        if (i == (ntok - 1)) {
          	//kprintf("\n Process stdout: %d", p);
          proctab[child].prdesc[1] = stdoutput;
        } else {
             //kprintf("\n change out: %d", p);
          proctab[child].prdesc[1] = pipIDs[p];
        }
        

        resume(child);
        //kprintf("\nProcess: %d", p);
        i++;
        p++;

      }
      //kprintf("STUCK1");
      msg = recvclr();
       //kprintf("STUCK2");
      if (!backgnd) {
          //kprintf("STUCK3");
        msg = receive();
          //kprintf("STUCK4");
        while (msg != child) {
          msg = receive();
          //kprintf("STUCK5");
        }
      }
      // //kprintf("END OF ELSE");
    }
    // //kprintf("END OF ELSE");
  }

  /* Terminate the shell process by returning from the top level */

  fprintf(dev, SHELL_EXITMSG);
  return OK;
}
