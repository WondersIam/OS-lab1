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
  execute_command(cmd_list);
}
static void execute_command(Command *cmd_list)
{
    if((cmd_list->rstdout)!=NULL)//output redirection
    {
      file_name = cmd_list->rstdout;
       redirection = 1;
      execute_pgm(cmd_list->pgm);
      print_cmd(cmd_list);
    }
    else if((cmd_list->rstdin)!=NULL)//input redirection
    {
       redirection = -1;
      execute_pgm(cmd_list->pgm);
      print_cmd(cmd_list);
    }
    else//no redirection
    {
       redirection = 0;//no redirection
      execute_pgm(cmd_list->pgm);
      print_cmd(cmd_list);
    }
   
}//static int redirection is respectively 1,-1,0 output_redirection,input_redirection,no redirection
static void execute_pgm(Pgm *p)
{
    if(p==NULL){return;}//end of the pointer structure
  else
  {
    char **pl = p->pgmlist; 
    execute_pgm(p->next);
    while(*pl)
    {
      if(strcmp(*pl,"ls")==0)
      {
        cmd_ls();
        *pl++;
      }
      else if(strcmp(*pl,"date")==0)//simplily date command
      {
          cmd_date();
          *pl++;
      }
      else if(strcmp(*pl,"who")==0)
      {
          cmd_who();
          *pl++;
      }
      else
      {
        printf("Now none is executed\n");
        *pl++;
      }
    }
  }
}
/*
 * Print a Command structure as returned by parse on stdout.
 *
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
static void cmd_ls()
{
  DIR *pdir = opendir(".");
  struct dirent *pdirent=NULL;
  if(pdir==NULL)
  {
    printf("failed\n");
  }
  while((pdirent=readdir(pdir))!=NULL)
  {
    if(pdirent->d_name[0]=='.')
    {
      continue;//omted files hided
    }
    //out redirection
    if(redirection == 0)
    {
      printf("%s   ",pdirent->d_name);
    }
    else if(redirection ==1)
    {
        FILE* pf = fopen(file_name,"a");
        //printf("%s   ",pdirent->d_name);
        fprintf(pf,"%s   ",pdirent->d_name);
        fclose(pf);
    }
  }
  closedir(pdir);
  printf("\n");
}
static void cmd_date()
{
          struct tm *ptr;//tm is structure including all arguments about time showing
          time_t it;    //long int
          char str[80]; //where temporary results stored
          it = time(NULL); //return a long int,which is how many seconds from Jan.1st 1970
          ptr=localtime(&it); // transfer time to local time
          strftime(str,sizeof(str),"%a %e %b %X %Y %Z",ptr);//strftime is format function for printf
          if(redirection ==0)
          {
            printf("%s\n",str);
          }
          else if(redirection == 1)
          {
          FILE* pf = fopen(file_name,"a");
          fprintf(pf,"%s   ",str);
          fclose(pf);
          }
          //printf("in execute_command: %s\n",*pl);
}
static void cmd_who()
{
  int ok;
  ok = mywho();
}
static int mywho()
{
  struct utmp *um; //utmp include all arguments about messages of users
  char timebuf[24];
  int users= 0;//innitial users number 0
  while(um=getutent())
  {
    if(um->ut_type != USER_PROCESS)
    {
      continue;
    }
    time_t tm;
    tm = (time_t)(um->ut_tv.tv_sec);
    strftime(timebuf,24,"%F %R",localtime(&tm));
    if(redirection==0)
    {printf("%-12s%-12s%-20.20s (%s)\n",um->ut_user,um->ut_line,timebuf,um->ut_host);}
    else if(redirection == 1)
    {
          FILE* pf = fopen(file_name,"a");
          /*char *s1[100];
          char *s2[100];
          char *s3[100];
          char *s4[100];
          strcpy(s1,um->ut_user);
          strcpy(s1,um->ut_line);
          strcpy(s1,timebuf);
          strcpy(s1,um->ut_host);
          printf(pf,"%-12s %-12s%-20.20s (%s)\n",s1,s2,s3,s4);
          */
          fprintf(pf,"%-12s%-12s%-20.20s (%s)\n",um->ut_user,um->ut_line,timebuf,um->ut_host);
          fclose(pf);
    }

  }
  endutent();
  return 0;


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
