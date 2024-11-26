/* makesem.c 
 * 
 * This program creates a semaphore. The user should pass  
 * a number to be used as the semaphore key and initial  
 * value as the only command line arguments. If that  
 * identifier is not taken, then a semaphore will be created.  
 * If a semaphore is set so that then no semaphore will be  
 * created. The semaphore is set so that anyone on the system  
 * can use it. 
 */ 
 
#include <errno.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/sem.h> 
 
#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */ 
 
int main (int argc, char **argv) 
{ 
    int s; 
    long int key; 
    int val; 
    
    if (argc != 3)  { 
        fprintf(stderr, 
        "%s: specify a key (long) and initial value (int)\n",  
        argv[0]); 
        
        exit(1); 
    } 
    
    /* get values from command line */ 
    if (sscanf(argv[1], "%ld", &key) != 1){  
        /* convert arg to long integer */ 
        fprintf(stderr, "%s: argument #1 must be an long integer\n", argv[0]); 
        exit(1); 
    } 
    if (sscanf(argv[2], "%d", &val) != 1){  
        /* convert arg to long integer */ 
        fprintf(stderr, "%s: argument #2 must be an integer\n", argv[0]); 
        exit(1); 
    } 
    
    /* semget() takes three parameters */ 
    /* Options: 
    *  IPC_CREAT - create a semaphore if not exists 
    *  IPC_EXCL  - creation fails if it already exists 
    *  SEM_MODE  - access permission  
    */ 
    s = semget(  key,   /* the unique name of the semaphore on the system */ 
                   1,   /* we create an array of semaphores, but just need 1. */ 
                IPC_CREAT | IPC_EXCL | SEM_MODE); 
    
    /* If semget () returns -1 then it failed. However,  
    * if it returns any other number >= 0 then that becomes 
    * the identifier within the program for accessing the semaphore. 
    */ 
    if (s < 0){ 
        fprintf(stderr, 
            "%s: creation of semaphore %ld failed: %s\n", argv[0],  
            key, strerror(errno)); 
        exit(1); 
    } 
    printf("Semaphore %ld created\n", key); 
    
    /* set semaphore (s[0]) value to initial value (val) */ 
    if ( semctl(s, 0, SETVAL, val) < 0 ){ 
        fprintf(stderr, 
            "%s: Unable to initialize semaphore: %s\n",  
            argv[0], strerror(errno)); 
        exit(0); 
    } 
    printf("Semaphore %ld has been initialized to %d\n", key, val); 
    
    return 0; 
}