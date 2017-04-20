
#ifndef __LCDTEST_H__
#define __LCDTEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>
#include <linux/fb.h>
#include "pmap.h"

typedef unsigned int  uint32_t;

#include "tccfb_ioctrl.h"
#include "tcc_overlay_ioctl.h"


typedef struct tcc_overlay_handle tcc_overlay_handle_t;
struct tcc_overlay_handle {
	int fd;
	overlay_config_t	ovrl_config;
	pmap_t				pmap;
	unsigned int scnsize;
	void *pdata;

    unsigned int (*makepixel)(unsigned int a, unsigned int r, unsigned int g, unsigned int b);
    void (*fill_rect)(void *pdata, int xres,
                      unsigned int pixel,
                      int xpos1,
                      int ypos1,
                      int xpos2,
                      int ypos2);

};

extern tcc_overlay_handle_t *new_overlay_handle(char *dev_name);
extern tcc_overlay_handle_t *delete_overlay_handle(tcc_overlay_handle_t *h);

#ifdef __cplusplus
}
#endif

#endif /*__LCDTEST_H__*/

