/*  
 * timer.c 
 */ 
#include <stdio.h>  // printf
#include <signal.h> // signal
#include <sys/time.h> // setitimer
#include <stdlib.h> // exit


void timer_handler (int signum) 
{ 
    static int count = 0; 
    printf ("timer expired %d times\n", ++count); 
} 

int main (int argc, char **argv) 
{ 
    struct sigaction sa; 
    struct itimerval timer; 
    
    /* Install timer_handler as the signal handler for SIGVTALRM */ 
    memset (&sa, 0, sizeof (sa)); 
    sa.sa_handler = &timer_handler; 
    sigaction (SIGVTALRM, &sa, NULL); 
    
    /* Configure the timer to expire after 250 msec */ 
    timer.it_value.tv_sec = 0; 
    timer.it_value.tv_usec = 250000; 
    
    /* Reset the timer back to 250 msec after expired */ 
    timer.it_interval.tv_sec = 0; 
    timer.it_interval.tv_usec = 250000; 
    
    /* Start a virtual timer */ 
    setitimer (ITIMER_VIRTUAL, &timer, NULL); 
    
    /* Do busy work */ 
    while (1); 
    
    return 0; 
} 

