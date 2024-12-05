#include <stdio.h>      // printf
#include <sys/shm.h>    // shmget, shmat, shmctl
#include <sys/ipc.h>    // key_t
#include <string.h>     // memset, strcmp
#include <stdlib.h>     // atoi, exit
#include <unistd.h>     // sleep
#include <signal.h>     // kill, SIGUSR1

// ANSI color codes
#define NONE "\033[m"
#define GREEN "\033[7;49;92m"

typedef struct {
    int guess;
    char result[8];
} shared_data;
int SHM_SIZE = sizeof(shared_data);
int shm_id;
shared_data* create_shared_memory(int shm_key);
void sigint_handler();
void atexit_fun();

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <key> <upper_bound> <Game PID>\n", argv[0]);
        return -1;
    }
    atexit(atexit_fun); // register atexit function
    signal(SIGINT, sigint_handler); // register signal handler

    int key = atoi(argv[1]);
    int upper_bound = atoi(argv[2]);
    int game_pid = atoi(argv[3]);

    int lower_bound = 0;
    int guess = 0;

    shared_data *shm;
    shm = create_shared_memory(key);

    while(1){
        guess = (upper_bound + lower_bound) / 2;
        shm->guess = guess;
        printf("Guess: %d\n", shm->guess);
        kill(game_pid, SIGUSR1);
        sleep(1);
        if(strcmp(shm->result, "GREATER") == 0){ // if guess is greater
            upper_bound = guess;
        } else if(strcmp(shm->result, "LESS") == 0){ // if guess is less
            lower_bound = guess;
        } else {
            printf( GREEN "Bingo: %d\n" NONE, guess);
            break;
        }
    }
    printf("Game Over\n");
    return 0;
}

shared_data* create_shared_memory(int shm_key){
    // Shared Memory
    if((shm_id = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(-1);
    }
    /* attach shared memory */ 
    shared_data *shm;
    if((shm = shmat(shm_id, NULL, 0)) == (shared_data *) -1){
        perror("shmat");
        exit(-1);
    }
    /* set shared memory to 0 */
    memset(shm, 0, SHM_SIZE);

    return shm;
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