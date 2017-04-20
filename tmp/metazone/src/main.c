#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <fcntl.h> 
#include <sys/syscall.h>  
#include <sys/ioctl.h>
#include "metazone.h"

#define METAZONE_ADDR 0x10060
#define RADAR_VOL      0x500

int main(int argc, char **argv){ 
	unsigned int readval = 0;

	MetaZone_Init();

	MetaZone_Write(METAZONE_ADDR, RADAR_VOL);
    MetaZone_Flush(1);	

    MetaZone_Read(METAZONE_ADDR, &readval);

    printf("the meatzone addr 0x%x, value is 0x%x\n", METAZONE_ADDR, readval);

    MetaZone_Deinit();

	return 0;  
}  

