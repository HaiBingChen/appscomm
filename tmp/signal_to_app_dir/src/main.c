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
	unsigned long lowpower_signal_status = 0;

	//初始化设置低电压检测信号
    lowpower_fd = open("/dev/low_power", O_RDWR);
    if (lowpower_fd < 0){
        printf("can't open low power!\n");
    }else{
		printf("open low power success\n");
	}

	while (1){
        if(read(lowpower_fd, &lowpower_signal_status, sizeof(unsigned long)) < 0){
            printf("read lowpower_signal_status fail\n");
            close(lowpower_fd);
        }else{
            printf("lowpower_signal_status: %ld\n", lowpower_signal_status);
        }

        if(lowpower_signal_status == 1){
            //进入倒车
            printf("into lowpower!\n");
        }else{
            //退出倒车
            printf("outof lowpower!\n");
        }
 
        usleep(100000);
    }

    return (void *)0;
}

void *backcar_thread_fun(void * arg){  
    unsigned long gpio_pin = 36;
    unsigned  backcar_val;
	int backcar_fd;
	unsigned long backcar_signal_status = 0;

    //初始化设置倒车检测信号
    backcar_fd = open("/dev/backcardrv", O_RDWR);
    if(backcar_fd < 0){
        printf("open backcar driver fail!\n");
    }

	//获取倒车状态，非阻塞
	if(ioctl(backcar_fd, IOCTL_BC_DETECTGPIO, &backcar_val)){
        printf("IOCTL_BC_DETECTGPIO fail!\n");
    }else{
        printf("IOCTL_BC_DETECTGPIO succesee\n");
    }

	if(backcar_val == 1){
        //进入倒车
        printf("into backcar!\n");
    }else{
        //退出倒车
        printf("outof backcar!\n");
    }

    //如果系统起来后还没退出倒车，则依然是arm2在控制，这个ioctl会阻塞直到退出倒车
    //下次运行则由app控制倒车，arm2不再运行倒车
   
    if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_APP_READY, &backcar_val)){
        printf("IOCTL_FSC_NOTIFY_APP_READY fail!\n");
    }else{
        printf("IOCTL_FSC_NOTIFY_APP_READY succesee\n");
    }
/*
	if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_ARM1_READY)){
        printf("IOCTL_FSC_NOTIFY_ARM1_READY fail!\n");
    }else{
        printf("IOCTL_FSC_NOTIFY_ARM1_READY succesee\n");
    }
*/
    //释放视频资源,pcm资源等
    if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_ARM2_STOP, &backcar_val)){
        printf("IOCTL_FSC_NOTIFY_ARM2_STOP fail!\n");
    }else{
        printf("IOCTL_FSC_NOTIFY_ARM2_STOP succesee \n");
    }

	//清空倒车fb缓冲
	if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET, &backcar_val)){
		printf("IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET fail!\n");
	}else{
		printf("IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET succesee\n");
	}

    //重新初始化IO引脚，设置成外部中断模式，注意这里的参数gpio_pin，不要加取地址&，否则系统会奔溃
    if(ioctl(backcar_fd, IOCTL_BC_GPIOINIT, gpio_pin)){
        printf("IOCTL_BC_GPIOINIT fail!\n");
    }else{
        printf("IOCTL_BC_GPIOINIT succesee\n");
    }

/*
    while (1){ 
		//判断io口状态来判断是进入倒车还是退出倒车，如果倒车状态没改变，会阻塞，状态改变了会唤醒
		if(read(backcar_fd, &backcar_signal_status, sizeof(unsigned long)) < 0){
			printf("read backcar_signal_status fail\n");
			close(backcar_fd);
		}else{
			printf("backcar_signal_status: %ld\n", backcar_signal_status);
		}

		if(backcar_signal_status == 1){
			//进入倒车
			printf("into backcar!\n");
		}else{
			//退出倒车
			printf("outof backcar!\n");
		}
 
        usleep(100000);  
    }
*/	
	return (void *)0;  
} 

int main(int argc, char **argv){ 
	
    int err;  
	pthread_t backcar_tid;
	//pthread_t lowpower_tid;

    err = pthread_create(&backcar_tid, NULL, backcar_thread_fun, NULL);
	if(err != 0){  
        printf("%s\n", strerror(err));  
		return -1;
    }  

/*
	err = pthread_create(&lowpower_tid, NULL, lowpower_thread_fun, NULL);
    if(err != 0){   
        printf("%s\n", strerror(err));
        return -1;
    }
*/
	pthread_join(backcar_tid, NULL);

//	pthread_join(lowpower_tid, NULL);

	return 0;  
}  

