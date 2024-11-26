/* 
 * select.c 
 */ 
 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/time.h> 
#include <sys/types.h> 
#include <time.h> 
#include <unistd.h> 
 
#define max(a, b) ((a > b) ? a : b) 
 
void ChildProcess(int *pfd, int sec) 
{ 
    char buffer[100]; 
    
    /* close unused read end */ 
    close(pfd[0]); 
    
    /* sleep a random time to wait parent process enter select() */ 
    printf("Child process (%d) wait %d secs\n", getpid(), sec); 
    sleep(sec); 
    
    /* write message to parent process */ 
    memset(buffer, 0, 100); 
    sprintf(buffer, "Child process (%d) sent message to parent process\n", 
    getpid()); 
    write(pfd[1], buffer, strlen(buffer)); 
    
    /* close write end */ 
    close(pfd[1]); 
    
    exit(EXIT_SUCCESS); 
} 
 
int main(int argc, char *argv[])  
{ 
    int pfd1[2], pfd2[2]; /* pipe's fd */ 
    int cpid1, cpid2; /* child process id */ 
    fd_set rfds, arfds; 
    int max_fd; 
    struct timeval tv; 
    int retval; 
    int fd_index; 
    char buffer[100]; 
    
    /* random seed */ 
    srand(time(NULL)); 
    
    /* create pipe */ 
    pipe(pfd1); 
    pipe(pfd2); 
    
    /* create 2 child processes and set corresponding pipe & sleep time */ 
    cpid1 = fork(); 
    if (cpid1 == 0) 
        ChildProcess(pfd1, random() % 5); 
    
    cpid2 = fork(); 
    if (cpid2 == 0) 
        ChildProcess(pfd2, random() % 4); 
    
    /* close unused write end */ 
    close(pfd1[1]); 
    close(pfd2[1]); 
    
    /* set pfd1[0] & pfd2[0] to watch list */ 
    FD_ZERO(&rfds); 
    FD_ZERO(&arfds); 
    FD_SET(pfd1[0], &arfds); 
    FD_SET(pfd2[0], &arfds); 
    max_fd = max(pfd1[0], pfd2[0]) + 1; 
    
    /* Wait up to five seconds. */ 
    tv.tv_sec = 5; 
    tv.tv_usec = 0; 
    
    while(1) 
    { 
        /* config fd_set for select */ 
        memcpy(&rfds, &arfds, sizeof(rfds)); 
        
        /* wait until any fd response */ 
        retval = select(max_fd, &rfds, NULL, NULL, &tv); 
        
        if (retval == -1) { /* error */ 
            perror("select()"); 
            exit(EXIT_FAILURE); 
        } 
        else if (retval) { /* # of fd got respone */ 
            printf("Data is available now.\n"); 
        } 
        else { /* no fd response before timer expired */ 
            printf("No data within five seconds.\n"); 
            break; 
        } 
        
        /* check if any response */ 
        for (fd_index = 0; fd_index < max_fd; fd_index++) 
        { 
            if (!FD_ISSET(fd_index, &rfds)) 
                continue; /* no response */ 
            
            retval = read(fd_index, buffer, 100); 
            
            if (retval > 0)  /* read data from pipe */ 
                printf("%.*s", retval, buffer); 
            else if (retval < 0) /* error */ 
                perror("pipe read()"); 
            else {   /* write fd closed */ 
                /* close read fd */ 
                close(fd_index); 
                /* remove fd from watch list */ 
                FD_CLR(fd_index, &arfds); 
            } 
        } 
    } 
    return 0; 
}