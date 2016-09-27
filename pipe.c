/* 

   Opgave 3

   pipe.c

 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "parser.h"
#include "print.h"
#include "pipe.h"

int pipecmd(Shellcmd *shellcmd)
{
    Cmd *cmdList = shellcmd->the_cmds;
    char ** cmd = cmdList->cmd;
    char * filenameIn = shellcmd->rd_stdin;

    if(cmd != NULL) {
      int fid_pipe[2];

      pipe(fid_pipe);
      pid_t pid2 = fork();

      if(pid2 == 0) { // child process
        close(fid_pipe[0]);
        close(1);
        dup(fid_pipe[1]);

        Shellcmd *newshellcmd = shellcmd;
        newshellcmd->the_cmds = cmdList->next;
        newshellcmd->rd_stdin = NULL;
        pipecmd(newshellcmd);

      } else { // parent process
        int returnStatus;

        close(fid_pipe[1]);
        close(0);
        dup(fid_pipe[0]);


        char * argv[] = { *cmd, filenameIn, NULL };

        if(execvp(*cmd, filenameIn ? argv : cmd) < 0) {
          printf("Command not found.\n");
        }

        waitpid(pid2, &returnStatus, 0);
        
      }

    } else {
      char * argv[] = { *cmd, filenameIn, NULL };
      if(execvp(*cmd, filenameIn ? argv : cmd) < 0) {
        printf("Command not found.\n");
      }
    }
    return 0;
}