/*
 * drivers/video/Tcc_overlay.c
 *
 * Copyright (C) 2004 Telechips, Inc. 
 *
 * Video-for-Linux (Version 2) graphic capture driver
 *
 * 
 * This package is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 * 
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED 
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/videodev2.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <linux/poll.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#endif

#include <soc/tcc/pmap.h>
#ifdef CONFIG_ARCH_TCC897X
#include <mach/bsp.h>

#include <mach/tccfb.h>
#include <mach/tcc_fb.h>
#include <mach/tcc_overlay_ioctl.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tca_display_config.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_global.h>
#else
#include <video/tcc/tccfb.h>
#include <video/tcc/tcc_fb.h>
#include <video/tcc/tcc_overlay_ioctl.h>
#include <video/tcc/tccfb_ioctrl.h>
#include <video/tcc/tca_display_config.h>

#include <video/tcc/vioc_outcfg.h>
#include <video/tcc/vioc_rdma.h>
#include <video/tcc/vioc_wdma.h>
#include <video/tcc/vioc_wmix.h>
#include <video/tcc/vioc_disp.h>
#include <video/tcc/vioc_global.h>
#endif
#include <mach/vioc_wmix.h>

#if 0
static int debug	   = 1;
#else
static int debug	   = 0;
#endif


#define dprintk(msg...)	if (debug) { printk( "tcc_overlay: " msg); }

#define DEVICE_NAME			"overlay"
#define DEV_MINOR	202

#define	VIDEO_CH_NUM		1

//#define OVERLAY_CNT_DEFAULT 2

static struct clk *overlay_lcdc_clk;

static overlay_config_t overlay_cfg;
static struct v4l2_format overlay_fmt;

struct overlay_struct_info_t{
	VIOC_RDMA *pRDMABase;
	unsigned int rdma_index;
	VIOC_WMIX *pWMIXBase;
	VIOC_DISP *pDISPBase;
	VIOC_IREQ_CONFIG *pIREQConfig;
};

static struct overlay_struct_info_t overlay_info;

static unsigned char start_en = 0;
static unsigned char wait_restart = 0;
static unsigned char pos_reset = 0;

static unsigned char overlay_en_count = 0;
unsigned char tcc_overlay_use = 0;

//#define VC_OVERLAY_PROFILE // CAM_28pin, GPIO_D24
#ifdef VC_OVERLAY_PROFILE
static unsigned char toggle_int = 0;
#endif

extern struct display_platform_data tcc_display_data;
//extern OUTPUT_SELECT_MODE	Output_Select;
OUTPUT_SELECT_MODE	Output_Select;
unsigned int lcd_lcdc_num;

extern TCC_OUTPUT_TYPE	Output_SelectMode;

static unsigned int g_ignore_overlay_priority = 0;

unsigned char tccxxx_overlay_use(void)
{
	return tcc_overlay_use;
}
EXPORT_SYMBOL(tccxxx_overlay_use);

void tccxxx_overlay_start(void)
{
	if(!start_en){
		dprintk("call start en \n");		
		start_en = 1;
	}
}
EXPORT_SYMBOL(tccxxx_overlay_start);

extern int range_is_allowed(unsigned long pfn, unsigned long size);
static int tccxxx_overlay_mmap(struct file *file, struct vm_area_struct *vma)
{
	if(range_is_allowed(vma->vm_pgoff, vma->vm_end - vma->vm_start) < 0){
		printk(KERN_ERR  "overlay: this address is not allowed \n");
		return -EAGAIN;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma,vma->vm_start, vma->vm_pgoff , vma->vm_end - vma->vm_start, vma->vm_page_prot))
	{
		return -EAGAIN;
	}

	vma->vm_ops	= NULL;
	vma->vm_flags 	|= VM_IO;
	vma->vm_flags 	|= VM_DONTEXPAND | VM_DONTDUMP;
	
	return 0;
}

DECLARE_WAIT_QUEUE_HEAD(overlay_wait);

static unsigned int tccxxx_overlay_poll(struct file *file, struct poll_table_struct *wait)
{
	dprintk(" tccxxx_overlay_poll wait[%d][%d]!!!\n", (unsigned)wait, (unsigned)&overlay_wait);
	poll_wait(file, &(overlay_wait), wait);	
	dprintk(" tccxxx_overlay_poll finish[%d][%d]!!!\n", (unsigned)wait, (unsigned)&overlay_wait);	
	return POLLIN;
}

static int tccxxx_overlay_get_pos(overlay_config_t * arg )
{
	overlay_config_t pos;

	pos.sx 		= overlay_cfg.sx;
	pos.sy 		= overlay_cfg.sy;
	pos.width	= overlay_cfg.width;
	pos.height	= overlay_cfg.height;
	dprintk(" Overlay -> Get Position :: (%d,%d) | (%d,%d) \n", overlay_cfg.sx, overlay_cfg.sy, overlay_cfg.width, overlay_cfg.height);
	
	if(copy_to_user((overlay_config_t *)arg, &pos, sizeof(overlay_config_t)))
		return -EFAULT;

	return 0;
}

static int tccxxx_overlay_get_screenInfo(overlay_config_t * arg )
{
	struct lcd_panel *panel;
	unsigned int screen_width, screen_height;

	overlay_config_t screen_info;

	panel = tccfb_get_panel();
    screen_width      = panel->xres;
    screen_height     = panel->yres;

#if defined(CONFIG_TCC_HDMI_UI_SIZE_1280_720)
    if(tcc_display_data.resolution == 1)
    {
        screen_width      = 720;
        screen_height     = 576;
    }
    else if(tcc_display_data.resolution == 2)
    {
        screen_width      = 800;
        screen_height     = 480;
    }
#endif	

	screen_info.sx 		= 0;
	screen_info.sy 		= 0;
	screen_info.width	= screen_width;
	screen_info.height	= screen_height;
	
	dprintk(" Overlay -> Get ScreenInfo :: (%d,%d) | (%d,%d) \n", screen_info.sx, screen_info.sy, screen_info.width, screen_info.height);
	
	if(copy_to_user((overlay_config_t *)arg, &screen_info, sizeof(overlay_config_t)))
		return -EFAULT;

	return 0;
}

void tccxxx_overlay_fmt_set(unsigned int fmt)
{
	dprintk(" Overlay -> S_FMT :: format = 0x%x(RGB565-0x%x, YUV420-0x%x, YUV420inter-0x%x) \n", fmt, V4L2_PIX_FMT_RGB565,V4L2_PIX_FMT_YVU420, V4L2_PIX_FMT_NV12);



	if(fmt  >= VIOC_IMG_FMT_UYVY)
	{
		VIOC_RDMA_SetImageFormat(overlay_info.pRDMABase, fmt);
		VIOC_RDMA_SetImageOffset(overlay_info.pRDMABase, fmt, overlay_cfg.width);
		#ifdef CONFIG_DISPLAY_PATH_Y2R_ENABLE
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 0);	
		#else
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 1);	
		#endif//
	}
	else	 if(fmt  <= VIOC_IMG_FMT_ARGB6666_3)
	{
		VIOC_RDMA_SetImageFormat(overlay_info.pRDMABase, fmt);
		VIOC_RDMA_SetImageOffset(overlay_info.pRDMABase, fmt, overlay_cfg.width);
		#ifdef CONFIG_DISPLAY_PATH_Y2R_ENABLE
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 1);	
		#else
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 0);	
		#endif//
	}
	else if(fmt == VIOC_IMG_FMT_COMP)
	{
		VIOC_PlugInOutCheck VIOC_PlugIn;

		VIOC_RDMA_SetImageFormat(overlay_info.pRDMABase, VIOC_IMG_FMT_COMP);
		VIOC_CONFIG_Device_PlugState(VIOC_FCDEC0,  &VIOC_PlugIn);
		dprintk("rdma index: %d enable : %d  plug in device: %d \n", overlay_info.rdma_index, VIOC_PlugIn.enable, VIOC_PlugIn.connect_device);
		if(VIOC_PlugIn.enable)
		{
			VIOC_CONFIG_SWReset(overlay_info.pIREQConfig, VIOC_CONFIG_FCDEC, 0, VIOC_CONFIG_RESET);
			VIOC_CONFIG_SWReset(overlay_info.pIREQConfig, VIOC_CONFIG_FCDEC, 0, VIOC_CONFIG_CLEAR);
			VIOC_CONFIG_PlugIn(VIOC_FCDEC0, overlay_info.rdma_index);		
		}
	}
	else
	{
		pr_err("ERROR FMT: %s fmt : %d", __func__, fmt);
	}

}


void tccxxx_overlay_common_enable(void)
{
	if(overlay_en_count < 2) //max overlay is 2.
		overlay_en_count++;
	dprintk("Enable :: overlay_en_count = %d \n", overlay_en_count);
}
EXPORT_SYMBOL(tccxxx_overlay_common_enable);

void tccxxx_overlay_common_disable(int channel, VIOC_RDMA *pRdmaBase)
{
	VIOC_RDMA* pRDMABase = pRdmaBase;
	unsigned int enable;

	dprintk("overlay disable ch:%d output mode:%d \n", channel, Output_Select);

	if(channel == 0)
	{
		
		if(Output_Select != OUTPUT_SELECT_NONE)
		{			
			VIOC_RDMA_SetImageDisable(pRDMABase);
		}

		VIOC_RDMA_GetImageEnable(pRDMABase, &enable);
		if(!enable)
			return;
	}
	else if(channel == 1)
	{

		VIOC_RDMA_GetImageEnable(pRDMABase, &enable);
		if(!enable)
			return;
	}
	else
	{
		//to do
	}

	if(overlay_en_count > 0)
		overlay_en_count--;


	if((!overlay_en_count)
	)
	{
		unsigned int nKeyEn, nKeyR, nKeyG, nKeyB, nKeyMaskR, nKeyMaskG, nKeyMaskB;
		
		VIOC_WMIX_GetChromaKey(overlay_info.pWMIXBase, 0, &nKeyEn, &nKeyR, &nKeyG, &nKeyB, &nKeyMaskR, &nKeyMaskG, &nKeyMaskB);
		nKeyEn = 0;
		VIOC_WMIX_SetChromaKey(overlay_info.pWMIXBase, 0, nKeyEn, nKeyR, nKeyG, nKeyB, nKeyMaskR, nKeyMaskG, nKeyMaskB);
	}

	if(Output_Select == OUTPUT_SELECT_NONE)
	{
		if(channel == 0)
		{
			VIOC_RDMA_SetImageDisable(pRDMABase);
		}
		else if(channel == 1)
		{
			VIOC_RDMA_SetImageDisable(pRDMABase);
		}
		else
		{
			
		}
	}
	
	dprintk("Disable :: overlay_en_count = %d \n", overlay_en_count);
}
EXPORT_SYMBOL(tccxxx_overlay_common_disable);

int tccxxx_overlay_q_buffer(unsigned int curY_phyAddr )
{
	unsigned int curU_phyAddr, curV_phyAddr;
	unsigned int enable;

//	dprintk(" Overlay -> Q_Buffer :: buffer = 0x%x pos_reset:%d start_en:%d\n", curY_phyAddr, pos_reset, start_en);

#ifdef VC_OVERLAY_PROFILE
	if(toggle_int)
	{
		(HwGPIOD->GPEN |= Hw24);	(HwGPIOD->GPDAT |= Hw24);
		toggle_int = 0;
	}
	else
	{
		(HwGPIOD->GPEN |= Hw24);	(HwGPIOD->GPDAT &= ~Hw24);
		toggle_int = 1;
	}
#endif

	//in case position reset in streamming.
	if(pos_reset)
	{
		pos_reset = 0;
		VIOC_WMIX_SetPosition(overlay_info.pWMIXBase, VIDEO_CH_NUM,  overlay_cfg.sx, overlay_cfg.sy);
		VIOC_RDMA_SetImageSize(overlay_info.pRDMABase, overlay_cfg.width, overlay_cfg.height);
		tccxxx_overlay_fmt_set(overlay_fmt.fmt.pix.pixelformat);		
	}

	// image address
	curU_phyAddr = GET_ADDR_YUV42X_spU(curY_phyAddr, overlay_cfg.width, overlay_cfg.height); 
	curV_phyAddr = GET_ADDR_YUV420_spV(curU_phyAddr, overlay_cfg.width, overlay_cfg.height);

	VIOC_RDMA_SetImageBase(overlay_info.pRDMABase, curY_phyAddr, curU_phyAddr,curV_phyAddr );
	
	if(!start_en)
	{
		tccxxx_overlay_common_enable();
		tccxxx_overlay_fmt_set(overlay_fmt.fmt.pix.pixelformat);
		
		VIOC_RDMA_SetImageIntl(overlay_info.pRDMABase , 0);
		VIOC_RDMA_SetImageEnable(overlay_info.pRDMABase );

		start_en = 1;
	}
	
	VIOC_RDMA_GetImageEnable(overlay_info.pRDMABase, &enable);
	if(!enable)
		VIOC_RDMA_SetImageEnable(overlay_info.pRDMABase);
	
	VIOC_WMIX_SetUpdate(overlay_info.pWMIXBase);
	VIOC_RDMA_SetImageUpdate(overlay_info.pRDMABase);
	
	return 0;
}


static int tccxxx_overlay_disable(void)
{
	VIOC_RDMA_SetImageDisable(overlay_info.pRDMABase);

	wait_restart = 1;
	return 0;
}

int tccxxx_overlay_set_centering(overlay_config_t pos)
{
	struct lcd_panel *panel = tccfb_get_panel();
	unsigned int screen_width, screen_height;

	overlay_cfg.sx		=	pos.sx; 	
	overlay_cfg.sy		=	pos.sy; 	
	overlay_cfg.width	=	pos.width;

/*
	if(overlay_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565 || overlay_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV422P)
		overlay_cfg.width	=	((overlay_cfg.width+3) >> 2)<<2;
	else
		overlay_cfg.width	=	((overlay_cfg.width+15) >> 4)<<4;
*/
	overlay_cfg.height	=	pos.height;

	screen_width      = panel->xres;
	screen_height     = panel->yres;

#if defined(CONFIG_TCC_HDMI_UI_SIZE_1280_720)
	if(tcc_display_data.resolution == 1)
	{
		screen_width      = 720;
		screen_height     = 576;
	}
	else if(tcc_display_data.resolution == 2)
	{
		screen_width      = 800;
		screen_height     = 480;
	}
#endif
	
	if(overlay_cfg.sx + overlay_cfg.width > screen_width)
	{
		if(overlay_cfg.width > screen_width)
		{
			overlay_cfg.sx = 0;
			overlay_cfg.width = screen_width;
		}
		else
		{		
			overlay_cfg.sx = (screen_width - overlay_cfg.width)/2;			
		}
	}

	if(overlay_cfg.sy + overlay_cfg.height > screen_height)
	{
		if(overlay_cfg.height > screen_height)
		{
			overlay_cfg.sy = 0;
			overlay_cfg.height = screen_height;
		}
		else
		{		
			overlay_cfg.sy = (screen_height - overlay_cfg.height)/2;			
		}
	}

	return 0;
}


static int tccxxx_overlay_set_pos(overlay_config_t pos )
{
	if(!start_en)
	{
		VIOC_RDMA_SetImageDisable(overlay_info.pRDMABase);
		wait_restart = 1;
	}
	
	tccxxx_overlay_set_centering(pos);
		
	dprintk(" Overlay -> Set Position adjust :: (%d,%d) | (%d,%d) \n", overlay_cfg.sx, overlay_cfg.sy, overlay_cfg.width, overlay_cfg.height);

	//in case position reset in streamming.
	if(start_en)
	{
		pos_reset = 1;
		return 0;
	}

	VIOC_WMIX_SetPosition(overlay_info.pWMIXBase, VIDEO_CH_NUM,  overlay_cfg.sx, overlay_cfg.sy);
	VIOC_RDMA_SetImageSize(overlay_info.pRDMABase, overlay_cfg.width, overlay_cfg.height);

	tccxxx_overlay_fmt_set(overlay_fmt.fmt.pix.pixelformat);
	
	VIOC_WMIX_SetUpdate(overlay_info.pWMIXBase);
	VIOC_RDMA_SetImageUpdate(overlay_info.pRDMABase);

	return 0;
}

static int tccxxx_overlay_set_configure(overlay_config_t config)
{
	unsigned int screen_width, screen_height;
	struct lcd_panel *panel = tccfb_get_panel();

	overlay_fmt.fmt.pix.width 	 	= config.width;
	overlay_fmt.fmt.pix.height 		= config.height ;
	overlay_fmt.fmt.pix.pixelformat	= config.format;
	dprintk(" Overlay -> S_FMT :: size(%d,%d), format = 0x%x(RGB565-0x%x, YUV420-0x%x, YUV420inter-0x%x) \n", overlay_fmt.fmt.pix.width, overlay_fmt.fmt.pix.height, overlay_fmt.fmt.pix.pixelformat, V4L2_PIX_FMT_RGB565,V4L2_PIX_FMT_YVU420, V4L2_PIX_FMT_NV12);

	overlay_cfg.sx		=	config.sx;	
	overlay_cfg.sy		=	config.sy;		
	overlay_cfg.width	=	overlay_fmt.fmt.pix.width;
/*
	if(overlay_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565 || overlay_fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV422P)
		overlay_cfg.width	=	((overlay_cfg.width+3) >> 2)<<2;
	else
		overlay_cfg.width	=	((overlay_cfg.width+15) >> 4)<<4;
*/
	overlay_cfg.height	=	overlay_fmt.fmt.pix.height;

    screen_width      = panel->xres;
    screen_height     = panel->yres;
#if defined(CONFIG_TCC_HDMI_UI_SIZE_1280_720)
    if(tcc_display_data.resolution == 1)
    {
        screen_width      = 720;
        screen_height     = 576;
    }
    else if(tcc_display_data.resolution == 2)
    {
        screen_width      = 800;
        screen_height     = 480;
    }
#endif

	if(overlay_cfg.sx + overlay_cfg.width > screen_width)
	{
		if(overlay_cfg.width > screen_width)
		{
			overlay_cfg.sx = 0;
			overlay_cfg.width = screen_width;
		}
		else
		{		
			overlay_cfg.sx = (screen_width - overlay_cfg.width)/2;			
		}
	}

	if(overlay_cfg.sy + overlay_cfg.height > screen_height)
	{
		if(overlay_cfg.height > screen_height)
		{
			overlay_cfg.sy = 0;
			overlay_cfg.height = screen_height;
		}
		else
		{		
			overlay_cfg.sy = (screen_height - overlay_cfg.height)/2;			
		}
	}

	dprintk(" Overlay -> S_FMT :: Real => size(%d,%d ~ %d,%d) \n", overlay_cfg.sx, overlay_cfg.sy, overlay_cfg.width, overlay_cfg.height);

//in case position reset in streamming.
	if(start_en)
	{
		pos_reset = 1;
		return 0;
	}
	VIOC_WMIX_SetPosition(overlay_info.pWMIXBase, VIDEO_CH_NUM,  overlay_cfg.sx, overlay_cfg.sy);
	VIOC_RDMA_SetImageSize(overlay_info.pRDMABase, overlay_cfg.width, overlay_cfg.height);
	
	tccxxx_overlay_fmt_set(overlay_fmt.fmt.pix.pixelformat);

	return 0;
}

int tccxxx_overlay_ignore_priority(unsigned int* arg )
{
	unsigned int ignore_priority=0;
	
	if(copy_from_user(&ignore_priority, (unsigned int *)arg, sizeof(unsigned int)))
		return -EFAULT;

	g_ignore_overlay_priority = ignore_priority;
	printk("g_ignore_overlay_priority:%d\n", g_ignore_overlay_priority);

	return 0;
}

static int tccxxx_overlay_display_video_buffer(overlay_video_buffer_t buffer_cfg)
{
	unsigned int curU_phyAddr, curV_phyAddr;
	dprintk("%s addr:0x%x \n",__func__, buffer_cfg.addr);
	dprintk("fmt : 0x%x position:%d %d  size: %d %d \n", buffer_cfg.cfg.format, buffer_cfg.cfg.sx, buffer_cfg.cfg.sy, buffer_cfg.cfg.width, buffer_cfg.cfg.height);

#ifdef OVERLAY_DRV_CHROMA_EN
	switch(overlay_info.rdma_index) {
	case 1 /* VIOC_RDMA01 */:
		VIOC_WMIX_SetChromaKey(overlay_info.pWMIXBase, 1, ON, 0x10, 0x10, 0x10, 0xF8, 0xFC, 0xF8);
		break;
	case 2 /* VIOC_RDMA02 */:
		VIOC_WMIX_SetChromaKey(overlay_info.pWMIXBase, 2, ON, 0x10, 0x10, 0x10, 0xF8, 0xFC, 0xF8);
		break;
	}
#endif
	VIOC_WMIX_SetPosition(overlay_info.pWMIXBase, VIDEO_CH_NUM,  buffer_cfg.cfg.sx, buffer_cfg.cfg.sy);
	VIOC_WMIX_SetUpdate(overlay_info.pWMIXBase);
	VIOC_RDMA_SetImageFormat(overlay_info.pRDMABase, buffer_cfg.cfg.format);
	VIOC_RDMA_SetImageSize(overlay_info.pRDMABase,  buffer_cfg.cfg.width, buffer_cfg.cfg.height);
	VIOC_RDMA_SetImageOffset(overlay_info.pRDMABase, buffer_cfg.cfg.format, buffer_cfg.cfg.width);

	if(buffer_cfg.cfg.format  >= VIOC_IMG_FMT_COMP)
	{
		#ifdef CONFIG_DISPLAY_PATH_Y2R_ENABLE
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 0);	
		#else
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 1);	
		VIOC_RDMA_SetImageY2RMode(overlay_info.pRDMABase, 1); /* Y2RMode Default 0 (Studio Color) */
		#endif//

		if(buffer_cfg.cfg.format == VIOC_IMG_FMT_COMP) {
			VIOC_PlugInOutCheck VIOC_PlugIn;
			VIOC_CONFIG_Device_PlugState(VIOC_FCDEC0,  &VIOC_PlugIn);

			if(!VIOC_PlugIn.enable)		{
				VIOC_CONFIG_SWReset(overlay_info.pIREQConfig, VIOC_CONFIG_FCDEC, 0, VIOC_CONFIG_RESET);
				VIOC_CONFIG_SWReset(overlay_info.pIREQConfig, VIOC_CONFIG_FCDEC, 0, VIOC_CONFIG_CLEAR);
				VIOC_CONFIG_PlugIn(VIOC_FCDEC0, overlay_info.rdma_index);
			}
		}
	}
	else	 if(buffer_cfg.cfg.format  <= VIOC_IMG_FMT_ARGB6666_3)
	{
		#ifdef CONFIG_DISPLAY_PATH_Y2R_ENABLE
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 1);	
		#else
		VIOC_RDMA_SetImageY2REnable(overlay_info.pRDMABase, 0);	
		#endif//
	}
	else
	{
		pr_err("ERROR FMT: %s fmt : %d", __func__, buffer_cfg.cfg.format);
		return 0;
	}

	curU_phyAddr = GET_ADDR_YUV42X_spU(buffer_cfg.addr,  buffer_cfg.cfg.width, buffer_cfg.cfg.height); 
	curV_phyAddr = GET_ADDR_YUV420_spV(curU_phyAddr,  buffer_cfg.cfg.width, buffer_cfg.cfg.height);
	VIOC_RDMA_SetImageBase(overlay_info.pRDMABase, buffer_cfg.addr, curU_phyAddr, curV_phyAddr);
	VIOC_RDMA_SetImageEnable(overlay_info.pRDMABase);
	
	//add for pixel alpha
	VIOC_RDMA_SetImageAlphaSelect(overlay_info.pRDMABase, 1);       // please refer to TCC897X full spec (RDMACTRL-ASEL field)
	VIOC_RDMA_SetImageAlphaEnable(overlay_info.pRDMABase, 1);       // please refer to TCC897X full spec (RDMACTRL-AEN  field)
	
	return 0;	
}


static int overlay_forbid;
static long tccxxx_overlay_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int intArg;
	overlay_config_t overCfg;
	unsigned int curY_phyAddr;
	overlay_video_buffer_t overBuffCfg;

	switch(cmd)
	{
		case OVERLAY_PUSH_VIDEO_BUFFER:
			if(copy_from_user(&overBuffCfg, (overlay_video_buffer_t *)arg, sizeof(overlay_video_buffer_t)))
				return -EFAULT;
			
			return tccxxx_overlay_display_video_buffer(overBuffCfg);

		case OVERLAY_SET_CONFIGURE:
			if(copy_from_user(&overCfg, (overlay_config_t *)arg, sizeof(overlay_config_t)))
				return -EFAULT;

			return 0;

		case OVERLAY_SET_WMIXER_OVP:
		{
			unsigned int ovp = 0;
			if(copy_from_user((void *)&ovp, (const void *)arg,sizeof(unsigned int)))
				return -EFAULT;
			printk("%s: WMIXER_OVP :: %d   \n", __func__ , ovp);
			VIOC_WMIX_SetOverlayPriority(overlay_info.pWMIXBase, ovp);
		}
			break;
		default:
			dprintk(" Unsupported IOCTL(%d)!!!\n", cmd);      
			break;			
		}

		return 0;
	}

static int tccxxx_overlay_release(struct inode *inode, struct file *file)
{
	tcc_overlay_use--;
	dprintk(" ===========> tccxxx_overlay_release num:%d \n", tcc_overlay_use);

	if(tcc_overlay_use==0)
	{
		start_en = 0;
		wait_restart = 0;

		VIOC_PlugInOutCheck VIOC_PlugIn;
		VIOC_RDMA_SetImageDisable(overlay_info.pRDMABase);
		VIOC_CONFIG_Device_PlugState(VIOC_FCDEC0,  &VIOC_PlugIn);

		dprintk(" ===========> forced close num:%d \n", tcc_overlay_use);

		if(VIOC_PlugIn.enable && (overlay_info.rdma_index == VIOC_PlugIn.connect_device))		{
			pr_info("tcc overlay drv fcdec plug out  from rdma : %d \n", overlay_info.rdma_index);				
			VIOC_CONFIG_PlugOut(VIOC_FCDEC0);		
			VIOC_CONFIG_SWReset(overlay_info.pIREQConfig, VIOC_CONFIG_FCDEC, 0, VIOC_CONFIG_RESET);
			VIOC_CONFIG_SWReset(overlay_info.pIREQConfig, VIOC_CONFIG_FCDEC, 0, VIOC_CONFIG_CLEAR);
		}

		clk_disable_unprepare(overlay_lcdc_clk);
	}

	return 0;
}

static int tccxxx_overlay_open(struct inode *inode, struct file *file)
{
	tcc_overlay_use++;
	clk_prepare_enable(overlay_lcdc_clk);

	dprintk(" ===========> tccxxx_overlay_open num:%d \n", tcc_overlay_use);

	return 0;	
}


static int tcc_overlay_parse_dt(struct platform_device *pdev) 
{
	struct device_node *np, *vioc_node;
	struct device_node *wmixer_node, *rdma_node, *config_node;
	unsigned int index, channel;
	int ret = 0;

	vioc_node = of_parse_phandle(pdev->dev.of_node,"fbdisplay-overlay", 0);

	/* get overlay image channel(1~4) */
	if(of_property_read_u32(pdev->dev.of_node,"overlay-channel", &channel)){
		pr_warning( "could not find  overlay-channel.\n");
		channel = 2;    // default rdma index = 2
	}

	if(vioc_node)
	{		
		if (of_property_read_u32(vioc_node, "telechips,fbdisplay_num", &lcd_lcdc_num)){
			pr_err( "could not find  telechips,fbdisplay_nubmer\n");
			ret = -ENODEV;
		}

		if(lcd_lcdc_num)
		{
			np = of_find_node_by_name(vioc_node, "fbdisplay1");
		} else {
			np = of_find_node_by_name(vioc_node, "fbdisplay0");
		}		
		
		if (!np) {
			pr_err( "could not find fb node \n");
			return -ENODEV;
		}

		wmixer_node = of_parse_phandle(np, "telechips,wmixer", 0);	
		of_property_read_u32_index(np,"telechips,wmixer", 1, &index);
		
		if (!wmixer_node) {
			pr_err( "could not find wmixer node\n");
			ret = -ENODEV;
		}else {
			overlay_info.pWMIXBase= of_iomap(wmixer_node, index);
			//pr_info(" WMIXER %d %x  \n", index, overlay_info.pWMIXBase);
		}

		rdma_node = of_parse_phandle(np,"telechips,rdma", 0);

		if (!rdma_node) {
			pr_err( "could not find rdma node\n");
			ret = -ENODEV;
		}else {
			of_property_read_u32_index(np, "telechips,rdma", channel, &index);	// telechips,rdma = <&vioc_rdma 0 1 2 3>;
			overlay_info.pRDMABase= of_iomap(rdma_node, index);
			overlay_info.rdma_index = index;
			//pr_info(" RDMA %d %x  \n", index, overlay_info.pRDMABase);
		}

		config_node = of_find_compatible_node(NULL, NULL, "telechips,vioc_config");
		
		if (config_node) {
			overlay_info.pIREQConfig = (PVIOC_IREQ_CONFIG)of_iomap(config_node, 0);
		} 

	}
		
	pr_info("%s lcdc:%d wmixer_info:%p  rdma_info:%p \n", __func__, lcd_lcdc_num, overlay_info.pWMIXBase, overlay_info.pRDMABase);

	return ret;
}

static const struct file_operations tcc_overlay_fops =
{
	.owner          = THIS_MODULE,
	.poll           = tccxxx_overlay_poll,
	.unlocked_ioctl = tccxxx_overlay_ioctl,
	.mmap           = tccxxx_overlay_mmap,
	.open           = tccxxx_overlay_open,
	.release        = tccxxx_overlay_release,
};

static struct miscdevice overlay_misc_device =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "overlay",
    .fops  = &tcc_overlay_fops,
};

static int tcc_overlay_probe(struct platform_device *pdev)
{	
	overlay_lcdc_clk = clk_get(0, "lcdc1");
	
	 if(IS_ERR(overlay_lcdc_clk)){
               printk(" tcc_overlay_probe : failed to get lcdc clock \n");
               overlay_lcdc_clk = NULL;
               //return -ENODEV;
       }	 

	tcc_overlay_parse_dt(pdev);	 
	
	if (misc_register(&overlay_misc_device))
	{
	    printk(KERN_WARNING "OVERLAY: Couldn't register device.\n");
	    return -EBUSY;
	}



	
	return 0;
}

static int tcc_overlay_remove(struct platform_device *pdev)
{
    misc_deregister(&overlay_misc_device);

	return 0;
}

#ifdef CONFIG_PM
static VIOC_RDMA RDMA_BackUp;
static int tcc_overlay_suspend(struct platform_device *pdev, pm_message_t state)
{
	if(tcc_overlay_use != 0)
	{	
		printk("tcc_overlay_suspend %d opened\n", tcc_overlay_use);
		RDMA_BackUp = *overlay_info.pRDMABase;
		clk_disable_unprepare(overlay_lcdc_clk);
	}
	
	return 0;
}

static int tcc_overlay_resume(struct platform_device *pdev)
{
	if(tcc_overlay_use != 0)
	{	
		printk("tcc_overlay_resume %d opened\n", tcc_overlay_use);
		
		clk_prepare_enable(overlay_lcdc_clk);

		 *overlay_info.pRDMABase = RDMA_BackUp;
	}
	
	return 0;
}

#else //CONFIG_PM
#define tcc_overlay_suspend NULL
#define tcc_overlay_resume NULL
#endif //CONFIG_PM

#ifdef CONFIG_OF
static struct of_device_id tcc_overlay_of_match[] = {
       { .compatible = "telechips,tcc_overlay" },
       {}
};
MODULE_DEVICE_TABLE(of, tcc_overlay_of_match);
#endif


static struct platform_driver tcc_overlay_driver = {
	.probe          = tcc_overlay_probe,
	.remove         = tcc_overlay_remove,
	.suspend        = tcc_overlay_suspend,
	.resume         = tcc_overlay_resume,
	.driver         = {
	     .name   = "tcc_overlay",
	     .owner  = THIS_MODULE,
#ifdef CONFIG_OF
	     .of_match_table = of_match_ptr(tcc_overlay_of_match),
#endif
	},
};


static void __exit
tccxxx_overlay_cleanup(void)
{
	platform_driver_unregister(&tcc_overlay_driver);

	dprintk(" ===========> tccxxx_overlay_cleanup \n");
	
	return;
}

static char banner[] __initdata = KERN_INFO "TCC Overlay driver initializing\n";

static int __init 
tccxxx_overlay_init(void)
{
	printk(banner);

	platform_driver_register(&tcc_overlay_driver);

	return 0;
}


MODULE_AUTHOR("Telechips.");
MODULE_DESCRIPTION("TCC Video for Linux overlay driver");
MODULE_LICENSE("GPL");


module_init(tccxxx_overlay_init);
module_exit(tccxxx_overlay_cleanup);

