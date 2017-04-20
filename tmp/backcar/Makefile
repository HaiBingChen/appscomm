TARGET := backcar  
   
CC     := /opt/arm-2013.11/bin/arm-none-linux-gnueabi-gcc  
   
CFLAG  := -Wall    

INCLUDES   := -Iinclude/   
 
SRCS    = ./src/main.c
  
OBJS    = $(SRCS:.c=.o)    

all:$(OBJS)   
	$(CC) $(LDFLAG) -o $(TARGET) $^  
	 
%.o:%.c   
	$(CC) -c $(INCLUDES) $(CFLAG) $(CPPFLAG) $< -o $@  
	   
clean:     
	rm -rf $(basename $(TARGET)) $(SRCS:.c=.o)