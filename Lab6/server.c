#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // sockaddr 相關
#include <signal.h>   // signal()
#include <unistd.h> // dup2() close()
#include <string.h>
#include <sys/wait.h> // waitpid()
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

int sockfd, reply_sockfd;
int shm_id = -1;
int sem = -1; 
pid_t pid;
struct sockaddr_in clientAddr;
int client_len = sizeof(clientAddr);

#define SEM_MODE 0666 /* rw(owner)-rw(group)-rw(other) permission */ 
#define SEM_KEY  95843 
#define BUFFER_SIZE 1024 // socket buffer size
#define SHM_SIZE sizeof(int)


int P(int s);
int V(int s);
int recv_data(int sockfd, char *data);
int creat_socket(int port);
void sigint_handler();
void sigchld_handler();
void exit_fun();
void handle_request(int sockfd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: server <Port>\n");
        return -1;
    }
    signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C
    signal(SIGCHLD, sigchld_handler); // when child process end
    atexit(exit_fun); // close socket when exit

    // Socket
    int port = atoi(argv[1]);
    int sockfd = creat_socket(port);

    // Semaphore
    /* create semaphore */ 
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE); 
    if (sem < 0){ 
        fprintf(stderr, "Sem %d creation failed: %s\n", SEM_KEY,  
                            strerror(errno)); 
        exit(-1); 
    } 
    /* initial semaphore value to 1 (binary semaphore) */ 
    if ( semctl(sem, 0, SETVAL, 1) < 0 ) { 
        fprintf(stderr, "Unable to initialize Sem: %s\n", strerror(errno)); 
        exit(0); 
    }     
    printf("Semaphore %d has been created & initialized to 1\n", SEM_KEY); 

    // Shared Memory
    if((shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(-1);
    }
    /* attach shared memory */ 
    int *shm;
    if((shm = shmat(shm_id, NULL, 0)) == (int *) -1){
        perror("shmat");
        exit(-1);
    }
    memset(shm, 0, SHM_SIZE);
    
    while(1){
        // Accept Connect and fork
        reply_sockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &client_len);
        if (reply_sockfd < 0) {
            printf("Accept failed!\n");
            perror("accept");
            close(sockfd);
            exit(-1);
        }
        printf("client [%s:%d] --- connect\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        pid = fork();
        if (pid < 0) {
            printf("Fork failed!\n");
            perror("fork");
            close(sockfd);
            close(reply_sockfd);
            exit(-1);
        }
        if (pid == 0) {
            // Child Process
            close(sockfd);
            handle_request(reply_sockfd);
            close(reply_sockfd);
            exit(0);
        } else {
            // Parent Process
            close(reply_sockfd);
        }
    }
}

// handle client request
void handle_request(int sockfd){
    char buffer[BUFFER_SIZE];
    int len;

    while(1){
        len = recv_data(sockfd, buffer);
        buffer[len] = '\0';
        int money = atoi(buffer);
        //printf("Received: %d\n", money);
        P(sem);
        int *shm = shmat(shm_id, NULL, 0);
        *shm += money;
        //printf("Money: %d\n", money);
        if(money < 0){
            printf("After withdraw: %d\n", *shm);
        }else{
            printf("After deposit: %d\n", *shm);
        }
        shmdt(shm);
        V(sem);
    }
}

// socket receive data
int recv_data(int sockfd, char *data) {
    data[0] = '\0';
    if (read(sockfd, data, BUFFER_SIZE) < 0) {
        perror("read");
        exit(-1);
    }else if(strlen(data) == 0){
        printf("Client [%s:%d] --- disconnected.\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        exit(0);
    }
    return strlen(data);
}


int creat_socket(int port) {
    sockfd = socket(AF_INET , SOCK_STREAM , 0); // Ipv4 TCP
    if (sockfd < 0) {
        printf("Fail to create a socket.\n");
    }
    // Address
    const struct sockaddr_in serverAddr = {
        .sin_family =AF_INET,             // Ipv4 (unsigned short int)
        .sin_addr.s_addr = INADDR_ANY,    // 沒有指定 ip address (struct in_addr)
        .sin_port = htons(port)          // 綁定 port (unsigned short int)
    };
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Bind socket failed!\n");
        perror("bind");
        close(sockfd);
        exit(-1);
    }
    // Listen
    if (listen(sockfd, 5) == -1) {
        printf("socket %d listen failed!\n", sockfd);
        perror("listen");
        close(sockfd);
        exit(-1);
    }
    printf("server [%s:%d] --- ready\n", 
        inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    return sockfd;
}

void sigint_handler(){
    printf("\n Received interrupt signal (Ctrl-C). \nExiting program safely...\n");
    exit(0);
}
void sigchld_handler(){
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
void exit_fun(){
    if(sockfd>=0){
        close(sockfd);
        sockfd = -1;
    }
    if(reply_sockfd>=0){
        close(reply_sockfd);
        reply_sockfd = -1;
    }
    printf("Socket closed.\n");

    if (shm_id >= 0){
        if(shmctl(shm_id, IPC_RMID, NULL) >=0 ){ // remove shared memory
            printf("Removed shared memory %d\n", shm_id);
        }
    }
    if (sem >= 0){
        if (semctl(sem, 0, IPC_RMID) >= 0){ // remove semaphore
            printf("Removed semaphore %d\n", sem);
        }
    }
    
    //exit(0);
}

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