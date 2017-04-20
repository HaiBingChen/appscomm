#ifndef __XMODEM_H
#define __XMODEM_H

//开始
#define XMODEM_SOH                1
//发送完成
#define XMODEM_EOT                4
//成功
#define XMODEM_ACK                6
//失败，要求重发
#define XMODEM_NAK                21
//无条件终止传输
#define XMODEM_CAN                24
//发送表示等待接收
#define XMODEM_NCG                67

#define XMODEM_DATA_SIZE          128
#define XMODEM_FILENAME_SIZE      50

typedef struct{
  unsigned char header;       //头 SOH
  unsigned char packet_number; //信息包序号，从 01 开始以发送一包将加1，加到FF hex 将循环。
  unsigned char packet_numberC;//信息包序号的补码。
  char file_name[XMODEM_FILENAME_SIZE];
  unsigned char data_len;
  unsigned char data[XMODEM_DATA_SIZE];//128数据
  unsigned char crc_high;//CRC16 高字节
  unsigned char crc_low; //CRC16 低字节

} XMODEM_packet;

#endif
