#ifndef _LINUX_MAIN_H_
#define _LINUX_MAIN_H_

#define IOCTL_FSC_NOTIFY_APP_READY         _IOR('M', 0x1, unsigned)
#define IOCTL_FSC_NOTIFY_ARM2_STOP         _IOR('M', 0x2, unsigned)
#define IOCTL_FSC_NOTIFY_ARM2_BUFF_MEMSET  _IOR('M', 0x3, unsigned)
#define IOCTL_FSC_VIDEO_BLACKSCREEN         _IOWR('M', 0x4, WCH_BUFF_INFO_T*)
#define IOCTL_FSC_NOTIFY_ARM1_READY        _IOR('M', 0x5, unsigned)
#define IOCTL_FSC_GET_ARM2_STATUS          _IOR('M', 0x8, unsigned)

#define IOCTL_BC_GPIOINIT _IOR('M', 0x6, unsigned)
#define IOCTL_BC_DETECTGPIO _IOR('M', 0x7, unsigned)
#define IOCTL_FSC_OUTOF_READ _IO('M', 0x8)

#define GPIOXLEVEL_LOW                _IOR('L', 0x1, unsigned)
#define GPIOXLEVEL_HIGH               _IOR('L', 0x2, unsigned)
#define LOWPOWER_OUTOF_READ           _IO('L', 0x3)

//可自定义消息类型
#define LOWPOWER_SIG   __SIGRTMIN+10
#define BACKCAR_SIG    __SIGRTMIN+11

//注意这个的宏定义一定要加，不然编译F_SETSIG会报错
#define _GNU_SOURCE

#endif

