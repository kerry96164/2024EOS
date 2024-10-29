#include <errno.h>      /* Errors */ 
#include <stdio.h>      /* Input/Output */ 
#include <stdlib.h>     /* General Utilities */ 
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <sys/wait.h>   /* Wait for Process Termination */ 
#include <unistd.h>     /* Symbolic Constants */  

pid_t childpid; /* variable to store the child's pid */  

void childfunc(void){ 
    int retval;     
    /* user-provided return code */ 
    printf("CHILD: I am the child process!\n"); 
    printf("CHILD: My PID: %d\n", getpid()); 
    printf("CHILD: My parent's PID is: %d\n", getppid()); 
    printf("CHILD: Sleeping for 1 second...\n"); 
    sleep(1); /* sleep for 1 second */ 
    printf("CHILD: Enter an exit value (0 to 255): "); 
    scanf(" %d", &retval); 
    printf("CHILD: Goodbye!\n");  
    exit(retval); /* child exits with user-provided return code */ 
}

void parentfunc(void) {   
    int status;     /* receive child's exit status */ 
    printf("PARENT: I am the parent process!\n"); 
    printf("PARENT: My PID: %d\n", getpid()); 
    printf("PARENT: My child's PID is %d\n", childpid); 
    printf("PARENT: I will now wait for my child to exit.\n"); 
    /* wait for child to exit, and store its status */ 
    wait(&status);
    printf("PARENT: Child's exit code is: %d\n", WEXITSTATUS(status)); 
    printf("PARENT: Goodbye!\n"); 
    exit(0);  /* parent exits */      
} 

int main(int argc, char *argv[]) { 
    /* now create new process */
    printf("MAIN: Hello!\n"); 
    childpid = fork();
    if (childpid >= 0) { /* fork succeeded */ 
        if (childpid == 0) { /* fork() returns 0 to the child process */ 
            childfunc(); 
        } else { /* fork() returns new pid to the parent process */ 
            parentfunc(); 
        } 
    } else { /* fork returns -1 on failure */ 
        perror("fork"); /* display error message */ 
        exit(0);  
    }
    printf("MAIN: Goodbye!\n");
    return 0; 
} 