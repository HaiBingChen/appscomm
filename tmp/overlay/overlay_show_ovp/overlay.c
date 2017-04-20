#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "overlay.h"
#include "pmap.h"
#include "vioc_global.h"

#define INC_UNIT 10
#define CONFIG_TCC_INVITE
#include "tcc_overlay_ioctl.h"

static inline unsigned int makepixel_32(unsigned int a, unsigned int r, unsigned int g, unsigned int b)
{
	return (unsigned int)((a<<24)|(r<<16)|(g<<8)|b);
}

static void fill_rect_32(void *pdata, int xres, unsigned int pixel, int xpos1, int ypos1, int xpos2, int ypos2)
{
    unsigned int *pfbdata = NULL;
	int offset;
	int t, tt;

    if (pdata) {
        pfbdata = (unsigned int *)pdata;

        if (xpos1 > xpos2) {
            t = xpos1;
            xpos1 = xpos2;
            xpos2 = t;
        }

        if (ypos1 > ypos2) {
            t = ypos1;
            ypos1 = ypos2;
            ypos2 = t;
        }

        for (t = ypos1; t <= ypos2; t++) {
            offset = t * xres;

            for (tt = xpos1; tt <= xpos2; tt++) {
                *(pfbdata + offset + tt) = pixel;
            }
        }
    }
}

#define OVERLAYDEVFILE	"/dev/overlay"

tcc_overlay_handle_t *new_overlay_handle(char *dev_name)
{
    int success = -1;
    tcc_overlay_handle_t *h = NULL;

    if (dev_name) {
        h = malloc(sizeof(tcc_overlay_handle_t));
        if (h) {
            memset(h, 0, sizeof(tcc_overlay_handle_t));
            h->fd = -1;
            h->pdata = MAP_FAILED;

            h->fd = open(dev_name, O_RDWR);
            if (h->fd != -1) {
				h->pmap.size = 0;
				pmap_get_info("overlay", &h->pmap);

				if (h->pmap.size)
				{
	                h->pdata = mmap(0,
	                                  h->pmap.size,
	                                  PROT_READ|PROT_WRITE,
	                                  MAP_SHARED,
	                                  h->fd,
	                                  h->pmap.base);
	                if (h->pdata != MAP_FAILED) {
                        h->makepixel = makepixel_32;
                        h->fill_rect = fill_rect_32;
	                    success = 0;
	                } else 
	                    perror("[lcdtest] overlay mmap error !!!\n");
				}
            } else 
                perror("[lcdtest] Cannot open device !!!\n");
        } else 
            printf("[lcdtest] %s:%d Cannot allocate memory !!!\n", __func__, __LINE__);
    } else 
        printf("[lcdtest] %s:%d dev_name is NULL !!!\n", __func__, __LINE__);

    if (success != 0) {
        h = delete_overlay_handle(h);
    }
    return h;
}

tcc_overlay_handle_t *delete_overlay_handle(tcc_overlay_handle_t *h)
{
    if (h) {
        if (h->pdata != MAP_FAILED) {
			munmap(h->pdata, h->pmap.size);
        }
		close(h->fd);
        free(h);
    }
    return NULL;
}


int main(void)
{
	mgcc_overlay_handle_t *h_overlay = NULL;
	tcc_overlay_handle_t *hovr = NULL;

	overlay_video_buffer_t info;
	int xres, yres;
	void *pdata;
	unsigned int base[3] = {0, 0, 0};

	h_overlay = new_overlay_handle(OVERLAYDEVFILE);
    if (!h_overlay) {
        printf("Cannot allocate tcc_overlay_handle_t (#0) \n");
        goto _end;
    }
/////设置overlay显示位置，大小和格式，如果如要做透明，设置为ARGB8888
	h_overlay->ovrl_config.sx = 0;
	h_overlay->ovrl_config.sy = 0;
	h_overlay->ovrl_config.width = 800;//fb->fbvar.xres;
	h_overlay->ovrl_config.height = 480;//fb->fbvar.yres;
	h_overlay->ovrl_config.format = VIOC_IMG_FMT_ARGB8888;

	ioctl(h_overlay->fd, OVERLAY_SET_CONFIGURE,&h_overlay->ovrl_config);

///////设置层次 
	#define OVERLAY_SET_WMIXER_OVP        80
	unsigned int ovp;

	ioctl(h_overlay->fd, OVERLAY_GET_WMIXER_OVP, &ovp);
	printf(" overlay get current ovp %d\n", ovp);

	//ovp = 16 ;   //ovp 1-0-3 2 is not use
	//ioctl(h_overlay->fd, OVERLAY_SET_WMIXER_OVP, &ovp);
	//printf(" overlay set current ovp %d\n", ovp);
//////////////////////////////////////////////////////////
	hovr = h_overlay;
	pdata = hovr->pdata;
	xres  = hovr->ovrl_config.width;
	yres  = hovr->ovrl_config.height;
	
	int size= xres*yres*4;
	memset(pdata, 0x0, size);

	base[0] = h_overlay->pmap.base;
	info.addr = base[0];
	info.cfg = h_overlay->ovrl_config;
	ioctl(h_overlay->fd, OVERLAY_PUSH_VIDEO_BUFFER,&info);

//////设置完后，只需要往pdata内存里写东西就可以显示在屏幕上了
#if 1
int count=5;
while(count--)
{
  	printf("count = %d...\n",count);
  	hovr->fill_rect(pdata, xres, hovr->makepixel(255, 255, 0, 0),
                 0, 0,
                 xres - 1, yres -1);
	usleep(1000*1000);		
  	hovr->fill_rect(pdata, xres, hovr->makepixel(127, 0, 255, 0),
                 0, 0,
                 xres -1 , yres /2);
	usleep(1000*1000);
  	hovr->fill_rect(pdata, xres, hovr->makepixel(127, 0, 0, 255),
                 0, yres /2,
                 xres - 1, yres -1);
	usleep(1000*1000);	
	int i=0;
	for(i=0;i<256;i++)
	{
	  	hovr->fill_rect(pdata, xres, hovr->makepixel(i, 255, 255, 0),
                 0, 0,
                 xres - 1, yres -1);
		usleep(1000);	
	}
	for(i=255;i>=0;i--)
	{
	  	hovr->fill_rect(pdata, xres, hovr->makepixel(i, 255, 255, 0),
                 0, 0,
                 xres - 1, yres -1);
		usleep(1000);	
	}
}

#endif

_end:
	h_overlay = delete_overlay_handle(h_overlay);
	return 0;

}


