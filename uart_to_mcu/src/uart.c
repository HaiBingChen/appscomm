#include "uart.h"

int uart_init(char *uart_path)
{
    int fd;
    struct termios Opt;
 
    fd = open(uart_path, O_RDWR);     //打开串口
    if (fd<0)
    {
        printf("open %s fial\n", uart_path);
        return -1;                        //没有打开返回
    }
 
    tcgetattr(fd, &Opt);                   //初始化
    tcflush(fd, TCIFLUSH);
    cfsetispeed(&Opt, B115200);     //设置波特率
    cfsetospeed(&Opt, B115200);
 
    Opt.c_cflag |= CS8;                   //设置数据位8位
    Opt.c_cflag &= ~PARENB;				  //无奇偶校验
    Opt.c_cflag &= ~CSTOPB;				  //一位停止位
    Opt.c_oflag &= ~(OPOST);			  //不执行输出处理
	Opt.c_lflag &= ~(ICANON|ISIG|ECHO|IEXTEN);   
    Opt.c_iflag &= ~(INPCK|BRKINT|ICRNL|ISTRIP|IXON);
 
    Opt.c_cc[VMIN] = 64;            //最大长度
    Opt.c_cc[VTIME] = 1;            //超时时间
 
    if (tcsetattr(fd,TCSANOW,&Opt) != 0)       //装载初始化参数
    {
        perror("SetupSerial!\n");
        close(fd);
        return -1;
    }
    return(fd);
}
 
