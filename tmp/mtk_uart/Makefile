#目标（要生成的文件名）  
TARGET := mtk_uart  

#编译器的选择（在Linux中其实可以忽略，因为cc指向的本来就是gcc）     
CC     := /opt/arm-2013.11/bin/arm-none-linux-gnueabi-gcc  
  
#编译的参数  
CFLAG  := -Wall    

#编译包含的头文件所在目录   
INCLUDES   := -Iinclude/   

#所有用到的源文件，注意：非当前目录的要+上详细地址  
SRCS    = ./src/main.c ./src/track_comm.c ./src/uart.c

#把源文件SRCS字符串的后缀.c改为.o   
OBJS    = $(SRCS:.c=.o)    

#匹配所有的伪目标依赖，即执行目标main.o & track_comm.o & track_comm.o
#当所有依赖目标都存在后，链接，即链接 main.o & track_comm.o & track_comm.o 
all:$(OBJS)   
	$(CC) $(LDFLAG) -o $(TARGET) $^  
	
#重定义隐藏规则，匹配上述目标：main.o & track_comm.o & track_comm.o  
#生成.o文件，注意，由于SRCS有个别包含详细地址的，生成的.o文件也是详细地址   
%.o:%.c   
	$(CC) -c $(INCLUDES) $(CFLAG) $(CPPFLAG) $< -o $@  
	
#清空除源文件外的所有生成文件   
clean:     
	rm -rf $(basename $(TARGET)) $(SRCS:.c=.o)