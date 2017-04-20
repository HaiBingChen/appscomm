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
//�͵�ѹ�źŴ�����  
void low_power_signal_fun(int signum){  

	unsigned long lowpower_signal_status = 0;
	
	//�ж�io��״̬���ж��ǽ��뵹�������˳�����
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

    //��Ӧ�ó����в�׽SIGIO�źţ������������ͣ� 
	memset(&action, 0, sizeof(action)); 
	action.sa_handler = low_power_signal_fun;  
    action.sa_flags = 0; 	
    sigaction(SIGIO, &action, NULL);    
  

    //����ǰ����PID����Ϊfd�ļ�����Ӧ��������Ҫ����SIGIO,SIGUSR�źŽ���PID  
    fcntl(lowpower_fd, F_SETOWN, getpid());  
      
    //��ȡfd�Ĵ򿪷�ʽ  
    Oflags = fcntl(lowpower_fd, F_GETFL);   
  
    //��fd�Ĵ򿪷�ʽ����ΪFASYNC --- �� ֧���첽֪ͨ  
    //���д���ִ�лᴥ�� ���������� file_operations->fasync ���� ------
	//fasync��������fasync_helper��ʼ��һ��fasync_struct�ṹ�壬�ýṹ�������˽�Ҫ�����źŵĽ���PID
	//(fasync_struct->fa_file->f_owner->pid)  
    fcntl(lowpower_fd, F_SETFL, Oflags | FASYNC);  
	
	printf("wait for low power signal...\n");

	//�����ý���һֱ�ܲ����˳�
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

