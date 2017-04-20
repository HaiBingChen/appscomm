TARGET := uart_to_mcu

CC     := gcc
  
CFLAG  := -Wall    

INCLUDES   := -Iinclude/  

SRCS    = ./src/main.c ./src/uart.c

OBJS    = $(SRCS:.c=.o)    

all:$(OBJS)   
	$(CC) $(LDFLAG) -o $(TARGET) $^  
	
%.o:%.c   
	$(CC) -c $(INCLUDES) $(CFLAG) $(CPPFLAG) $< -o $@  
	
clean:     
	rm -rf $(basename $(TARGET)) $(SRCS:.c=.o)
