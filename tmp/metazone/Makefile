TARGET := metazone_radar
   
CC     := /opt/arm-2013.11/bin/arm-none-linux-gnueabi-gcc  
   
CFLAG  := -Wall    

INCLUDES   := -I./include/   

LIBS	= -L./lib -lpthread -lmetazone
 
SRCS    = ./src/main.c
  
OBJS    = $(SRCS:.c=.o)    

#这一步其实是gcc编译的最后一步，连接生产目标文件
all:$(OBJS)   
	$(CC) $(LDFLAG) $(LIBS) -o $(TARGET) $^  

#这一步其实是gcc编译的前三步，预处理生产.i，编译生产.s，汇编生产.o	 
%.o:%.c   
	$(CC) -c $(INCLUDES)  $(LIBS) $(CFLAG) $(CPPFLAG) $< -o $@  
	   
clean:     
	rm -rf $(basename $(TARGET)) $(SRCS:.c=.o)
