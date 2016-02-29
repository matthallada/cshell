#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
void parseArgs(char* command, char** args);
int runCommand(char** args);

//present prompt for commands
int main()
{
  unsigned int len_max = 9;
  unsigned int current_size = 0;

  char *command = malloc(len_max);
  current_size = len_max;
  char** args = malloc(len_max);
  //accept command input of arbitrary length until q is input
  printf ("Enter your command: q to quit");
  while(1){
    // Read command from standard input
    unsigned int i = 0;
    int c = 0;
        //accept user input until hit enter or end of file
    while (( c = getchar() ) != '\n')
    {
        command[i++]=(char)c;

        //if i reached maximize size then realloc size
        if(i == current_size)
        {
	  current_size = i+len_max;
	  command = realloc(command, current_size);
        }
    }
    command[i] = '\0';

    // split command into array of parameters
    parseArgs(command, args);
    
    // determine if to exit
    if(strcmp(args[0], "q") == 0) break;

    // run the command
    if(runCommand(args) == 0) break;
    
  }
  return 0;
}

// splits the given command into an array of arguments
void parseArgs(char* command, char** args)
{
  int current_size = 9;
  int x = 0;
  while (1){
    if (x == current_size){
      current_size += 9;
      args = realloc(args, current_size);
    }
    args[x] = strsep(&command, " ");
    if(args[x] == NULL) break;
    x += 1;
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
    execvp(args[0], args);

    //check for error
    char* error = strerror(errno);
    printf("shell: %s: %s\n", args[0], error);
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

