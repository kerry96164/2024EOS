/* doodle.c 
 * 
 * This program shows how P () and V () can be implemented,  
 * then uses a semaphore that everyone has access to. 
 */ 
 
#include <errno.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/sem.h> 
#include <unistd.h> 
 
#define DOODLE_SEM_KEY 1122334455 
 
/* P () - returns 0 if OK; -1 if there was a problem */ 
int P(int s){ 
    struct sembuf sop; /* the operation parameters */ 
    sop.sem_num =  0; /* access the 1st (and only) sem in the array */ 
    sop.sem_op  = -1;    /* wait..*/ 
    sop.sem_flg =  0;    /* no special options needed */ 
    
    if (semop (s, &sop, 1) < 0) {  
        fprintf(stderr,"P(): semop failed: %s\n",strerror(errno)); 
        return -1; 
    } else { 
        return 0; 
    } 
} 
 
/* V() - returns 0 if OK; -1 if there was a problem */ 
int V(int s){ 
    struct sembuf sop; /* the operation parameters */ 
    sop.sem_num =  0; /* the 1st (and only) sem in the array */ 
    sop.sem_op  =  1; /* signal */ 
    sop.sem_flg =  0; /* no special options needed */ 
    
    if (semop(s, &sop, 1) < 0) {  
        fprintf(stderr,"V(): semop failed: %s\n",strerror(errno)); 
        return -1; 
    } else { 
        return 0; 
    } 
} 
 
int main ( int argc, char **argv){ 
    int s, secs; 
    long int key; 
    
    if  (argc != 2) { 
        fprintf(stderr, "%s: specify a key (long)\n", argv[0]); 
        exit(1); 
    } 
    
    /* get values from command line */ 
    if (sscanf(argv[1], "%ld", &key)!=1){ 
        /* convert arg to long integer */ 
        fprintf(stderr, 
        "%s: argument #1 must be an long integer\n", argv[0]); 
        exit(1); 
    } 
    
    s = semget(key, 1, 0); 
    
    if (s < 0) { 
        fprintf (stderr, 
        "%s: cannot find semaphore %ld: %s\n",  
        argv[0], key, strerror(errno)); 
        exit(1); 
    } 
    while (1) { 
        printf ("#secs to doodle in the critical section? (0 to exit):"); 
        scanf ("%d",&secs); 
        if (secs == 0) 
            break; 
        printf ("Preparing to enter the critical section..\n"); 
        P(s); 
        printf ("Now in the critical section! Sleep %d secs..\n", secs); 
        while (secs) { 
            printf ("%d...doodle...\n",secs--); 
            sleep (1); 
        } 
        printf ("Leaving the critical section..\n"); 
        V(s); 
    } 
    return 0; 
} 