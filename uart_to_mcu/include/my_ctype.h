#ifndef __MY_CTYPE_H
#define __MY_CTYPE_H

#define DEBUG_ON 1
#define DEBUG_NAME "[UART_DEBUG] "

//打印等级开关,大于这个等级的都会打印出来
#define DEBUG_LEVEL 0x03

#define DEBUG_DATA  0x01
#define DEBUG_CMD   0x02
#define DEBUG_INFO  0x03
#define DEBUG_WARN  0x04
#define DEBUG_ERR   0x05

#if DEBUG_ON
#define dprintf(level, msg...) do{\
			           if(level >= DEBUG_LEVEL){\
				       printf(DEBUG_NAME msg);\
			           }\
				}while(0);
#else
#define dprintf(level, msg...) 
#endif
				       
#endif
