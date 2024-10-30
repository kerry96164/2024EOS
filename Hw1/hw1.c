#include <stdio.h>      // fprintf(), perror()
#include <stdlib.h>     // exit(), atoi()
#include <unistd.h>     // read(), write(), close()
#include <string.h>     // strlen()
#include <fcntl.h>      // open()
#include <signal.h>     // signal()
#include <stdint.h>     // uint8_t

#define LED_Bar_Array_file "/dev/LED_Bar_Array_device"
#define LED_7_seg_file "/dev/LED_7_seg_device"


void LED_Bar_Array_writer(char data);
void LED_7_seg_writer(char data);
void sigint_handler(int signum);
void open_device();
void close_device();

int bar_array_file = -1, seven_seg_file = -1;

int main(int argc, char* argv[]){
    signal(SIGINT, sigint_handler); // SIGINT： Ctrl-C
    // when exit will call 
    if (atexit(close_device) != 0) {
        fprintf(stderr, "Failed to register cleanup function.\n");
        return 1;
    }

    open_device();

    u_int8_t data = 1;
    for(int i=0;i<9;i++){
        LED_Bar_Array_writer(data);
        printf("Write %d\n",data);
        data <<= 1;
        sleep(1);
    }
    for(int i=0;i<21;i++){
        LED_7_seg_writer(i);
        printf("Write %d\n",i);
        sleep(1);
    }
    return 0;
}

void LED_Bar_Array_writer(char data){
    if(write(bar_array_file, &data, 1) == -1) {
        perror("Error writing to LED Bar Array");
        exit(EXIT_FAILURE);
    }
}

void LED_7_seg_writer(char data){
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
    printf("Exiting program safely...Succsee\n");
}

// SIGINT： Ctrl-C
void sigint_handler(int signum) {
    printf("\n Received interrupt signal (Ctrl-C). \nExiting program safely...\n");
    exit(0);
}