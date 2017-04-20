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

	//��ʼ�����õ͵�ѹ����ź�
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
            //���뵹��
            printf("into lowpower!\n");
        }else{
            //�˳�����
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

    //��ʼ�����õ�������ź�
    backcar_fd = open("/dev/backcardrv", O_RDWR);
    if(backcar_fd < 0){
        printf("open backcar driver fail!\n");
    }

	//��ȡ����״̬��������
	if(ioctl(backcar_fd, IOCTL_BC_DETECTGPIO, &backcar_val)){
        printf("IOCTL_BC_DETECTGPIO fail!\n");
    }else{
        printf("IOCTL_BC_DETECTGPIO succesee\n");
    }

	if(backcar_val == 1){
        //���뵹��
        printf("into backcar!\n");
    }else{
        //�˳�����
        printf("outof backcar!\n");
    }

    //���ϵͳ������û�˳�����������Ȼ��arm2�ڿ��ƣ����ioctl������ֱ���˳�����
    //�´���������app���Ƶ�����arm2�������е���
   
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

/*
    while (1){ 
		//�ж�io��״̬���ж��ǽ��뵹�������˳��������������״̬û�ı䣬��������״̬�ı��˻ỽ��
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

