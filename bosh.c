/* 

   bosh.c : BOSC shell 

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.h"
#include "print.h"
#include "pipe.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100

/* --- use the /proc filesystem to obtain the hostname --- */
char *gethostname(char *hostname)
{
  FILE *fp;
  fp = fopen("/proc/sys/kernel/hostname", "r");
  if(fgets(hostname, HOSTNAMEMAX, fp)==NULL) {
    hostname = NULL;
  } else {
    hostname[strlen(hostname)-1] = '\0';
  }
  fclose(fp);
  return hostname;
}

/* --- execute a shell command --- */
int executeshellcmd (Shellcmd *shellcmd)
{
  pid_t pid = fork();
  char * filenameIn = shellcmd->rd_stdin;
  char * filenameOut = shellcmd->rd_stdout;
  int isBackground = shellcmd->background;
  Cmd * cmd = shellcmd->the_cmds;
  if(pid == 0) { // child process
    if(isBackground) {
      int fid = open("/dev/null", O_RDWR);
      dup2(fid, 1);
    } else if(filenameOut) {
      int fid = open(filenameOut, O_RDWR|O_CREAT, 0666);
      dup2(fid, 1);
    }

    pipecmd(cmd,filenameIn);

  } else { // parent process
    if(!isBackground) {
      int returnStatus;
      waitpid(pid, &returnStatus, 0);
    }
  }
  return 0;
}

/* --- execute program --- */
int executeProgram(char **argv) {
  execvp(argv[0], argv);
  return 0;
}

/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* initialize the shell */
  char *cmdline;
  char hostname[HOSTNAMEMAX];
  int terminate = 0;
  Shellcmd shellcmd;
  
  if (gethostname(hostname)) {

    /* parse commands until exit or ctrl-c */
    while (!terminate) {
      printf("%s", hostname);
      if (cmdline = readline(":# ")) {
	if(*cmdline) {
	  add_history(cmdline);
	  if (parsecommand(cmdline, &shellcmd)) {
      const char *firstCmd = (&shellcmd)->the_cmds->cmd[0];
      if (strcmp(firstCmd, "exit") == 0) {
        return 0;
      }
	    terminate = executeshellcmd(&shellcmd);
	  }
	}
	free(cmdline);
      } else terminate = 1;
    }
    printf("Exiting bosh.\n");
  }    
    
  return EXIT_SUCCESS;
}

