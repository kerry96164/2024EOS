#include <stdio.h>      // printf
#include <sys/ipc.h>    // key_t
#include <sys/shm.h>    // shmget, shmat, shmctl
#include <signal.h>     // sigaction
#include <stdlib.h>     // atoi, exit
#include <string.h>     // memset, strcpy
#include <unistd.h>     // getpid, sleep

// ANSI color codes
#define NONE "\033[m"
#define GREEN "\033[7;49;92m"

typedef struct {
    int guess;
    char result[8];
} shared_data;
int SHM_SIZE = sizeof(shared_data);

int shm_id = -1;
key_t shm_key = 0;
int answer = 0;
shared_data *shm;

void create_shared_memory();
void atexit_fun();
void sigint_handler();

void handler(int signo, siginfo_t *info, void *context){
    //printf("Process (%d) sent SIGUSR1\n", info->si_pid);
    if(shm->guess > answer){
        strcpy(shm->result, "GREATER");
    } else if(shm->guess < answer){
        strcpy(shm->result, "LESS");
    } else {
        strcpy(shm->result, "EQUAL");
        printf( GREEN "Bingo: %d\n" NONE, answer);
        exit(0);
    }
    printf("Guess: %d, %s\n", shm->guess, shm->result);
}

int main(int argc, char *argv[]) {
    atexit(atexit_fun); // register atexit function
    signal(SIGINT, sigint_handler); // register signal handler
    if (argc != 3) {
        printf("Usage: %s <key> <answer>\n", argv[0]);
        return -1;
    }
    shm_key = atoi(argv[1]);
    answer = atoi(argv[2]);

    create_shared_memory();

    /* register handler to SIGUSR1 */
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigaction(SIGUSR1, &act, NULL);
    printf("Answer: %d\n", answer);
    printf("Game PID: %d\n", getpid());
    while(1){
        sleep(1);
    }
    return 0;
}

void create_shared_memory(){
    // Shared Memory
    if((shm_id = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(-1);
    }
    /* attach shared memory */ 
    if((shm = shmat(shm_id, NULL, 0)) == (shared_data *) -1){
        perror("shmat");
        exit(-1);
    }
}

void sigint_handler(){
    printf("\n Received interrupt signal (Ctrl-C). \n");
    exit(0);
}

void atexit_fun(){
    printf("\nExiting program safely...\n");
    if (shm_id >= 0){
        if(shmctl(shm_id, IPC_RMID, NULL) >=0 ){ // remove shared memory
            printf("Removed shared memory %d\n", shm_id);
        }
    }
}