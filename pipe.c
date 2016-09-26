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

int pipecmd(Cmd * cmd, char * inFilename)
{
    char ** args = cmd->cmd;

    if(cmd->next != NULL) {
      int fid_pipe[2];
      pipe(fid_pipe);
      pid_t pid = fork();

      if(pid == 0) { // child process
        close(fid_pipe[0]);
        close(1);
        dup(fid_pipe[1]);
        pipecmd(cmd->next, NULL);
      } else { // parent process
        int returnStatus;
        close(fid_pipe[1]);
        dup(fid_pipe[0]);
        close(0);
        char * argv[] = { args[0], inFilename, NULL };
        if(inFilename) {
          if(execvp(args[0], argv) < 0) {
            printf("Command '%s' not found.\n", args[0]);
          }
        } else {
          if(execvp(args[0], args) < 0) {
            printf("Command '%s' not found.\n", args[0]);
          }
        }
        waitpid(pid, &returnStatus, 0);
      }

    } else {
      char * argv[] = { args[0], inFilename, NULL };
      if(inFilename) {
        if(execvp(args[0], argv)) {
          printf("Command '%s' not found.\n", args[0]);
        }
      } else {
        if(execvp(args[0], args) < 0) {
          printf("Command not found.\n");
        }
      }
    }
    return 0;
}