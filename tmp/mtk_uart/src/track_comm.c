#include "track_comm.h"

unsigned short  ca_get_crc(char* ptr, unsigned short len)
{
          unsigned short  crc = 0;
          unsigned short  i;

          for (i = 0; i < len; i++)
                crc = (crc << 8) ^ CRC_DATA[((unsigned char)(crc >> 8) ^ ptr[i]) & 0xFF];

        return crc;
}

unsigned short create_crc16(char *ptr, unsigned short len)
{
        unsigned short crc = 0;

        ptr[5] = 0x00;
        ptr[6] = 0x00;

        crc = ca_get_crc(ptr+2, len-2);

        ptr[5] = crc & 0xff;
        ptr[6] = crc>>8 & 0xff;

        return crc;
}
