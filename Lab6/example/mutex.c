#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
 
#define checkResults(string, val) {   
    if (val) {
        printf("Failed with %d at %s", val, string);  
        exit(1);
    }
} 
 
#define                 NUMTHREADS   3 
 
pthread_mutex_t         mutex = PTHREAD_MUTEX_INITIALIZER; 
int                     sharedData  = 0; 
int                     sharedData2 = 0; 
 
void *theThread(void *parm) { 
    int   rc; 
    
    printf("\tThread %lu: Entered\n", (unsigned long) pthread_self()); 
    
    /* lock mutex */ 
    rc = pthread_mutex_lock(&mutex); 
    checkResults("pthread_mutex_lock()\n", rc); 
    
    /**************** Critical Section *****************/ 
    printf("\tThread %lu: Start critical section, holding lock\n", 
            (unsigned long) pthread_self()); 
    
    /* Access to shared data goes here */ 
    ++sharedData; 
    --sharedData2; 
    
    printf("\tsharedData = %d, sharedData2 = %d\n",  
            sharedData, sharedData2); 
    
    printf("\tThread %lu: End critical section, release lock\n", 
            (unsigned long) pthread_self()); 
    /**************** Critical Section *****************/ 
    
    /* unlock mutex */ 
    rc = pthread_mutex_unlock(&mutex); 
    checkResults("pthread_mutex_unlock()\n", rc); 
    
    return NULL; 
} 
 
int main(int argc, char **argv){ 
    pthread_t             thread[NUMTHREADS]; 
    int                   rc = 0; 
    int                   i; 
    
    /* lock mutex */ 
    printf("Main thread hold mutex to prevent access to shared data\n"); 
    rc = pthread_mutex_lock(&mutex); 
    checkResults("pthread_mutex_lock()\n", rc); 
    
    /* create thread */ 
    printf("Main thread create/start threads\n"); 
    for (i = 0; i < NUMTHREADS; ++i) { 
        rc = pthread_create(&thread[i], NULL, theThread, NULL); 
        checkResults("pthread_create()\n", rc); 
    } 
    
    /* wait for thread creation complete */ 
    printf("Main thread wait a bit until 'done' with the shared data\n"); 
    sleep(3); 
    
    /* unlock mutex */ 
    printf("Main thread unlock shared data\n"); 
    rc = pthread_mutex_unlock(&mutex); 
    checkResults("pthread_mutex_lock()\n",rc); 
    
    /* wait thread complete */ 
    printf("Main thread Wait for threads to complete, " 
    "and release their resources\n"); 
    for (i=0; i <NUMTHREADS; ++i) { 
        rc = pthread_join(thread[i], NULL); 
        checkResults("pthread_join()\n", rc); 
    } 
    
    /* destroy mutex */ 
    printf("Main thread clean up mutex\n"); 
    rc = pthread_mutex_destroy(&mutex); 
    
    printf("Main thread completed\n"); 
    
    return 0; 
}