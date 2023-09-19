/*
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file(s)
 * you will need to modify the CMakeLists.txt to compile
 * your additional file(s).
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Using assert statements in your code is a great way to catch errors early and make debugging easier.
 * Think of them as mini self-checks that ensure your program behaves as expected.
 * By setting up these guardrails, you're creating a more robust and maintainable solution.
 * So go ahead, sprinkle some asserts in your code; they're your friends in disguise!
 *
 * All the best!
 */
#include <assert.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// The <unistd.h> header is your gateway to the OS's process management facilities.
#include <unistd.h>

#include "parse.h"
//head file I added
#include<getopt.h>
#include<stdbool.h>
#include<utmp.h>
#include<dirent.h>
#include<signal.h>

static void run_cmds(Command *);
static void print_cmd(Command *cmd);
static void print_pgm(Pgm *p);
void stripwhite(char *);

//below is fuction I added
static void execute_command(Command *cmd_list);
static void execute_pgm(Pgm *p);
static void cmd_date();
static void cmd_who();
static int mywho();
static void cmd_ls();
static int redirection = 0;
char *file_name="n";
int main(void)
{
  for (;;)
  {
    char *line;
    line = readline("> ");

    // If EOF encountered, exit shell
    if (!line)
    {
      break;
    }

    // Remove leading and trailing whitespace from the line
    stripwhite(line);

    // If stripped line not blank
    if (*line)
    {
      add_history(line);

      Command cmd;
      if (parse(line, &cmd) == 1)
      {
        run_cmds(&cmd);
      }
      else
      {
        signal(SIGINT,SIG_DFL);
        printf("Parse ERROR\n");
      }
    }

    // Clear memory
    free(line);
  }

  return 0;
}

/* Execute the given command(s).

 * Note: The function currently only prints the command(s).
 *
 * TODO:
 * 1. Implement this function so that it executes the given command(s).
 * 2. Remove the debug printing before the final submission.
 */
static void run_cmds(Command *cmd_list)
{
  //print_cmd(cmd_list);
  int runpipe[2];//used for pipe both(used for firrst parent and child process)
  int runpipe2[2];//ueed for the second pipe
  int newfd;
  int pid;
  int pid2;
if(cmd_list->pgm->next==NULL)//End command
{
  if((pid=fork())==-1)
  {
    printf("Fork() failed.");
  }
  if(pid==0)
  {
    execvp(cmd_list->pgm->pgmlist[0],&cmd_list->pgm->pgmlist[0]);
    //exit(0);
  }
  else
  {
    wait(NULL);
  }
}
else if(cmd_list->pgm->next->next ==NULL)//one pipe
{
  if(pipe(runpipe)==0)
  {
    printf("Pipe fialed");
  }
  if((pid=fork())==-1)
  {
    printf("Fork() failed.");
  }
  if(pid==0)
  {
    close(runpipe[0]);
    if(dup2(runpipe[1],STDOUT_FILENO)==-1)//last process output is assigned to the write of pipe
    {
      printf("cannot redirect");
    }
    close(runpipe[1]);
    execvp(cmd_list->pgm->next->pgmlist[0],&cmd_list->pgm->next->pgmlist[0]);
    
  }
  else
  {
    wait(NULL);
    close(runpipe[1]);
        if(dup2(runpipe[0],STDIN_FILENO)==-1)//patent process input assigned to read of the pipe
    {
      printf("cannot redirect");
    }
      close(runpipe[0]);
      execvp(cmd_list->pgm->pgmlist[0],&cmd_list->pgm->pgmlist[0]);
      
  }
}
//two pipes
else if(cmd_list->pgm->next->next==NULL)
{
  if(pipe(runpipe)==0)
  {
    printf("pipe failed");
  }
  if((pid=fork())==-1)
  {
    printf("Fork() failed.");
  }

  if(pid==0)//the last child
  {
    if(pipe(runpipe2)==0)
    {
      printf("pipe failed");
    }
    if((pid=fork())==-1)
    {
      printf("Fork() failed.");
    }
    
    if(pid==0)//the last child
    {
      close(runpipe2[0]);
      if(dup2(runpipe2[1],STDOUT_FILENO)==-1)
      {
        printf("cannot redirect");
      }
      close(runpipe2[1]);
      execvp(cmd_list->pgm->next->next->pgmlist[0],&cmd_list->pgm->next->next->pgmlist[0]);
      printf("last here");
    }
    else//the last child parent and the first child
    {
      wait(NULL);
      close(runpipe2[1]);
      if(dup2(runpipe2[0],STDIN_FILENO)==-1)
      {
        printf("cannot redirect");
      }
      close(runpipe2[0]);
      close(runpipe[0]);
      if(dup2(runpipe[1],STDOUT_FILENO)==-1)
      {
        printf("cannot redirect");
      }
      close(runpipe[1]);
      execvp(cmd_list->pgm->next->pgmlist[0],&cmd_list->pgm->next->pgmlist[0]);
    }
    

  }
  else//first parent process
  {
    wait(NULL);
    wait(NULL);
    close(runpipe[1]);
        if(dup2(runpipe[0],STDIN_FILENO)==-1)
    {
      printf("cannot redirect");
    }
      close(runpipe[0]);
      execvp(cmd_list->pgm->pgmlist[0],&cmd_list->pgm->pgmlist[0]);
}

}
}



static void print_cmd(Command *cmd_list)
{
  printf("------------------------------\n");
  printf("Parse OK\n");
  printf("stdin:      %s\n", cmd_list->rstdin ? cmd_list->rstdin : "<none>");
  printf("stdout:     %s\n", cmd_list->rstdout ? cmd_list->rstdout : "<none>");
  printf("background: %s\n", cmd_list->background ? "true" : "false");
  printf("Pgms:\n");
  print_pgm(cmd_list->pgm);
  printf("------------------------------\n");
}

/* Print a (linked) list of Pgm:s.
 *
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
static void print_pgm(Pgm *p)
{
  if (p == NULL)
  {
    return;
  }
  else
  {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    print_pgm(p->next);
    printf("            * [ ");
    while (*pl)
    {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}


/* Strip whitespace from the start and end of a string.
 *
 * Helper function, no need to change.
 */
void stripwhite(char *string)
{
  size_t i = 0;

  while (isspace(string[i]))
  {
    i++;
  }

  if (i)
  {
    memmove(string, string + i, strlen(string + i) + 1);
  }

  i = strlen(string) - 1;
  while (i > 0 && isspace(string[i]))
  {
    i--;
  }

  string[++i] = '\0';
}
