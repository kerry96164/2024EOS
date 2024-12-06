// This program is a simple TCP client that connects to a server, sends messages, and receives responses.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define BUFFER_SIZE 256

int sockfd = -1;
void close_socket();
void sigint_handler(int signum);

int main(int argc, char* argv[]) {
    atexit(close_socket); // close socket when exit
    signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C
    if(argc != 3){
        printf(" Usage: ./hw2_client <IP> <Port>\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    unsigned short server_port = atoi(argv[2]);

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    while(1){
        // Get message from user
        printf("Enter message: ");
        fgets(message, BUFFER_SIZE, stdin);

         // Send message to server
        send(sockfd, message, BUFFER_SIZE, 0);

        // Receive response from server
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Receive error");
            exit(EXIT_FAILURE);
        }

        // Null-terminate the received data
        buffer[bytes_received] = '\0';

        // Print server response
        printf("Server response:\n%s\n", buffer); 
    }
    
    // Close socket
    close_socket();

    return 0;
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