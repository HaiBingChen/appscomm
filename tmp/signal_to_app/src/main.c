#i

:


n







clude "main.h"
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

int lowpower_fd;
int backcar_fd;

void lowpower_signal_fun(int signum){
	
	unsigned long lowpower_signal_status = 0;
	
	//判断io口状态来判断是进入倒车还是退出倒车
	if(read(lowpower_fd, &lowpower_signal_status, sizeof(unsigned long)) < 0){
		printf("read lowpower_signal_status fail\n");
		close(lowpower_fd);
	}else{
		printf("lowpower_signal_status: %ld\n", lowpower_signal_status);
	}
	
	if(lowpower_signal_status == 1){
		//进入低电压
		printf("into lowpower!\n");
    }else{
		//退出低电压
		printf("outof lowpower!\n");
	}
}

void backcar_signal_fun(int signum){
	unsigned long backcar_signal_status = 0;
	
	//判断io口状态来判断是进入倒车还是退出倒车
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
}

void lowpower_init(void){
	
	int Oflags;
	struct sigaction action; 
	
	memset(&action, 0, sizeof(action)); 
	action.sa_handler = lowpower_signal_fun;  
    action.sa_flags = 0; 	
    sigaction(LOWPOWER_SIG, &action, NULL);   
	
	//初始化设置低电压检测信号
	lowpower_fd = open("/dev/low_power", O_RDWR);  
    if (lowpower_fd < 0){  
        printf("can't open low power!\n");  
    }  
 /* 
    //将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
    fcntl(lowpower_fd, F_SETOWN, getpid());  
	
    //获取fd的打开方式  
    Oflags = fcntl(lowpower_fd, F_GETFL);     
    fcntl(lowpower_fd, F_SETFL, Oflags | FASYNC);  
	fcntl(lowpower_fd, F_SETSIG, LOWPOWER_SIG);
*/
	//将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
    if(fcntl(lowpower_fd, F_SETOWN, getpid())){
        printf("F_SETOWN success\n");
    }else{
        printf("F_SETOWN fail\n");
    }

    //获取fd的打开方式  
    Oflags = fcntl(lowpower_fd, F_GETFL);
    if(Oflags){
        printf("F_GETFL success\n");
    }else{
        printf("F_GETFL fail\n");
    }

    if(fcntl(lowpower_fd, F_SETFL, Oflags | FASYNC)){
        printf("F_SETFL success\n");
    }else{
        printf("F_SETFL fail\n");
    }

    if(fcntl(lowpower_fd, F_SETSIG, LOWPOWER_SIG)){
        printf("F_SETSIG success\n");
    }else{
        printf("F_SETSIG fail\n");
    }

}

void backcar_init(void){
	
	int Oflags;
	struct sigaction action; 
	unsigned long gpio_pin = 36;
	unsigned  backcar_val;

	memset(&action, 0, sizeof(action)); 
	action.sa_handler = backcar_signal_fun;  
    action.sa_flags = 0; 	
    sigaction(BACKCAR_SIG, &action, NULL); 

	
	//初始化设置倒车检测信号
	backcar_fd = open("/dev/backcardrv", O_RDWR);
	if(backcar_fd < 0){
		printf("open backcar driver fail!\n");
	}

	//如果系统起来后还没退出倒车，则依然是arm2在控制，这个ioctl会阻塞直到退出倒车
	//下次运行则由app控制倒车，arm2不再运行倒车
	if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_APP_READY, &backcar_val)){
		printf("IOCTL_FSC_NOTIFY_APP_READY fail!\n");
	}else{
		printf("IOCTL_FSC_NOTIFY_APP_READY succesee\n");
	}
	
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
	
	//将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
    if(fcntl(backcar_fd, F_SETOWN, getpid())){
		printf("F_SETOWN success\n");
	}else{
		printf("F_SETOWN fail\n");
	}
	 
    //获取fd的打开方式  
    Oflags = fcntl(backcar_fd, F_GETFL);   
	if(Oflags){
		printf("F_GETFL success\n");
	}else{
		printf("F_GETFL fail\n");
	}

    if(fcntl(backcar_fd, F_SETFL, Oflags | FASYNC)){
		printf("F_SETFL success\n");
	}else{
		printf("F_SETFL fail\n");
	}

	if(fcntl(backcar_fd, F_SETSIG, BACKCAR_SIG)){
		printf("F_SETSIG success\n");
	}else{
		printf("F_SETSIG fail\n");
	}	
}

int main(int argc, char **argv){ 
	
	unsigned long backcar_read_status = 0;
	unsigned long lowpower_read_status = 0;
	
	lowpower_init();
	
	//backcar_init();
	
	while(1){
	/*	
		//app主动查询IO口状态，调用read接口读取IO口状态
		if(read(backcar_fd, &backcar_read_status, sizeof(unsigned long)) < 0){
			printf("read backcar_read_status fail\n");
			close(backcar_fd);
			break;
		}else{
			//printf("backcar_read_status: %ld\n", backcar_read_status);
		}
		
		if(read(lowpower_fd, &lowpower_read_status, sizeof(unsigned long)) < 0){
			printf("read lowpower_read_status fail\n");
			close(lowpower_fd);
			break;
		}else{
			//printf("lowpower_read_status: %ld\n", lowpower_read_status);
		}
	*/	
		usleep(100000);
	}
	
    return 0;  
}  

