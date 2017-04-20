#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <string.h>

#define GPIOXLEVEL_LOW  0
#define GPIOXLEVEL_HIGH 1

int main(void){
	unsigned long gpio_pin=150;
	int fd;

	fd = open("/dev/low_power", O_RDWR);
	if(fd < 0){
        printf("open lowpower fail!\n");
    }

	if(ioctl(fd, GPIOXLEVEL_LOW, gpio_pin)){
        printf("GPIOXLEVEL_LOW %d fail!\n", gpio_pin);
    }else{
        printf("GPIOXLEVEL_LOW %d  succesee\n", gpio_pin);
    }

	return 0;
}
