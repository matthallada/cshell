#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "utils.h"
#include <sys/wait.h>
void parseArgs(char* command, char** args);
int runCommand(char** args);
int childCommands(char** args);
void exitFunc(char** args);
void chdirFunc(char** args);
void getenvFunc(char** args);
void setenvFunc(char** args);
void echoFunc(char** args);
void parentCommands(char** args);

void freeArgs(char **args)
{
  int index = 0;
  while (args[index] != NULL)
  {
    free(args[index]);
  }
}

char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

//present prompt for commands
int main()
{
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
    char** args = malloc(len_max);
    unsigned int comi = 0;
    int c = 0;
        //accept user input until hit enter or end of file
    while (( c = getchar() ) != '\n' || EOF)
    {
      if (c == '\n')
      {
        command[comi++] = '\0';
        break;

      }
      else
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
            free(command);
            command = comptr;
          }
        }
      }
    }

    // split command into array of parameters
    parseArgs(command, args);
    //printf("%s", args[3]);

    // run the command breaks the while for the child
    if(runCommand(args) == 0) 
    {
      break;
    }


    free(args);
  }
  return 0;
}

// splits the given command into an array of arguments
void parseArgs(char* command, char** args)
{
  // int current_size = 9;
  int x = 0;
  char * tmpcom = strtok(command, " ");
  while (1){
    if (tmpcom == NULL) 
    {
        args[x++] = NULL;
        break;
    }

    // increase the size of args
    // current_size += strlen(tmpcom);
    // char **args2 = realloc(args, current_size);
    // if (args2 == NULL)
    // {
    //   printf("Realloc failed!!\n");
    //   freeArgs(args);
    //   exit(0);
    // }
    // else
    // {
    //   freeArgs(args);
    //   args = args2;
    // }

    //increase the size of args
    // char *args3 = realloc(args[x], strlen(tmpcom));
    // if (args3 == NULL)
    // {
    //   printf("Realloc failed!!\n");
    //   free(args[x]);
    //   exit(0);
    // }
    // else
    // {
    //   free(args[x]);
    //   args[x] = args3;
    // }

    args[x] = tmpcom;
    x++;
    tmpcom = strtok(NULL, " ");
  }
}

int runCommand(char** args)
{
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
      execv(args[0], args);
      //check for error
      char* error = strerror(errno);
      printf("shell: %s: %s\n", args[0], error);
    }
    else 
    {
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
    parentCommands(args);
    return 1;
  }
}

// runs my commands for child
int childCommands(char** args)
{
  if (strcmp(args[0], "exit") == 0) return 0;
  else if (strcmp(args[0], "chdir") == 0 || strcmp(args[0], "cd") == 0) return 0;
  else if (strcmp(args[0], "getenv") == 0) getenvFunc(args);
  else if (strcmp(args[0], "setenv") == 0) return 0;  
  else if (strcmp(args[0], "echo") == 0) echoFunc(args);
  else return 1;
  return 0;
}

// runs my commands for child
void parentCommands(char** args)
{
  if (strcmp(args[0], "exit") == 0) exitFunc(args);
  else if (strcmp(args[0], "chdir") == 0 || strcmp(args[0], "cd") == 0) chdirFunc(args);
  else if (strcmp(args[0], "setenv") == 0) setenvFunc(args);  
}

// exit function
void exitFunc(char** args)
{
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

// change the dir
void chdirFunc(char** args)
{
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
    int parse = 1;
    while (parse == 1)
    {
      if (args[index] == NULL) break;
      // determine if the string is unescaped
      if (args[index][strlen(args[index]) - 1] != '/')
      {
        chdir(args[index]);
        break; // breaks when it finds an unescaped string
      }
      // continue to loop
      chdir(args[index]);
      index++;
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
  if (args[1] == NULL) printf("error: no argument given to setenv\n");
  int comSize = 0;
  char *envchar = "";
  int index = 1;
  char *envname;
  while (args[index] != NULL)
  {
    comSize += strlen(args[index]);
    char *envcharptr = realloc(envchar, comSize);
    if (envcharptr == NULL)
    {
      printf("Realloc failed!!\n");
      free(envchar);
      exit(0);
    }
    else
    {
      free(envchar);
      envchar = envcharptr;
    }
    if (index == 1)
    {
      char * tmpcom = strtok(args[index], "=");
      int str = 1;
      while (1)
      {
        if (tmpcom == NULL) 
        {
          break;
        }
        // set the env name
        if (str == 1)
        {
          envname = tmpcom;
          str = 0;
        }
        else
        {
          envchar = tmpcom;
          break;
        }
      }
    }
    else
    {
      envchar = concat(envchar, args[index]);
    }
    index++;
  }
  FILE *enverr;
  enverr = fopen("enverr.txt", "a+");
  char *unenv = unescape(envchar, enverr);
  if (unenv != NULL)
  {
    setenv(envname, envchar, 1);
  }
  fclose(enverr);
}

// unescape a string and print it out with a new line
void echoFunc(char** args)
{
  if (args[1] == NULL) printf("error: no argument given to echo\n");
  else if (args[2] != NULL) printf("error: more than one argument given to echo\n");
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