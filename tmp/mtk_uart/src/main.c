#include "uart.h"
#include "track_comm.h"

#include <fcntl.h>
#include <unistd.h>
#include <termio.h>
#include <string.h>

#define ADRESS_SENDER                     0x55
#define ADRESS_RECEIVER                   0xA0
#define GID_MCU_TX_CAN_INFO               0x15
#define SID_MCU_TX_RADAR_SYS_INFO         0x31

#define UART_DEBUG	 0
#define dprintf(msg...)      if (UART_DEBUG) { printf("[UART_DEBUG]" msg); }

int main()
{
    int fd;
	//char write_buf[20] = {0xA0, 0x55, 0x00, 0xB1, 0x09, 0xE7, 0x17, 0x01, 0x02};
	char write_buf[10] = {0xA0, 0x55, 0x05, 0xB6, 0x0A, 0x48, 0xE8, 0x01, 0xFF, 0xFF};
	char rec_byte[20];
	unsigned short crc_from_mcu;
    unsigned short crc_check;
	unsigned short i;
	
    if((fd = uart_init(0)) <0)   //打开串口，波特率为115200；
    {
        printf("Open uart err \n");
        return -1;
    }
	printf("mtk usart open success\n");
    
	write(fd, write_buf, sizeof(write_buf));
#if 0	
	while(1){
		
		read(fd, rec_byte, 1);	
		if(rec_byte[0] == ADRESS_SENDER){
			read(fd, rec_byte+1, 1);
			
			if(rec_byte[1] == ADRESS_RECEIVER){
				dprintf("Read a mcu start head success\n");					
				dprintf("byte 0: 0x%x\n", rec_byte[0]);                    
				dprintf("byte 1: 0x%x\n\n", rec_byte[1]);
			}else{
				printf("Not a mcu start head\n");
				printf("byte 0: 0x%x\n", rec_byte[0]);
				printf("byte 1: 0x%x\n\n", rec_byte[1]);
				memset(rec_byte, 0x00, sizeof(rec_byte));				
				continue;	
			}
		}else{
			memset(rec_byte, 0x00, sizeof(rec_byte));
			continue;
		}

		//read radar package length				
		read(fd, rec_byte+2, 3);				
		dprintf("Command serial no\n")				
		dprintf("byte 2: 0x%x\n\n", rec_byte[2]);
		dprintf("Package length\n");
		dprintf("byte 4: 0x%x\n\n", rec_byte[4]);

		//read the leave package data
		read(fd, rec_byte+5, rec_byte[4]-5);

		//check crc               
		crc_from_mcu = rec_byte[5] & 0xFF;
		crc_from_mcu |= (rec_byte[6]<<8) & 0xFF00;
		crc_check =  create_crc16(rec_byte, rec_byte[4]);
		if(crc_check != crc_from_mcu){		
			printf("Check crc error mcu 0x%x apu:0x%x\n", crc_from_mcu, crc_check);
			printf("byte 5: 0x%x\n", rec_byte[5]);		
			printf("byte 6: 0x%x\n\n",  rec_byte[6]);  
			memset(rec_byte, 0x00, sizeof(rec_byte));
			continue;            
		}else{		
			dprintf("Check crc success mcu: 0x%x apu:0x%x\n", crc_from_mcu, crc_check);
			dprintf("byte 5: 0x%x\n", rec_byte[5]);      
			dprintf("byte 6: 0x%x\n\n", rec_byte[6]);   
		}                

		//check if it is a radar package            
		if(!(rec_byte[3] == GID_MCU_TX_CAN_INFO && rec_byte[7] == SID_MCU_TX_RADAR_SYS_INFO)){    
			printf("Not a radar package\n");			
			printf("byte 3: 0x%x\n", rec_byte[3]);		
			printf("byte 7: 0x%x\n\n", rec_byte[7]);   
		}else{				
			dprintf("Read a radar package success\n");		
			dprintf("byte 3: 0x%x\n", rec_byte[3]);              
			dprintf("byte 7: 0x%x\n\n", rec_byte[7]);        
		}

		printf("\nAPP UART: ");
		for(i=0; i<rec_byte[4]; i++)
			printf("0x%x ", rec_byte[i]);
		}	
#endif
    return 0;
}


