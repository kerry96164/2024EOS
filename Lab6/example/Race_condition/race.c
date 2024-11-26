/*  
* race.c 
*/

#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h>

#ifdef USE_SEM 
#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */ 
#define SEM_KEY  1122334455 

int sem; 

/* P () - returns 0 if OK; -1 if there was a problem */ 
int P (int s)  { 
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
int V(int s) { 
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
#endif

/* increment value saved in file */ 
void Increment() 
{ 
    int ret; 
    int fd;  /* file descriptor */ 
    int counter; 
    char buffer[100]; 
    int i = 10000; 
    
    while(i){ 
        /* open file */ 
        fd = open("./counter.txt", O_RDWR ); 
        if (fd < 0) { 
            printf("Open counter.txt error.\n"); 
            exit(-1); 
        } 
        
        #ifdef USE_SEM 
        /* acquire semaphore */ 
        P(sem); 
        #endif 
        
        /**************** Critical Section *****************/ 
        /* clear */ 
        memset(buffer, 0, 100); 
        
        /* read raw data from file */ 
        ret = read(fd, buffer, 100); 
        if (ret < 0){ 
            perror("read counter.txt"); 
            exit(-1); 
        } 
        
        /* transfer string to integer & increment counter */ 
        counter = atoi(buffer); 
        counter++; 
        
        /* write back to counter.txt */ 
        lseek(fd, 0, SEEK_SET); /* reposition to the head of file */ 
        
        /* clear */ 
        memset(buffer, 0, 100); 
        sprintf(buffer, "%d", counter); 
        ret = write(fd, buffer, strlen(buffer)); 
        if (ret < 0){ 
            perror("write counter.txt"); 
            exit(-1); 
        } 
        /**************** Critical Section *****************/ 
        
        #ifdef USE_SEM 
        /* release semaphore */ 
        V(sem); 
        #endif 
        
        /* close file */ 
        close(fd); 
        
        i--; 
    } 
} 
 
int main(int argc, char **argv){
    int childpid; 
    int status; 
    
    #ifdef USE_SEM 
    /* create semaphore */ 
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE); 
    if (sem < 0){ 
        fprintf(stderr, "Sem %ld creation failed: %s\n", SEM_KEY,  
                            strerror(errno)); 
        exit(-1); 
    } 
    
    /* initial semaphore value to 1 (binary semaphore) */ 
    if ( semctl(sem, 0, SETVAL, 1) < 0 ) { 
        fprintf(stderr, "Unable to initialize Sem: %s\n", strerror(errno)); 
        exit(0); 
    } 
    
    printf("Semaphore %ld has been created & initialized to 1\n", SEM_KEY); 
    #endif 
    
    /* fork process */ 
    if ((childpid = fork()) > 0){  /* parent */
        Increment(); 
        waitpid(childpid, &status, 0); 
    } 
    else if (childpid == 0){       /* child */ 
        Increment(); 
        exit(0); 
    } 
    else {               /* error */ 
        perror("fork"); 
        exit(-1);  
    } 
    
    #ifdef USE_SEM 
    /* remove semaphore */ 
    if (semctl (sem, 0, IPC_RMID, 0) < 0){ 
        fprintf (stderr, "%s: unable to remove sem %ld\n", argv[0], 
        SEM_KEY); 
        exit(1); 
    } 
    printf("Semaphore %ld has been remove\n", SEM_KEY); 
    #endif 
    return 0; 
} 