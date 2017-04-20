#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
 
#include <asm/types.h>        
#include <linux/videodev2.h>
  
struct buffer {
        void *                  start;
        size_t                  length;
};
 
struct buffer *buffers;
unsigned long  n_buffers;
unsigned long file_length;

int file_fd;
char *dev_name = "/dev/video0";
int fd;

static int read_frame (void)
{
     struct v4l2_buffer buf;
     
     /*֡����*/
     buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     buf.memory = V4L2_MEMORY_MMAP;
     ioctl (fd, VIDIOC_DQBUF, &buf);

     write(file_fd,buffers[buf.index].start,buffers[buf.index].length);
     
     /*buf����*/
     ioctl(fd, VIDIOC_QBUF, &buf);

     return 1;
}
 
int main (int argc,char ** argv)
{
     struct v4l2_capability cap;
     struct v4l2_format fmt;
     struct v4l2_requestbuffers req;
     struct v4l2_buffer buf; 
     unsigned int i;
     enum v4l2_buf_type type;
     
     
     file_fd = open("test.jpg", O_RDWR | O_CREAT, 0777);
    
     fd = open (dev_name, O_RDWR | O_NONBLOCK, 0);

     /*��ȡ������Ϣ*/
      ioctl (fd, VIDIOC_QUERYCAP, &cap);
      printf("Driver Name:%s\n Card Name:%s\n Bus info:%s\n\n",cap.driver,cap.card,cap.bus_info);
          
     /*����ͼ���ʽ*/
     fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     fmt.fmt.pix.width       = 320;
     fmt.fmt.pix.height      = 240;
     fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
     fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
     //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
     //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;

     ioctl (fd, VIDIOC_S_FMT, &fmt);
	 
     /*����ͼ�񻺳���*/
     req.count               = 4;
     req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     req.memory              = V4L2_MEMORY_MMAP;
     ioctl (fd, VIDIOC_REQBUFS, &req);
   
     
     buffers = calloc (req.count, sizeof (*buffers));
    
     for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
     { 
           /*��ȡͼ�񻺳�������Ϣ*/
           buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           buf.memory      = V4L2_MEMORY_MMAP;
           buf.index       = n_buffers;
 
           ioctl (fd, VIDIOC_QUERYBUF, &buf); 
             
           buffers[n_buffers].length = buf.length; 
           
           // ���ں˿ռ��е�ͼ�񻺳���ӳ�䵽�û��ռ�
          buffers[n_buffers].start = mmap (NULL ,    //ͨ��mmap����ӳ���ϵ
                                        buf.length,
                                        PROT_READ | PROT_WRITE ,
                                        MAP_SHARED ,
                                        fd,
                                        buf.m.offset);
     }

        
     /*ͼ�񻺳����*/ 
       
       for (i = 0; i < n_buffers; ++i)
       {
               buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
               buf.memory      = V4L2_MEMORY_MMAP;
               buf.index       = i; 
               ioctl (fd, VIDIOC_QBUF, &buf);
               
       }
    
    //��ʼ��׽ͼ������  
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl (fd, VIDIOC_STREAMON, &type);


   fd_set fds;

   FD_ZERO (&fds);
   FD_SET (fd, &fds);

   select(fd + 1, &fds, NULL, NULL, NULL);
   
   /*��ȡһ��ͼ��*/
   read_frame();

   for (i = 0; i < n_buffers; ++i)
      munmap (buffers[i].start, buffers[i].length);   

 

   close (fd);
   close (file_fd);
   printf("Camera Done.\n");


   return 0;
}
