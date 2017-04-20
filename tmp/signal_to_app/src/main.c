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
	
	//�ж�io��״̬���ж��ǽ��뵹�������˳�����
	if(read(lowpower_fd, &lowpower_signal_status, sizeof(unsigned long)) < 0){
		printf("read lowpower_signal_status fail\n");
		close(lowpower_fd);
	}else{
		printf("lowpower_signal_status: %ld\n", lowpower_signal_status);
	}
	
	if(lowpower_signal_status == 1){
		//����͵�ѹ
		printf("into lowpower!\n");
    }else{
		//�˳��͵�ѹ
		printf("outof lowpower!\n");
	}
}

void backcar_signal_fun(int signum){
	unsigned long backcar_signal_status = 0;
	
	//�ж�io��״̬���ж��ǽ��뵹�������˳�����
	if(read(backcar_fd, &backcar_signal_status, sizeof(unsigned long)) < 0){
		printf("read backcar_signal_status fail\n");
		close(backcar_fd);
	}else{
		printf("backcar_signal_status: %ld\n", backcar_signal_status);
	}
	
	if(backcar_signal_status == 1){
		//���뵹��
		printf("into backcar!\n");
    }else{
		//�˳�����
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
	
	//��ʼ�����õ͵�ѹ����ź�
	lowpower_fd = open("/dev/low_power", O_RDWR);  
    if (lowpower_fd < 0){  
        printf("can't open low power!\n");  
    }  
 /* 
    //����ǰ����PID����Ϊfd�ļ�����Ӧ��������Ҫ����SIGIO,SIGUSR�źŽ���PID  
    fcntl(lowpower_fd, F_SETOWN, getpid());  
	
    //��ȡfd�Ĵ򿪷�ʽ  
    Oflags = fcntl(lowpower_fd, F_GETFL);     
    fcntl(lowpower_fd, F_SETFL, Oflags | FASYNC);  
	fcntl(lowpower_fd, F_SETSIG, LOWPOWER_SIG);
*/
	//����ǰ����PID����Ϊfd�ļ�����Ӧ��������Ҫ����SIGIO,SIGUSR�źŽ���PID  
    if(fcntl(lowpower_fd, F_SETOWN, getpid())){
        printf("F_SETOWN success\n");
    }else{
        printf("F_SETOWN fail\n");
    }

    //��ȡfd�Ĵ򿪷�ʽ  
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

	
	//��ʼ�����õ�������ź�
	backcar_fd = open("/dev/backcardrv", O_RDWR);
	if(backcar_fd < 0){
		printf("open backcar driver fail!\n");
	}

	//���ϵͳ������û�˳�����������Ȼ��arm2�ڿ��ƣ����ioctl������ֱ���˳�����
	//�´���������app���Ƶ�����arm2�������е���
	if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_APP_READY, &backcar_val)){
		printf("IOCTL_FSC_NOTIFY_APP_READY fail!\n");
	}else{
		printf("IOCTL_FSC_NOTIFY_APP_READY succesee\n");
	}
	
	//�ͷ���Ƶ��Դ,pcm��Դ��
	if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_ARM2_STOP, &backcar_val)){
		printf("IOCTL_FSC_NOTIFY_ARM2_STOP fail!\n");
	}else{
		printf("IOCTL_FSC_NOTIFY_ARM2_STOP succesee \n");
	}
	
	//��յ���fb����
	if(ioctl(backcar_fd, IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET, &backcar_val)){
		printf("IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET fail!\n");
	}else{
		printf("IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET succesee\n");
	}

	//���³�ʼ��IO���ţ����ó��ⲿ�ж�ģʽ��ע������Ĳ���gpio_pin����Ҫ��ȡ��ַ&������ϵͳ�ᱼ��
	if(ioctl(backcar_fd, IOCTL_BC_GPIOINIT, gpio_pin)){
		printf("IOCTL_BC_GPIOINIT fail!\n");
	}else{
		printf("IOCTL_BC_GPIOINIT succesee\n");
	}
	
	//����ǰ����PID����Ϊfd�ļ�����Ӧ��������Ҫ����SIGIO,SIGUSR�źŽ���PID  
    if(fcntl(backcar_fd, F_SETOWN, getpid())){
		printf("F_SETOWN success\n");
	}else{
		printf("F_SETOWN fail\n");
	}
	 
    //��ȡfd�Ĵ򿪷�ʽ  
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
		//app������ѯIO��״̬������read�ӿڶ�ȡIO��״̬
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

