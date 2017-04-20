#include <stdio.h>  
#include <errno.h>  
#include <stdlib.h>  
#include <string.h>  
#include <dirent.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <fcntl.h>
#include "uart.h"
#include "xmodem.h"
#include "my_ctype.h"
 
#define MAX_PATH 512  

int uart_fd;
int file_fd = 0;
unsigned int read_size = 0;
unsigned int write_size = 0;
unsigned char file_read_buf[128];

XMODEM_packet pkt;
unsigned long crc_total=0;
unsigned short cur_crc=0;
unsigned char pkt_num=1;

unsigned short crc_calc(unsigned char *start, unsigned char *end)
{
	unsigned short crc = 0x0;
	unsigned char  *data=start;

	for (data = start; data < end; data++){
		crc  = (crc >> 8) | (crc << 8);
		crc ^= *data;
		crc ^= (crc & 0xff) >> 4;
		crc ^= crc << 12;
		crc ^= (crc & 0xff) << 5;
	}
	return crc;
}

int send_file_to_mcu(char *pathname, char *filename)
{    
    unsigned long file_pos = 0;
    unsigned long remaind_len = 0;
    unsigned char stop_flag=0;
    unsigned char ack=0;
    struct stat statbuff;  
   
    dprintf(DEBUG_INFO, "pathname: %s\n",  pathname);  
    dprintf(DEBUG_INFO, "filename: %s\n",  filename);  
 
    char *file_suffix=strstr(filename,".");
    
    if(strcmp(file_suffix, ".bmp")){
	dprintf(DEBUG_ERR, "the file is not a bmp\n");
    	return -1;
    }

    file_fd = open(pathname, O_RDONLY);
    if(file_fd < 0){
       dprintf(DEBUG_ERR, "open file %s fail\n", pathname);
       return -1;
    }

    if(stat(pathname, &statbuff) < 0){  
        dprintf(DEBUG_ERR, "get file len fail \n");
        return -1;  
    }else{  
        remaind_len = statbuff.st_size;  
        dprintf(DEBUG_INFO, "file ramaind len is: %ld\n", remaind_len);
    }      

    //传输前先清空串口接收fifo
    tcflush(uart_fd, TCIOFLUSH);
    
    while(remaind_len>0 && !stop_flag){	
	lseek(file_fd, file_pos, SEEK_SET);

	if(remaind_len >= 128){
        	read_size = read(file_fd, file_read_buf, sizeof(file_read_buf));
	}else{
		dprintf(DEBUG_INFO, "remaind size less than 128\n");
		read_size = read(file_fd, file_read_buf, remaind_len);
	}

        dprintf(DEBUG_INFO, "read size is %d \n", read_size);

        pkt.header = XMODEM_SOH;
        pkt.packet_number = pkt_num;
        pkt.packet_numberC = ~pkt_num;
        strcpy(pkt.file_name, filename);
        pkt.data_len=read_size;
        memcpy((unsigned char *)pkt.data, (unsigned char *)file_read_buf, read_size);
    
        cur_crc = crc_calc((unsigned char *) pkt.data, (unsigned char *) &(pkt.crc_high));
        dprintf(DEBUG_INFO, "cur crc is: %d\n", cur_crc);
        dprintf(DEBUG_INFO, "crc high: %d, crc low: %d\n", (cur_crc >> 8) & 0xff, cur_crc & 0xff);
        pkt.crc_high= (cur_crc >> 8) & 0xff;
        pkt.crc_low= cur_crc & 0xff;

        write_size = write(uart_fd, (XMODEM_packet *)&pkt, sizeof(pkt));
        if(write_size <= 0){
            dprintf(DEBUG_ERR, "write ptk error\n");
            return -1;
        }

        dprintf(DEBUG_INFO, "write size is %d \n", write_size);

read_ack:

	//等待应答
	read(uart_fd, (unsigned char *)&ack, 1);
	dprintf(DEBUG_INFO, "receive %d\n", ack);

	switch(ack){
	    case XMODEM_ACK:
	    {
		dprintf(DEBUG_INFO, "rev mcu ack\n");
		
		file_pos += read_size;
		remaind_len -= read_size;

		dprintf(DEBUG_INFO, "file_pos is:%ld , remaind len is:%ld\n", file_pos, remaind_len);

        	pkt_num++;
        	if(pkt_num >= 0xff){
            	    pkt_num=1;
        	}
		dprintf(DEBUG_INFO, "pkt_num is:%d\n", pkt_num);

        	crc_total += cur_crc;

		break;
            }

	    case XMODEM_NAK:
            {
		dprintf(DEBUG_INFO, "need restart send pkt\n");

                break;
	    }	

	    case XMODEM_CAN:
	    { 
		dprintf(DEBUG_INFO, "stop send pkt\n");
		stop_flag = 1;

		break;
	    }
	   
	    default:
	      goto read_ack;
	}  
   }

   close(file_fd);    

   return write_size;
}

void dir_order(char *pathname)  
{  
    DIR *dfd;  
    char name[MAX_PATH];  
    struct dirent *dp;  
    struct stat filestat;

    if ((dfd = opendir(pathname)) == NULL){  
        dprintf(DEBUG_ERR, "dir_order: can't open %s\n %s", pathname,strerror(errno));  
        return;  
    }  

    while ((dp = readdir(dfd)) != NULL)  {  
        if (strncmp(dp->d_name, ".", 1) == 0)  
            continue; /* 跳过当前目录和上一层目录以及隐藏文件*/  

        if (strlen(pathname) + strlen(dp->d_name) + 2 > sizeof(name)){  
            dprintf(DEBUG_WARN, "dir_order: name %s/%s too long\n", pathname, dp->d_name);  
        } else{  
            memset(name, 0, sizeof(name));  
            sprintf(name, "%s/%s", pathname, dp->d_name);  
    
            if (stat(name, &filestat) == -1){  
                dprintf(DEBUG_ERR, "cannot access the file %s\n", pathname);  
                return;  
            }
  		
  	    //目录
            if ((filestat.st_mode & S_IFMT) == S_IFDIR){  
                dir_order(name);  
	    //文件
            }else{
		send_file_to_mcu(name, dp->d_name);
            }
	}
     }	  
    
    closedir(dfd);    
}  
 
int main(int argc, char *argv[])  
{
    if(argc != 2){
	dprintf(DEBUG_ERR, "usage: ./uart_to_muc dir_path\n");
	return -1;
    }

    uart_fd = uart_init("/dev/ttyUSB0");

    tcflush(uart_fd, TCIOFLUSH);

    dir_order(argv[1]);  
    
    //send_file_to_mcu("./123.bmp", "123.bmp");
    
    close(uart_fd);

    return 0;  
} 
