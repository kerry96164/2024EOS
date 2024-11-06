#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit(), atoi(), system()
#include <unistd.h>     // read(), write(), close()
#include <string.h>     // strlen()
#include <fcntl.h>      // open()
#include <signal.h>     // signal()
#include <stdint.h>     // uint8_t
#include <sys/wait.h>   // Wait for Process Termination
#include <sys/socket.h>
#include <arpa/inet.h>  // sockaddr 相關
#include <string.h>     // strtok()

#define NUM_SHOP 3
#define NUM_FOOD 2
#define BUFFER_SIZE 256

typedef struct {
    char item[20];
    int  price;
    int  quantity;
} food_t;

typedef struct {
    char name[20];
    int  distance;
    food_t menu[NUM_FOOD];
} shop_t;

shop_t shop[NUM_SHOP];
shop_t cart; // shopping cart

unsigned short port; // 0~65535
int sockfd = -1;
int reply_sockfd = -1;

int bar_array_file = -1, seven_seg_file = -1;

void sigint_handler(int signum);
void init_data();
void send_data(int sockfd, char *data);
void recv_data(int sockfd, char *data);
void close_socket();
void deliverySystem();
void show_shop_list();
void order(char food[], char quantity[]);
void confirm();
void show_cart();
void init_cart();

int main(int argc, char* argv[]){
    signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C
    atexit(close_socket); // close socket when exit
    if(argc != 2){
        printf(" Usage: ./hw2 <Port>\n");
        return -1;
    }
    port = atoi(argv[1]);
    init_data(); // Initial shop & food data
    /* Socket */
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
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); // Force using socket address already in use
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

    while(1){
        // Accept Connection
        reply_sockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &client_len);
        printf("client [%s:%d] --- connect\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        deliverySystem();
    }
    return 0;
}

// SIGINT： Ctrl-C
void sigint_handler(int signum) {
    printf("\n Received interrupt signal (Ctrl-C). \nExiting program safely...\n");
    close_socket();
    exit(0);
}

// close socket
void close_socket(){
    if(sockfd>=0){
        close(sockfd);
        sockfd = -1;
    }
}

// Initial shop & food data
void init_data(){
    // Dessert shop
    shop[0].distance = 3;
    snprintf(shop[0].name, sizeof(shop[0].name), "Dessert shop");
    snprintf(shop[0].menu[0].item, sizeof(shop[0].menu[0].item), "cookie");
    shop[0].menu[0].price = 60;
    snprintf(shop[0].menu[1].item, sizeof(shop[0].menu[1].item), "cake");
    shop[0].menu[1].price = 80;

    // Beverage shop
    shop[1].distance = 5;
    snprintf(shop[1].name, sizeof(shop[1].name), "Beverage shop");
    snprintf(shop[1].menu[0].item, sizeof(shop[1].menu[0].item), "tea");
    shop[1].menu[0].price = 40;
    snprintf(shop[1].menu[1].item, sizeof(shop[1].menu[1].item), "boba");
    shop[1].menu[1].price = 70;

    // Diner
    shop[2].distance = 8;
    snprintf(shop[2].name, sizeof(shop[2].name), "Diner");
    snprintf(shop[2].menu[0].item, sizeof(shop[2].menu[0].item), "fried-rice");
    shop[2].menu[0].price = 120;
    snprintf(shop[2].menu[1].item, sizeof(shop[2].menu[1].item), "Egg-drop-soup");
    shop[2].menu[1].price = 50;
}

void init_cart(){
    // Initial cart
    cart.name[0] = '\0';
    cart.distance = -1;
    for(int i=0;i<NUM_FOOD;i++){
        cart.menu[i].item[0] = '\0';
        cart.menu[i].price = 0;
        cart.menu[i].quantity = 0;
    }
}

// socket send data
void send_data(int sockfd, char *data) {
    if (write(sockfd, data, BUFFER_SIZE) < 0) {
        perror("write");
        exit(-1);
    }
    printf("Send data: %s\n", data);
}

// socket receive data
void recv_data(int sockfd, char *data) {
    data[0] = '\0';
    if (read(sockfd, data, BUFFER_SIZE) < 0) {
        perror("read");
        exit(-1);
    }
    printf("Receive data: %s\n", data);
}

// deliverySystem main
void deliverySystem(){

    char buffer[BUFFER_SIZE];
    char cmd1[20], cmd2[20], cmd3[20];

    while(1){
        recv_data(reply_sockfd, buffer);
        if(strlen(buffer) == 0){
            printf("Client disconnected.\n");
            return;
        }
        cmd1[0] = '\0'; // shop, order, cancel, confirm
        cmd2[0] = '\0'; // food name
        cmd3[0] = '\0'; // quantity
        sscanf(buffer, "%s %s %s\n", cmd1, cmd2, cmd3);
        //printf("%s %s %s\n", cmd1, cmd2, cmd3);
        if(strcmp(cmd1, "shop") == 0 && strcmp(cmd2, "list") == 0){
            show_shop_list();
        } else if(strcmp(cmd1, "order") == 0){
            order(cmd2, cmd3);
        } else if(strcmp(cmd1, "cancel") == 0){
            init_cart();
            return;
        } else if(strcmp(cmd1, "confirm") == 0){
            if(cart.name[0] == '\0'){
                send_data(reply_sockfd, "Please order some meals\n");
            } else{
                confirm();
                init_cart();
                return;
            }
        } else {
            printf("Invalid option. %s %s %s\n", cmd1, cmd2, cmd3);
            continue;
        }
    }
}

void show_shop_list() {
    /* char data[BUFFER_SIZE] = "Dessert shop:3km\n"
                             " - cookie:$60|cake:$80\n"
                             "Beverage shop:5km\n"
                             " - tea:$40|boba:$70\n"
                             "Diner:8km\n"
                             " - fried-rice:$120|Egg-drop-soup:$50\n";
    send_data(reply_sockfd, data); */
    char data[BUFFER_SIZE] = "";
    char cat_buf[BUFFER_SIZE];
    for(int i=0;i<NUM_SHOP;i++){
        sprintf(cat_buf, "%s:%dkm\n", shop[i].name, shop[i].distance);
        strcat(data, cat_buf);
        strcat(data, "- ");
        for(int j=0;j<NUM_FOOD;j++){
            sprintf(cat_buf, "%s:$%d", shop[i].menu[j].item, shop[i].menu[j].price);
            strcat(data, cat_buf);
            if(j<NUM_FOOD-1){
                strcat(data, "|");
            }
        }
        strcat(data, "\n");
    }
    send_data(reply_sockfd, data);

}

void order(char food[], char quantity[]){
    printf("order %s %s\n", food, quantity);
    if(cart.name[0] == '\0'){
        for(int i=0;i<NUM_SHOP;i++){
            for(int j=0;j<NUM_FOOD;j++){
                if(strcmp(shop[i].menu[j].item, food) == 0){
                    cart = shop[i];
                    cart.menu[j].quantity = atoi(quantity);
                }
            }
        }
    }else{
        for(int i=0;i<NUM_FOOD;i++){
            if(strcmp(cart.menu[i].item, food) == 0){
                cart.menu[i].quantity += atoi(quantity);
            }
        }
    }
    show_cart();
    return;
}

void confirm() {
    int delivery_time = cart.distance;
    int total = 0;
    char data[BUFFER_SIZE] = "";
    for(int i=0;i<NUM_FOOD;i++){
        total += cart.menu[i].price * cart.menu[i].quantity;
    }
    send_data(reply_sockfd, "Please wait a few minutes...\n");
    sleep(delivery_time);

    // Delivery has arrived and you need to pay 260$
    printf("%d\n",total);
    sprintf(data, "Delivery has arrived and you need to pay %d$\n", total);
    send_data(reply_sockfd, data);
    return;
}

void show_cart() {
    char data[BUFFER_SIZE] = "";
    char cat_buf[BUFFER_SIZE];
    int n = 0; // number of >0 items
    for(int i=0;i<NUM_FOOD;i++){
        if(cart.menu[i].quantity > 0){
            sprintf(cat_buf, "%s %d", cart.menu[i].item, cart.menu[i].quantity);
            if(n){ // not first
                strcat(data, "|");
            }
            strcat(data, cat_buf);
            n++;
        }
    }
    strcat(data, "\n");
    send_data(reply_sockfd, data);
}
