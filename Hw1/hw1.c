#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit(), atoi(), system()
#include <unistd.h>     // read(), write(), close()
#include <string.h>     // strlen()
#include <fcntl.h>      // open()
#include <signal.h>     // signal()
#include <stdint.h>     // uint8_t
#include <sys/wait.h>   // Wait for Process Termination

#define RPI

#define LED_Bar_Array_file "/dev/LED_Bar_Array_device"
#define LED_7_seg_file "/dev/LED_7_seg_device"
#define NUM_SHOP 3
#define NUM_FOOD 2

typedef struct {
    char item[20];
    int  price;
} food_t;

typedef struct {
    char name[20];
    int  distance;
    food_t menu[NUM_FOOD];
} shop_t;

shop_t shop[NUM_SHOP];

int bar_array_file = -1, seven_seg_file = -1;

void LED_Bar_Array_writer(char data);
void LED_7_seg_writer(char data);
void sigint_handler(int signum);
void open_device();
void close_device();
void init_data();
void deliverySystem();

void show_shop_list();
void order_shop();
void order_menu(int s);
void confirm_order(int total,int dist);
void led_show_price(int price);
void led_show_dist(int dist);

int main(int argc, char* argv[]){
    signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C

#ifdef RPI
    if (atexit(close_device) != 0) {
        fprintf(stderr, "Failed to register cleanup function.\n");
        return 1;
    }
    open_device();
#endif
    init_data();

    deliverySystem(); 

    return 0;
}
#ifdef RPI
void LED_Bar_Array_writer(char data){
    // 0b00000000 0~255
    if(write(bar_array_file, &data, 1) == -1) {
        perror("Error writing to LED Bar Array");
        exit(EXIT_FAILURE);
    }
}
void LED_7_seg_writer(char data){
    // 0~9 : 0~9 no dp
    // 10~19 : 0~9 with dp
    // 20 : space 
    if(write(seven_seg_file, &data, 1) == -1) {
        perror("Error writing to LED 7 Segment Display");
        exit(EXIT_FAILURE);
    }
}

void open_device(){
    // Open device file
    if((bar_array_file = open(LED_Bar_Array_file, O_RDWR)) < 0) {
        perror("Failed to open LED Bar Array device");
        exit(EXIT_FAILURE);
    }
    if((seven_seg_file = open(LED_7_seg_file, O_RDWR)) < 0) {
        perror("Failed to open LED 7 Segment device");
        exit(EXIT_FAILURE);
    }
}

// register in atexit
void close_device(){
    if(bar_array_file>=0){
        close(bar_array_file);
        bar_array_file = -1;
    }
    if(seven_seg_file>=0){
        close(seven_seg_file);
        seven_seg_file = -1;
    }
    //printf("Exiting program safely...Succsee\n");
}

#else
void LED_Bar_Array_writer(char data){
    printf("Bar: %d\n", data);
}
void LED_7_seg_writer(char data){
    printf("7seg: %d\n", data);
}
#endif

// SIGINT： Ctrl-C
void sigint_handler(int signum) {
    printf("\n Received interrupt signal (Ctrl-C). \nExiting program safely...\n");
    exit(0);
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
    snprintf(shop[2].menu[0].item, sizeof(shop[2].menu[0].item), "fried rice");
    shop[2].menu[0].price = 120;
    snprintf(shop[2].menu[1].item, sizeof(shop[2].menu[1].item), "egg-drop soup");
    shop[2].menu[1].price = 50;
}

/*
deliverySystem()
    |->1. Shop list => show_shop_list();
    |->2. Order     => order_shop();
        |-> Choose Shop => order_menu(shop_choice);
            | Choose Food : add to total
*/

// deliverySystem main
void deliverySystem(){
    int choice;
    while(1){
        LED_7_seg_writer(20);   // space
        LED_Bar_Array_writer(0);// space
        system("clear");
        printf("1. Shop list\n");
        printf("2. Order\n");
        printf("Choose an option: ");
        scanf("%d", &choice);

        switch(choice){
            case 1:
                show_shop_list();
                break;
            case 2:
                order_shop();
                break;
            default:
                printf("Invalid option. Please select 1 or 2...\n");
                sleep(1);
                break;
        }
    }
}

void show_shop_list() {
    system("clear");
    for(int i=0;i<NUM_SHOP;i++){
        printf("%-14s: %d km\n", shop[i].name, shop[i].distance);
    }
    printf("\nPress any key to return to the main menu...\n");
    getchar(); // Clear buffer
    getchar(); // Wait for user to press any key
}

void order_shop(){
    int shop_choice;
    system("clear");
    printf("Please choose from 1~%d\n",NUM_SHOP);
    // Show shop
    for(int i=0;i<NUM_SHOP;i++){
        printf("%d. %s\n", i+1, shop[i].name);
    }
    printf("Choose a restaurant: ");
    scanf("%d", &shop_choice);
    
    if (shop_choice < 1 || shop_choice > NUM_SHOP) {
        printf("Invalid restaurant choice...\n");
        sleep(1);
        return;
    }
    order_menu(shop_choice-1);
}

void order_menu(int s) {
    int item_choice, quantity, total = 0;
    while (1) {
        system("clear");
        printf("[%s] Cart: $%d\n",shop[s].name, total);
        printf("Please choose from 1~4\n");
        // Show menu
        for(int i=0;i<NUM_FOOD;i++){
            printf("%d. %-14s: $%3d\n", i+1, shop[s].menu[i].item, shop[s].menu[i].price);
        }
        printf("3. confirm\n");
        printf("4. cancel\n");
        printf("Choose an option: ");
        
        scanf("%d", &item_choice);

        if (item_choice == 4) { // cancel
            printf("Order cancelled. Returning to main menu...\n");
            sleep(1);
            return;
        } else if (item_choice == 3) { // confirm
            if(!total){ // no order
                printf("Cart is empty...\n");
                sleep(1);
                continue;
            }
            confirm_order(total, shop[s].distance);
            return;
        } else if (item_choice < 1 || item_choice > NUM_FOOD) {
            printf("Invalid choice...\n");
            sleep(1);
            continue;
        }
        // choose food
        printf("How many? ");
        scanf("%d", &quantity);
        total += shop[s].menu[item_choice-1].price * quantity;
        if (total<0) total =0;        
    }
}

void confirm_order(int total,int dist) {
    printf("Total:    $%2d\n", total);
    printf("Distance:  %2d\n", dist);
    printf("\nStarting delivery...\n");
    printf("Please wait for a few minutes...\n");

    /* now create new process */
    pid_t pid = fork();
    if (pid == 0){      /* child process */
        led_show_price(total);
        exit(0);
    }
    else if(pid < 0){            /* fork failure */ 
        perror("fork failure"); /* display error message */
        exit(-1); 
    }
    
    led_show_dist(dist);
    printf("Your order has arrived! Please pick up your meal.\n");
    printf("Press any key to return to the main menu...\n");
    wait(NULL);
    getchar(); // Clear buffer
    getchar(); // Wait for user to press any key
}

void led_show_price(int price){
    char str[10] = {};
    snprintf(str, sizeof(str), "%d", price);
    for(int i=0;i<strlen(str);i++){
        LED_7_seg_writer(20); // space
        usleep(100000);
        LED_7_seg_writer(str[i]-'0');
        usleep(500000);
    }
}

void led_show_dist(int dist){
    u_int8_t d = (1 << dist)-1;
    for(;d>0;d>>=1){
        LED_Bar_Array_writer(d);
        sleep(1);
    }
    LED_Bar_Array_writer(0);
}
