#include <stdio.h>     // fprintf(), perror()
#include <stdlib.h>    // exit()
#include <unistd.h>    // read(), write(), close()
#include <string.h>    // strlen()
#include <fcntl.h>     // open()
#include <stdlib.h>    // atoi()

#define LED_Bar_Array_file "/dev/LED_Bar_Array_device"
#define LED_7_seg_file "/dev/LED_7_seg_device"


void LED_Bar_Array_writer(char data);
void LED_7_seg_writer(char data);

int main(int argc, char* argv[]){
    uint8_t data = 1;
    for(int i=0;i<9;i++){
        LED_Bar_Array_writer(data);
        data <<= 1;
        sleep(0.5);
    }
    for(int i=0;i<21;i++){
        LED_Bar_Array_writer(i);
        sleep(0.5);
    }
    return 0;
}

void LED_Bar_Array_writer(char data){
    int fd;
    // open device file
    if((fd = open(LED_Bar_Array_file, O_RDWR)) < 0) {
        perror(LED_Bar_Array_file);
        exit(EXIT_FAILURE);
    }
    if(write(fd, &data, 1) == -1) {
        perror("write()");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void LED_7_seg_writer(char data){
    int fd;
    // open device file
    if((fd = open(LED_7_seg_file, O_RDWR)) < 0) {
        perror(LED_7_seg_file);
        exit(EXIT_FAILURE);
    }
    if(write(fd, &data, 1) == -1) {
        perror("write()");
        exit(EXIT_FAILURE);
    }
    close(fd);
}