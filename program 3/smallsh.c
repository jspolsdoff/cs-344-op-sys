#include <sys/stat.h>  
#include <sys/types.h> 
#include <sys/wait.h>
#include <errno.h>     
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h> 
#include <fcntl.h>     
#include <limits.h>    
#include <signal.h>     
#include <unistd.h>    

#define MAX_PIDS       1000
#define MAX_LENGTH     2048
#define MAX_ARGS        512
#define DEBUG             0

typedef enum { false, true } bool;

// global variables
pid_t bgpid[MAX_PIDS];         
pid_t completed_pid[MAX_PIDS]; 
pid_t fgpid = INT_MAX; 
int completed_cur = 0;
int cur = 0;                  
int signalNum = 0;

// function prototypes
void bgHandler(int sig, siginfo_t* info, void* vp);
void sigintHandler();

int main(int argc, char** argv){
	// set up local variables
	bool isBackgroundProcess = false;
  bool repeat = true;
  char *args[MAX_ARGS + 1];
  char input[MAX_LENGTH];
  char *marker;
  pid_t cpid;
  int bgExitStatus, bgStatus;
  int exitStatus, status;
  int fd, fd2;
  int i, j;
  int numArgs;

	// set up some sigactions

	// the regular action process
	struct sigaction reg_act;
  reg_act.sa_handler = SIG_IGN;
  reg_act.sa_flags = SA_RESTART;
  sigfillset(&(reg_act.sa_mask));
  sigaction(SIGINT, &reg_act, NULL); 

	// the foreground action process
	struct sigaction foreground_act;
  foreground_act.sa_handler = sigintHandler;
  foreground_act.sa_flags = SA_RESTART;
  sigfillset(&(foreground_act.sa_mask));
  sigaction(SIGINT, &foreground_act, NULL); 

	// the background action process
	struct sigaction background_act;
  background_act.sa_sigaction = bgHandler;     
  background_act.sa_flags = SA_SIGINFO|SA_RESTART;
  sigfillset(&(background_act.sa_mask));
  sigaction(SIGCHLD, &background_act, NULL);

  // allocate mem for argument array
  for (i = 0; i <= MAX_ARGS; i++){
  	args[i] = (char *) malloc((MAX_LENGTH + 1) * sizeof(char)); 
  }  

	// background array init
	for (i = 0; i < MAX_PIDS; i++){
    completed_pid[i] = bgpid[i] = INT_MAX;
  }

  // loop for shell until user quits   
  do {
  	char **next = args;

	  // make sure to init arg array
	  for (i = 0; i <= MAX_ARGS; i++){
	    strcpy(args[i], "\n");
	  }

	  // make sure to clear out the buffer
	  strcpy(input, "\0");

	  i = 0;

	  // time to be like the walking dead and take out the zombies
	  while (i < MAX_PIDS && completed_pid[i] != INT_MAX){
	    if (DEBUG){
	      printf("Now cleaning up process %d\n", completed_pid[i]);
	    }

	    completed_pid[i] = waitpid(completed_pid[i], &bgStatus, 0);

	    // print out all ids and status
	    if (WIFEXITED(bgStatus)){
	      bgExitStatus = WEXITSTATUS(bgStatus);
	      printf("background pid %d is done: exit value %d.\n", completed_pid[i], bgExitStatus);
	    }
	    else{
	      bgExitStatus = WTERMSIG(bgStatus);
	      printf("background pid %d is done: terminated by signal %d\n", completed_pid[i], bgExitStatus);
	    }

	    j = 0;
	    while (j < MAX_PIDS && bgpid[j] != INT_MAX){ 
        if (bgpid[j] == completed_pid[i]){
          if (DEBUG){
            printf("Now removing process %d from array.\n", bgpid[j]);
          }                   

          bgpid[j] = INT_MAX;

          // fill in the pid gap
          int k = j;                       
          while (k + 1 < MAX_PIDS && bgpid[k+1] != INT_MAX){
            bgpid[k] = bgpid[k+1];
            bgpid[k+1] = INT_MAX;
            k++;
          }    
 
          cur--; 
        }
        j++;
	    }

	    completed_pid[i] = INT_MAX;

	    i++; 
  	}

    completed_cur = 0;

    // flush out buffer prompt
    fflush(stdin);
    fflush(stdout);

    // prompt and get user input
    printf(": ");
    fgets(input, MAX_LENGTH, stdin);

    // flush out prompt
    fflush(stdin);

    // conditional for null and blank line
    if (input[0] == '\n' || input[0] == '\0'){
      continue;
    }

    // parse the input
    numArgs = 0;
    marker = strtok(input, " "); 


    // loops through and process up to 512 args
    while (marker != NULL && numArgs < MAX_ARGS){

      if (strlen(marker) == 0){
        continue;
      }   

      strcpy(*next, marker);

      if (DEBUG){
        printf("args[%d] is: %s\n", numArgs, args[numArgs]); 
      }

      numArgs++;

      marker = strtok(NULL, " ");

      if (marker != NULL){
        *next++;
      } 
   	}

   	// remove newline char from last arg, if any
    marker = strtok(*next, "\n"); 
    if (marker != NULL){
      strcpy(*next, marker);
    }
  
    // check if the command is a background process
    if (strcmp(args[numArgs - 1], "&") == 0){
      // set variable appropriately for later 
      isBackgroundProcess = true;

      numArgs--; 
    }
    else{
      *next++;
    }

    if (strncmp(args[0], "#", 1) == 0)
    {
        // do nothing for comments
    }
    else if (strcmp(args[0], "exit") == 0){

      i = 0;
      while (i < MAX_PIDS && bgpid[i] != INT_MAX){
        if (DEBUG)
        {
            printf("Killing process %d\n", bgpid[i]);
        }

        kill(bgpid[i], SIGKILL);
        i++;
      }

      // let go of memory
      for (i = 0; i <= MAX_ARGS; i++){
        if (DEBUG){
          printf("Freeing memory for args[%d], which has a value of %s\n",i, args[i]);
        } 
        free(args[i]); 
      }  

      repeat = false;
    }
    else if (strcmp(args[0], "cd") == 0){ 
      // with no args change to home dir
      if (numArgs == 1){
        chdir(getenv("HOME"));
      }
      // if one arg, change to dir provided
      else{
        chdir(args[1]);
      }
    }
    else if (strcmp(args[0], "status") == 0){ 
      if (WIFEXITED(status)){
        exitStatus = WEXITSTATUS(status);
        printf("exit value %d\n", exitStatus);
      }
      else if (signalNum != 0){
        printf("terminated by signal %d\n", signalNum);
      } 
    }

    // put it through the bas interp
    else 
    {
      cpid = fork();

      if (cpid == 0){
        int inputOffset = 0;
        int outputOffset = 0;
        bool checkStatus = false; 
        bool redirectInput = false;
        bool redirectOutput = false;

        if (numArgs > 4 && strcmp(args[numArgs-4], "<") == 0){
          redirectInput = true;
          inputOffset = 3; 
        }
        else if (numArgs > 2 && strcmp(args[numArgs-2], "<") == 0){
          redirectInput = true;
          inputOffset = 1; 
        }
        
        if (numArgs > 4 && strcmp(args[numArgs-4], ">") == 0){
          redirectOutput = true;
          outputOffset = 3; 
        }
        else if (numArgs > 2 && strcmp(args[numArgs-2], ">") == 0){
          redirectOutput = true;
          outputOffset = 1; 
        }

        if (isBackgroundProcess == true && redirectInput == false){
          fd = open("/dev/null", O_RDONLY);
          checkStatus = true;      
        }
        else if (redirectInput == true){
          fd = open(args[numArgs - inputOffset], O_RDONLY);
          checkStatus = true;  
        }

        if (checkStatus == true){
          if (fd == -1){
            printf("smallsh: cannot open %s for input\n", args[numArgs - inputOffset]);
            exit(1); 
          }

          fd2 = dup2(fd, 0);
          
          if (fd2 == -1){
            printf("smallsh: cannot open %s for input\n", args[numArgs - inputOffset]);
            exit(1);
        	}   
      	}

        if (redirectOutput == true)
        {
          fd = open(args[numArgs - outputOffset], O_WRONLY|O_CREAT|O_TRUNC, 0644);

          if (fd == -1){
            printf("smallsh: cannot open %s for output\n", args[numArgs - outputOffset]);
            exit(1); 
          }

          fd2 = dup2(fd, 1);
          
          if (fd2 == -1){
            printf("smallsh: cannot open %s for output\n", args[numArgs - outputOffset]);
            exit(1);
          }   
        }

      	// find greater offset
        i = 0;
        if (inputOffset > outputOffset){
          i = inputOffset + 1;
        }
        else if (outputOffset > inputOffset){
          i = outputOffset + 1;
        }

        // move the pointer 
        for (j = i; j > 0; j--){
          *next--;
        }

        // add NULL in child
        *next = NULL;

        execvp(args[0], args);
        printf("%s", args[0]);
        fflush(NULL);
        perror(" ");  
        exit(1); 
      }
      // parent process
      else if (cpid == -1) {   
        // can't fork print error
        printf("%s", args[0]);
        fflush(NULL);                 
        perror(" ");
      } 
      else
      {
        // test if its a bg process
        if (isBackgroundProcess == true){
          printf("background pid is %d\n", cpid);

          isBackgroundProcess = false;

          // add bg process array
          if (cur < MAX_PIDS){  
            bgpid[cur++] = cpid;
          }
        } 
        else{
          // reset value of signal number
          signalNum = 0;                     

          // assign cpid to global variable
          fgpid = cpid;

          // set interrupt handler for fg process 
          sigaction(SIGINT, &foreground_act, NULL);

          // wait for fg child process
          fgpid = waitpid(fgpid, &status, 0);

          // restore to ignore interrupts
          sigaction(SIGINT, &reg_act, NULL);

          // reset global variable 
          fgpid = INT_MAX;

          // if process was terminated by signal, print message
          if (signalNum != 0){
            printf("terminated by signal %d\n", signalNum);
          }   
        }
      }
    }
  }
  while(repeat == true);

	return 0;
}	

void sigintHandler(){

	if (fgpid != INT_MAX){
		kill(fgpid, SIGKILL);
 
    signalNum = 2;  
  }  

	return;
}

void bgHandler(int sig, siginfo_t* info, void* vp)
{
	// include conditional for debug purpose
  if (DEBUG){
    printf("You're in the bgHandler.\n");
  }

  pid_t ref_pid = info->si_pid; 

  if (ref_pid != fgpid && completed_cur < MAX_PIDS){
  	completed_pid[completed_cur++] = ref_pid;
  } 

  return;
}