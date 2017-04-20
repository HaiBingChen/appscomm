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

//���뵹���źŴ�����  
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
	
	//============================================================================//
	
	//��Ӧ�ó����в�׽SIGIO�źţ������������ͣ� 
	//���뵹���¼�
	memset(&backcar_action, 0, sizeof(backcar_action)); 
	backcar_action.sa_handler = backcar_signal_fun;  
    backcar_action.sa_flags = 0; 	
    sigaction(SIG_MYINT, &backcar_action, NULL);   

	//����ǰ����PID����Ϊfd�ļ�����Ӧ��������Ҫ����SIGIO,SIGUSR�źŽ���PID  
    fcntl(backcar_fd, F_SETOWN, getpid());  
      
    //��ȡfd�Ĵ򿪷�ʽ  
    Oflags = fcntl(backcar_fd, F_GETFL);   
  
    //��fd�Ĵ򿪷�ʽ����ΪFASYNC --- �� ֧���첽֪ͨ  
    //���д���ִ�лᴥ�� ���������� file_operations->fasync ���� ------
	//fasync��������fasync_helper��ʼ��һ��fasync_struct�ṹ�壬�ýṹ�������˽�Ҫ�����źŵĽ���PID
	//(fasync_struct->fa_file->f_owner->pid)  
    fcntl(backcar_fd, F_SETFL, Oflags | FASYNC);  
	fcntl(backcar_fd, F_SETSIG, SIG_MYINT);
	
	//�����ý���һֱ�ܲ����˳�
	while(1){
		
		//app������ѯIO��״̬
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
