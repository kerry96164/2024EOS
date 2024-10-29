#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main() {
    int incr = -80;
    int ret;
    ret = nice(incr);
    if(ret == -1){
        printf("%d\n",ret);
        printf("Error : %s\n", strerror(errno));
    }else{
        printf("%d\n",ret);
    }
    return 0;
}