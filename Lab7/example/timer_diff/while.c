#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int pid = 0;
int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage: %s <core number>\n", argv[0]);
        return -1;
    }
    int core = atoi(argv[1]);
    for(int i = 0; i < core; i++){
        pid = fork();
        if(pid == 0){
            printf("Process (%d) is running on core %d\n", getpid(), i);
            while(1);
            break;
        }
    }
    return 0;
}