#ifndef __TRACK_COMM_H
#define __TRACK_COMM_H

#include "checksum_data.h"

unsigned short  ca_get_crc(char* ptr, unsigned short len);
unsigned short create_crc16(char *ptr, unsigned short len);

#endif