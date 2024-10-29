#include <stdio.h>     // fprintf(), perror()
#include <stdlib.h>    // exit()
#include <unistd.h>    // read(), write(), close()
#include <string.h>    // strlen()
#include <fcntl.h>     // open()

int main(int argc, char* argv[]){
    char w_buf;
    char r_buf[16] = {};
    int fd, ret;
    
    if(argc != 3) {
        fprintf(stderr, "Usage: ./writer <Text> <device_path>\n");
        exit(EXIT_FAILURE);
    }
    // open device file
    if((fd = open(argv[2], O_RDWR)) < 0) {
        perror(argv[2]);
        exit(EXIT_FAILURE);
    }

    printf("Text: %s . Length: %ld\n",argv[1],strlen(argv[1]));
    for(int i=0;i<strlen(argv[1]);i++){
        w_buf = argv[1][i];
        // write to driver
        if(write(fd, &w_buf, 1) == -1) {
            perror("write()");
            exit(EXIT_FAILURE);
        }
        printf("[%s] writed: %c\n", __FILE__, w_buf);

        // read from driver(TEST)
        // if((ret = read(fd, r_buf, sizeof(r_buf))) == -1) {
        //     perror("read()");
        //     exit(EXIT_FAILURE);
        // }
        // for (int i = 0; i < 16; ++i) {
        //     printf("%c ", r_buf[i]);
        // }
        // printf("\n");
        sleep(1);
    }
    close(fd);
}