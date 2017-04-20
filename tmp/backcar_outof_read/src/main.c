#include "main.h"
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <poll.h>  
#include <signal.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <fcntl.h> 
#include <string.h>
#include <pthread.h>   
#include <sys/syscall.h>  
#include <sys/ioctl.h>

void *lowpower_thread_fun(void * arg){
	int lowpower_fd;

	//初始化设置低电压检测信号
    lowpower_fd = open("/dev/low_power", O_RDWR);
    if (lowpower_fd < 0){
        printf("can't open low power!\n");
    }else{
		printf("open low power success\n");
	}

	if(ioctl(lowpower_fd, LOWPOWER_OUTOF_READ)){
        printf("LOWPOWER_OUTOF_READ fail!\n");
    }else{
        printf("LOWPOWER_OUTOF_READ succesee\n");
    }

    return (void *)0;
}

void *backcar_thread_fun(void * arg){  
	int backcar_fd;

    //初始化设置倒车检测信号
    backcar_fd = open("/dev/backcardrv", O_RDWR);
    if(backcar_fd < 0){
        printf("open backcar driver fail!\n");
    }

	if(ioctl(backcar_fd, IOCTL_FSC_OUTOF_READ)){
        printf("IOCTL_FSC_OUTOF_READ fail!\n");
    }else{
        printf("IOCTL_FSC_OUTOF_READ succesee\n");
    }


	return (void *)0;  
} 

int main(int argc, char **argv){ 
    int err;  
	pthread_t backcar_tid;
	pthread_t lowpower_tid;

    err = pthread_create(&backcar_tid, NULL, backcar_thread_fun, NULL);
	if(err != 0){  
        printf("%s\n", strerror(err));  
		return -1;
    }  

	err = pthread_create(&lowpower_tid, NULL, lowpower_thread_fun, NULL);
    if(err != 0){   
        printf("%s\n", strerror(err));
        return -1;
    }

	pthread_join(backcar_tid, NULL);

	pthread_join(lowpower_tid, NULL);

	return 0;  
}  

