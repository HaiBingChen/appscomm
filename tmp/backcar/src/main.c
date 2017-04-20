#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <backcar.h>
#include <poll.h>  
#include <signal.h>  
#include <string.h>

#define SIG_MYINT 40

int backcar_fd;

//进入倒车信号处理函数  
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
		printf("into backcar!\n");
    }else{
		printf("outof backcar!\n");
	}
}

int main(void){

	unsigned  backcar_val;
	unsigned long backcar_read_status = 0;
	unsigned long gpio_pin = 36;
	struct sigaction backcar_action; 
	int Oflags; 
	 
	printf("satrt backcar\n");

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
	
	//============================================================================//
	
	//在应用程序中捕捉SIGIO信号（由驱动程序发送） 
	//进入倒车事件
	memset(&backcar_action, 0, sizeof(backcar_action)); 
	backcar_action.sa_handler = backcar_signal_fun;  
    backcar_action.sa_flags = 0; 	
    sigaction(SIG_MYINT, &backcar_action, NULL);   

	//将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
    fcntl(backcar_fd, F_SETOWN, getpid());  
      
    //获取fd的打开方式  
    Oflags = fcntl(backcar_fd, F_GETFL);   
  
    //将fd的打开方式设置为FASYNC --- 即 支持异步通知  
    //该行代码执行会触发 驱动程序中 file_operations->fasync 函数 ------
	//fasync函数调用fasync_helper初始化一个fasync_struct结构体，该结构体描述了将要发送信号的进程PID
	//(fasync_struct->fa_file->f_owner->pid)  
    fcntl(backcar_fd, F_SETFL, Oflags | FASYNC);  
	fcntl(backcar_fd, F_SETSIG, SIG_MYINT);
	
	//必须让进程一直跑不能退出
	while(1){
		
		//app主动查询IO口状态
		if(read(backcar_fd, &backcar_read_status, sizeof(unsigned long)) < 0){
			printf("read backcar_read_status fail\n");
			close(backcar_fd);
			return 1;
		}else{
			printf("backcar_read_status: %ld\n", backcar_read_status);
		}
		
		usleep(100000);
	}
	
	close(backcar_fd);
	
	return 0;
}
