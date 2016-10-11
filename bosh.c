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
#include <signal.h>
#include <unistd.h>

/* --- symbolic constants --- */
#define HOSTNAMEMAX 100
pid_t pid;

/* --- use the /proc filesystem to obtain the hostname --- */
char *getboshhostname(char *hostname)
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
  pid = fork();
  char * filenameOut = shellcmd->rd_stdout;
  int isBackground = shellcmd->background;
  if(pid == 0) { // child process
    if(isBackground) {
      int fid = open("/dev/null", O_RDWR);
      dup2(fid, 1);
    } else if(filenameOut) {
      int fid = open(filenameOut, O_RDWR|O_CREAT, 0666);
      dup2(fid, 1);
    }

    pipecmd(shellcmd);

  } else { // parent process
    if(!isBackground) {
      waitpid(pid, 0, 0);
    }
  }
  return 0;
}

void ctrlCHandler(int dummy) {
  if(dummy == SIGINT) {
    if(pid) {
      kill(pid, SIGTERM);
    }
  }
}

/* --- main loop of the simple shell --- */
int main(int argc, char* argv[]) {

  /* initialize the shell */
  char *cmdline;
  char hostname[HOSTNAMEMAX];
  int terminate = 0;
  Shellcmd shellcmd;

  signal(SIGINT, ctrlCHandler);

  if (getboshhostname(hostname)) {

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

