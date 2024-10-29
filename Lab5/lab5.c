#include <errno.h>      /* Errors */ 
#include <stdio.h>      /* Input/Output */ 
#include <stdlib.h>     /* General Utilities */             // atoi
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <sys/wait.h>   /* Wait for Process Termination */ 
#include <unistd.h>     /* Symbolic Constants */ 
#include <sys/socket.h>
#include <arpa/inet.h>  // sockaddr 相關
#include <signal.h>   // signal()

pid_t childpid; /* variable to store the child's pid */
unsigned short port; // 0~65535
int sockfd;

void handler(int signum);
void childfunc(void);
void parentfunc(void);

int main(int argc, char *argv[]) { 
    signal(SIGINT, handler);
    signal(SIGCHLD, handler);

    if(argc != 2){
        printf(" Usage: lab5 <Port>");
        return -1;
    }
    port = atoi(argv[1]);

    /* Socket */
    int reply_sockfd;
    struct sockaddr_in clientAddr;
    int client_len = sizeof(clientAddr);

    // 建立 socket, 並且取得 socket_fd
    sockfd = socket(PF_INET , SOCK_STREAM , 0); // Ipv4 TCP
    if (sockfd < 0) {
        printf("Fail to create a socket.\n");
    }
    // Address
    const struct sockaddr_in serverAddr = {
        .sin_family =AF_INET,             // Ipv4 (unsigned short int)
        .sin_addr.s_addr = INADDR_ANY,    // 沒有指定 ip address (struct in_addr)
        .sin_port = htons(port)          // 綁定 port 4444 (unsigned short int)
    };
    // Bind
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Bind socket failed!\n");
        perror("bind");
        close(sockfd);
        exit(0);
    }
    // Listen
    if (listen(sockfd, 5) == -1) {
        printf("socket %d listen failed!\n", sockfd);
        perror("listen");
        close(sockfd);
        exit(0);
    }
    printf("server [%s:%d] --- ready\n", 
        inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    // Accept Connect and fork
    while(1){
        reply_sockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &client_len);
         

    }
    /* now create new process */ 
    childpid = fork();
    if (childpid >= 0) { /* fork succeeded */ 
        if (childpid == 0) { /* fork() returns 0 to the child process */ 
            childfunc(); 
        } else { /* fork() returns new pid to the parent process */ 
            parentfunc(); 
        } 
    } else { /* fork returns -1 on failure */ 
        perror("fork"); /* display error message */ 
        exit(0);  
    }
    return 0; 
} 

void handler(int signum) {
   while (waitpid(-1, NULL, WNOHANG) > 0);
   close(sockfd);
}

void childfunc(void){ 

    exit(0); /* child exits with user-provided return code */ 
}

void parentfunc(void) {   

    exit(0);  /* parent exits */      
} 