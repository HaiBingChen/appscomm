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

int lowpower_fd;
//低电压信号处理函数  
void low_power_signal_fun(int signum){  

	unsigned long lowpower_signal_status = 0;
	
	//判断io口状态来判断是进入倒车还是退出倒车
	if(read(lowpower_fd, &lowpower_signal_status, sizeof(unsigned long)) < 0){
		printf("read lowpower_signal_status fail\n");
		close(lowpower_fd);
	}else{
		printf("lowpower_signal_status: %ld\n", lowpower_signal_status);
	}
	
	if(lowpower_signal_status == 1){
		printf("into lowpower!\n");
    }else{
		printf("outof lowpower!\n");
	}
}  
 
int main(int argc, char **argv){  

	struct sigaction action;  
	unsigned long lowpower_read_status = 0;
    int Oflags;  

    //在应用程序中捕捉SIGIO信号（由驱动程序发送） 
	memset(&action, 0, sizeof(action)); 
	action.sa_handler = low_power_signal_fun;  
    action.sa_flags = 0; 	
    sigaction(SIGIO, &action, NULL);    
  

    //将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
    fcntl(lowpower_fd, F_SETOWN, getpid());  
      
    //获取fd的打开方式  
    Oflags = fcntl(lowpower_fd, F_GETFL);   
  
    //将fd的打开方式设置为FASYNC --- 即 支持异步通知  
    //该行代码执行会触发 驱动程序中 file_operations->fasync 函数 ------
	//fasync函数调用fasync_helper初始化一个fasync_struct结构体，该结构体描述了将要发送信号的进程PID
	//(fasync_struct->fa_file->f_owner->pid)  
    fcntl(lowpower_fd, F_SETFL, Oflags | FASYNC);  
	
	printf("wait for low power signal...\n");

	//必须让进程一直跑不能退出
    while(1){
		if(read(lowpower_fd, &lowpower_read_status, sizeof(unsigned long)) < 0){
			printf("read lowpower_read_status fail\n");
			close(lowpower_fd);
			return 1;
		}else{
			printf("lowpower_read_status: %ld\n", lowpower_read_status);
		}

		usleep(100000);
	}
	
    return 0;  
}  

