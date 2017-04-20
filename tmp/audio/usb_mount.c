#include <stdio.h>

int main(void){
	FILE *fd;
	unsigned char write_buf[]={1,2,3,4,5,6,7,8,9,0};
	unsigned char read_buf[10];
	
	fd = fopen("/media/udisk1/usb_test.c", "w+");
	if(fd == NULL){
		printf("open /media/udisk1 fail\n");	
		return -1;	
	}else{
		printf("open /media/udisk1 success\n");
	} 

	while(1){
		//fwrite(write_buf, sizeof(write_buf), 1, fd);
		//fread(read_buf, sizeof(read_buf), 1, fd);

		usleep(100000);
	}
	
	return 0;
}
