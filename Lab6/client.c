#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024 // socket buffer size

void close_socket();
void sigint_handler();
void create_socket(char* server_ip, int server_port);
int sockfd = -1;

int main(int argc, char *argv[]) {    
    atexit(close_socket); // close socket when exit
    //signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C
    if(argc != 6){
        printf("Usage: %s <Server IP> <Port> <deposit/withdraw> <amount> <times>\n", argv[0]);
        return -1;
    }
    
    char operation[10];
    int amount, times;
    operation[0] = '\0'; // deposit/withdraw
    amount = 0;
    times = 0;
    sscanf(argv[3], "%9s", operation);
    if(strcmp(operation, "deposit") != 0 && strcmp(operation, "withdraw") != 0){
        printf("Invalid option. %s\n", operation);
        return -1;
    }
    if(sscanf(argv[4], "%d", &amount) != 1){
        printf("Invalid amount. %s\n", argv[4]);
        return -1;
    }
    if(sscanf(argv[5], "%d", &times) != 1){
        printf("Invalid times. %s\n", argv[5]);
        return -1;
    }

    // Socket
    create_socket(argv[1], atoi(argv[2]));
    
    for (int i = 0; i < times; i++){
        char buffer[BUFFER_SIZE];
        buffer[0] = '\0';
        if(operation[0] == 'd'){ // deposit
            sprintf(buffer, "%d", amount);
        }else{ // withdraw
            sprintf(buffer, "-%d", amount);
        }
        if (write(sockfd, buffer, BUFFER_SIZE) < 0) {
            perror("write");
            exit(-1);
        }
        //printf("Sent %d: %s\n", i, buffer);
        usleep(1);
    }

    return 0;
}

void create_socket(char* server_ip, int server_port){
    if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) < 0){
        printf("Fail to create a socket.\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port); 
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
}

// close socket
void close_socket(){
    if(sockfd != -1){
        close(sockfd);
        printf("Socket closed\n");
        sockfd = -1;
    }
}

// SIGINT： Ctrl-C
void sigint_handler(int signum) {
    printf("\n Received interrupt signal (Ctrl-C). \nExiting program safely...\n");
    close_socket();
    exit(0);
}