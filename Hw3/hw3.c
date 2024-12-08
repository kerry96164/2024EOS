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
#include <sys/types.h>  // pid_t
#include <sys/ipc.h>    // IPC_CREAT
#include <sys/shm.h>    // shmget(), shmat()
#include <sys/sem.h>    // semget(), semop()
#include <errno.h>      // errno

#define NUM_COURIER 2 // number of couriers
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

// Courier status in shared memory
int shm_id = -1;
//int shm_key = 5698;
int *shm_ptr; // courier status
int shm_size = sizeof(int) * NUM_COURIER;

// Semaphore
int sem_id = -1;


unsigned short port; // 0~65535
int sockfd = -1;
int reply_sockfd = -1;
pid_t child_pid = -1;
pid_t timer_pid = -1;
int order_count = 0;
sig_atomic_t sec = 0;

void sigint_handler(int signum);
void zombie_handler(int signum);
void init_data();
void send_data(int sockfd, char *data);
void recv_data(int sockfd, char *data);
void close_handler();
void deliverySystem();
void show_shop_list();
void order(char food[], char quantity[]);
void confirm();
void show_cart();
void init_cart();
void create_shared_memory();
void create_semaphore();
void alarm_handler();
int P(int s, int n);
int V(int s, int n);

int main(int argc, char* argv[]){
    signal(SIGCHLD, zombie_handler); // SIGCHLD: child process terminated
    signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C
    atexit(close_handler); // close socket when exit
    if(argc != 2){
        printf(" Usage: %s <Port>\n", argv[0]);
        return -1;
    }
    port = atoi(argv[1]);
    init_data(); // Initial shop & food data
    create_shared_memory(); // Create shared memory
    create_semaphore(); // Create semaphore
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
    // Alarm
    timer_pid = fork();
    if(timer_pid == 0){
        while(1){
            sleep(1);
            alarm_handler();
        }
    }

    // Accept Connection
    while(1){
        // Accept Connection
        reply_sockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &client_len);
        printf("client %d [%s:%d] --- connect\n", order_count,
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        order_count++;
        child_pid = fork();
        if (child_pid == 0) {// child process
            deliverySystem();
            return 0;
        }
        
    }
    return 0;
}

// SIGINT： Ctrl-C
void sigint_handler(int signum) {
    printf("\n Received interrupt signal (Ctrl-C). \nExiting program safely...\n");
    exit(0);
}
// SIGCHLD: child process terminated
void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
// close socket, shared memory, semaphore
void close_handler(){
    // close socket
    if(sockfd>=0){
        if(close(sockfd)==0){
            printf("Socket %d closed.\n", sockfd);
        }else{
            perror("Failed to close socket");
        }
        sockfd = -1;
    }
    if(reply_sockfd>=0){
        if(close(reply_sockfd)==0){
            printf("Socket %d closed.\n", reply_sockfd);
        }else{
            perror("Failed to close socket");
        }
        reply_sockfd = -1;
    }
    printf("Socket closed.\n");

    if(child_pid&timer_pid){ // parent process
        // remove shared memory
        if (shm_id >= 0){
            if(shmctl(shm_id, IPC_RMID, NULL) >=0 ){
                printf("Removed shared memory %d\n", shm_id);
            }else{
                perror("Failed to remove shared memory");
            }
            shm_id = -1;
        }
        // remove semaphore
        if (sem_id >= 0){
            if (semctl(sem_id, 0, IPC_RMID, 0) >= 0){
                printf("Removed semaphore %d\n", sem_id);
            }else{
                perror("Failed to remove semaphore");
            }
            sem_id = -1;
        }
        printf("Parent process %d closed.\n", getpid());
    }else{
        printf("Child process %d closed.\n", getpid());
    }
    printf("\n");
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
    printf("[%d] Send data: %s", order_count, data);
}

// socket receive data
void recv_data(int sockfd, char *data) {
    data[0] = '\0';
    if (read(sockfd, data, BUFFER_SIZE) < 0) {
        perror("read");
        exit(-1);
    }
    printf("[%d] Receive data: %s", order_count, data);
}

// deliverySystem main
void deliverySystem(){

    char buffer[BUFFER_SIZE];
    char cmd1[20], cmd2[20], cmd3[20];

    while(1){
        recv_data(reply_sockfd, buffer);
        if(strlen(buffer) == 0){
            printf("Client %d disconnected.\n",order_count);
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
            printf("[%d]Invalid option. %s %s %s\n", order_count, cmd1, cmd2, cmd3);
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
    //printf("order %s %s\n", food, quantity);
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
    char buffer[BUFFER_SIZE];
    for(int i=0;i<NUM_FOOD;i++){
        total += cart.menu[i].price * cart.menu[i].quantity;
    }
    
    int min_time = shm_ptr[0];
    int min_index = 0;
    for(int i=0;i<NUM_COURIER;i++){
        if(shm_ptr[i] < min_time){
            //printf("[%d] Courier %d: %d\n", order_count, i, shm_ptr[i]);
            min_time = shm_ptr[i];
            min_index = i;
        }
    }
    printf("[%d] Min_Courier %d: %d\n", order_count, min_index, min_time);
       
    if(delivery_time + min_time > 30){
        send_data(reply_sockfd, "Your delivery will take a long time, do you want to wait?\n");
        recv_data(reply_sockfd, buffer);
        if(strcmp(buffer, "No\n") == 0){
            return;
        }else if(strcmp(buffer, "Yes\n") == 0){
            send_data(reply_sockfd, "Please wait a few minutes...\n");
            // continue
        }else{
            printf("[%d] Invalid option. %s\n", order_count, buffer);
            return;
        }
    }else{
        send_data(reply_sockfd, "Please wait a few minutes...\n");
    }

    P(sem_id, min_index);
    shm_ptr[min_index] += delivery_time;
    V(sem_id, min_index);
    printf("[%d] Delivery time: %d\n", order_count, shm_ptr[min_index]);
    sleep(shm_ptr[min_index]);

    // Delivery has arrived and you need to pay 260$
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

void create_shared_memory(){
    // Shared Memory
    if((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | 0666)) < 0){
        perror("shmget");
        exit(-1);
    }
    /* attach shared memory */ 
    if((shm_ptr = shmat(shm_id, NULL, 0)) == (int*) -1){
        perror("shmat");
        exit(-1);
    }
}

void create_semaphore(){
    // Semaphore
    // create NUM_COURIER semaphores
    if((sem_id = semget(IPC_PRIVATE, NUM_COURIER, IPC_CREAT | 0666)) < 0){
        perror("semget");
        exit(-1);
    }
    // set semaphore value to 1
    unsigned short sem_val[NUM_COURIER];
    for(int i=0;i<NUM_COURIER;i++){
        sem_val[i] = 1;
    }
    if(semctl(sem_id, 0, SETALL, sem_val) < 0){
        perror("semctl");
        exit(-1);
    }
}

void alarm_handler(){
    sec++;
    printf("=========Time: %3d==========\n", sec);

    for(int i=0;i<NUM_COURIER;i++){
        if(shm_ptr[i] > 0){
           P(sem_id, i);
           shm_ptr[i]--;
           V(sem_id, i);
           printf("Courier %d: %d\n", i, shm_ptr[i]);
        }
    }
    return;
}


/* P () - returns 0 if OK; -1 if there was a problem */ 
int P (int s,int n)  { 
    struct sembuf sop;  /* the operation parameters */ 
    sop.sem_num =  n;   /* access the n-th sem in the array */ 
    sop.sem_op  = -1;   /* wait..*/ 
    sop.sem_flg =  0;   /* no special options needed */ 
    
    if (semop (s, &sop, 1) < 0) {  
        fprintf(stderr,"P(): semop failed: %s\n",strerror(errno)); 
        return -1; 
    } else { 
        return 0; 
    } 
}

/* V() - returns 0 if OK; -1 if there was a problem */ 
int V(int s, int n) { 
    struct sembuf sop; /* the operation parameters */ 
    sop.sem_num =  n;   /* access the n-th sem in the array */ 
    sop.sem_op  =  1; /* signal */ 
    sop.sem_flg =  0; /* no special options needed */ 
    
    if (semop(s, &sop, 1) < 0) {  
        fprintf(stderr,"V(): semop failed: %s\n",strerror(errno)); 
        return -1; 
    } else { 
        return 0; 
    } 
}