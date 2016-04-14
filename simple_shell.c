#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "utils.h"
#include <sys/wait.h>
char ** parseArgs(char* command);
int runCommand(char** args);
int childCommands(char** args);
void exitFunc(char** args);
void chdirFunc(char** args);
void getenvFunc(char** args);
void setenvFunc(char** args);
void echoFunc(char** args);
void writeHistory(char** args);
void getPath(char **args);
int parseScript(char *script);

//present prompt for commands
int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    parseScript(argv[1]);
    return 1;
  }
  FILE *sh_profile = fopen(".421sh_profile", "r");
  if (sh_profile != NULL)
  {
    fclose(sh_profile);
    parseScript(".421sh_profile");
  }
  unsigned int len_max = 9;

  //char *command = malloc(len_max);
  //unsigned int command_size = len_max;
  //char** args = malloc(len_max);
  //accept command input of arbitrary length until q is input

  while(1){
    // Read command from standard input
    printf ("Enter your command:");
    char *command = malloc(len_max);
    unsigned int command_size = len_max;
    unsigned int comi = 0;
    int c = 0;
    int search = 0;
        //accept user input until hit enter or end of file or a comment
    while (( c = getchar() ))
    {
      if (c == '\n')
      {
        command[comi++] = '\0';
        break;

      }
      else if (c == '#')
      {
        //do nothing
        search = 1;
      }
      else if (search == 0)
      {
        command[comi++]=(char)c;
        //if i reached maximize size then realloc size
        if(comi == (command_size-1))
        {
	        command_size = comi+len_max;
	        char *comptr = realloc(command, command_size);
          if (comptr == NULL)
          {
            printf("Realloc failed!!\n");
            free(command);
            exit(0);
          }
          else
          {
            command = comptr;
          }
        }
      }
    }
    // split command into array of parameters
    char ** args = parseArgs(command);
    //printf("%s", args[3]);
    // writeHistory(args);
    // run the command breaks the while for the child
    if(runCommand(args) == 0) 
    {
      break;
    }

    free(command);
    free(args);
  }
  return 0;
}

// splits the given command into an array of arguments
// command name, and rest of input
char **parseArgs(char* command)
{
  char **args = malloc(20*sizeof(char));
  int current_size = 0;
  int actual_size = 0;
  int x = 0;
  char * tmpcom = strtok(command, " ");
  while (1){
    if (tmpcom == NULL) 
    {
      args[x++] = NULL;
      break;
    }

    actual_size += strlen(tmpcom);

        // increase the size of args
    while (actual_size >= (current_size-2))
    {
      current_size += 20;
    }
    char **args2 = realloc(args, current_size);
    if (args2 == NULL)
    {
      printf("Realloc failed!!\n");
      free(args);
      exit(0);
    }
    else
    {
      args = args2;
    }

    args[x++] = tmpcom;
    tmpcom = strtok(NULL, "\0");
  }
  return args;
}

int runCommand(char** args)
{
  //run parent commands
  if (args[0] == NULL || args[0] == '\0')
  {
    return 1;
  }
  else if (strcmp(args[0], "exit") == 0) 
  {
    exitFunc(args);
    return 1;
  }
  else if (strcmp(args[0], "chdir") == 0 || strcmp(args[0], "cd") == 0)
  {
    chdirFunc(args);
    return 1;
  }
  else if (strcmp(args[0], "setenv") == 0) 
  {
    setenvFunc(args);
    return 1;
  }


  //fork the shell
  pid_t pid = fork();


  // Error
  if (pid == -1) {
    char* error = strerror(errno);
    printf("fork: %s\n", error);
    return 1;
  }

  // Child process
  else if (pid == 0){
    if (childCommands(args) == 0);
    else if (args[0][0] == '/') 
    {
      getPath(args);
      execv(args[0], args);
      //check for error
      char* error = strerror(errno);
      printf("shell: %s: %s\n", args[0], error);
    }
    else 
    {
      getPath(args);
      execvp(args[0], args);
      //check for error
      char* error = strerror(errno);
      printf("shell: %s: %s\n", args[0], error);
    }
    return 0;
  }

  // Parent process
  else {
    // Wait for child to finish
    int childStatus;
    waitpid(pid, &childStatus, 0);
    return 1;
  }
}

// gets the program path for each program
void getPath(char **args)
{
  printf("this is \n");

  int index = 1;
  if (args[index] != NULL)
  {
    int space = first_unquoted_space(args[index]);
    FILE *enverr;
    char *tmpstr;
    enverr = fopen("enverr.txt", "a+");
    while (space != -1)
    {
      tmpstr = &args[index][space+1];
      args[index][space] = '\0';
      args[index] = unescape(args[index], enverr);
      index++;
      args[index] = tmpstr;
      printf("this is %s\n", args[index]);
      space = first_unquoted_space(args[index]);
    }
    args[index] = unescape(args[index], enverr);
    printf("this is %s\n", args[index]);

    fclose(enverr);
  }
}

// runs my commands for child works
int childCommands(char** args)
{
  if (strcmp(args[0], "getenv") == 0) 
  {
    getenvFunc(args);
  }
  else if (strcmp(args[0], "echo") == 0) 
  {
    echoFunc(args);
  }
  else 
  {
    return 1;
  }
  return 0;
}

// exit function
void exitFunc(char** args)
{
  getPath(args);
  if (args[1] == NULL) 
  {
    exit(0);
  }
  int exitcode = atoi(args[1]);
  if ((exitcode == 0 && args[1][0] != 0) || args[2] != NULL) 
  {
    printf("error: incorrect exit arguments\n");
  }
  else 
  {
    exit(exitcode);
  }
}

// change the dir, gets passed an array of 
void chdirFunc(char** args)
{
  getPath(args);
  if (args[1] == NULL)
  {
    if (getenv("HOME") == NULL) printf("error: Home environment variable is not set\n");
    // go to home directory
    else
    {
      chdir(getenv("HOME"));
      // update the PWD
      setenv("PWD", getenv("HOME"), 1);
    }
  }
  else
  {
    int index = 1;
    while (args[index] != NULL)
    {
      if (chdir(args[index++]) == -1)
      { 
        printf("error: %s\n", strerror(errno));
      }
      // update the PWD
      setenv("PWD", getenv("HOME"), 1);
    }
  }
}

// look up the specified environment variable
void getenvFunc(char** args)
{
  if (args[1] == NULL) printf("error: no argument given to setenv\n");
  else if (args[2] != NULL) printf("error: more than one argument given to setenv\n");
  else
  {
    if (getenv(args[1]) == NULL) printf("\n");
    else printf("%s\n", getenv(args[1]));
  }
}

// look up the specified environment variable
void setenvFunc(char** args)
{
  // printf("error: unquoted space found\n");

  char *var = strtok(args[1], "=");
  char *message = strtok(NULL, "\0");
  args[1] = var;
  args[2] = message;
  if (first_unquoted_space(args[2]) != -1)
  {
    printf("error: unquoted space found\n");
    exit(0);
  }

  if (args[1] == NULL || args[2] == NULL) printf("error: no arguments given to setenv\n");
  FILE *enverr;
  enverr = fopen("enverr.txt", "a+");
  char *unenv = unescape(args[2], enverr);
  if (unenv != NULL)
  {
    setenv(args[1], unenv, 1);
  }
  fclose(enverr);
}

// unescape a string and print it out with a new line
void echoFunc(char** args)
{

  if (args[1] == NULL) printf("error: no argument given to echo\n");
  else if (first_unquoted_space(args[1]) != -1)
  {
    printf("error: unquoted space found\n");
  }
  else if (args[2] != NULL) printf("error: more than one argument given to echo\n");
  else
  {
    // open a file for echo errors
    FILE *echoerr;
    echoerr = fopen("echoerr.txt", "a+");
    char *unecho = unescape(args[1], echoerr);
    if (unecho != NULL) 
    { 
      printf("%s\n", unecho);
    }
    fclose(echoerr);
  }
}

// write to the 421sh_history file
void writeHistory(char** args)
{
  char **temp_args = args;
  FILE *history = fopen("./.421sh_history", "a+");
  fprintf(history, "test\n");
  int index = 0;
  while (temp_args[index] != NULL)
  {
    char *writeString = temp_args[index++];
    fprintf(history, "%s ", writeString);
  }
  fprintf(history, "\n");
  fclose(history);
}

// takes a script, executes it, returns 0 on success -1 on failure
int parseScript(char *script)
{
  //try and open script, exit if can't
  FILE *scriptF = fopen(script, "r");
  if (scriptF == NULL)
  {
    printf("error opening file: %s, %s\n", script, strerror(errno));
    return -1;
  }
  unsigned int len_max = 9;
  char *command = malloc(len_max);
  unsigned int command_size = len_max;
  unsigned int comi = 0;
  int c = 0;
  int search = 0;
  while (1)
  {
    c = fgetc(scriptF);
    // determine if end of file
    if (feof(scriptF))
    {
      break;
    }
    // run the command
    else if (c == '\n')
    {
      command[comi++] = '\0';
      // split command into array of parameters
      char ** args = parseArgs(command);
      //printf("%s", args[3]);
      // writeHistory(args);
      // run the command breaks the while for the child
      if(runCommand(args) == 0) 
      {
        break;
      }
      free(args);
      free(command);
      command = malloc(len_max);
      command_size = len_max;
      comi = 0;
      c = 0;
      search = 0;
    }
    else if (c == '#')
    {
      //do nothing
      search = 1;
    }
    else if (search == 0)
    {
      command[comi++]=(char)c;
      //if i reached maximize size then realloc size
      if(comi == (command_size-1))
      {
        command_size = comi+len_max;
        char *comptr = realloc(command, command_size);
        if (comptr == NULL)
        {
          printf("Realloc failed!!\n");
          free(command);
          exit(0);
        }
        else
        {
          command = comptr;
        }
      }
    }
  }
  fclose(scriptF);
  return 0;
  //read line by line, parse args and run commands

}