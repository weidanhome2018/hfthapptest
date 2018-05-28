#ifdef __cplusplus
extern "C"{
#endif

/*=======================================================================
                                        INCLUDE FILES
=======================================================================*/
/* Standard Include Files */
#include <errno.h>

/* Verification Test Environment Include Files */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <linux/videodev2.h>
//#include <linux/mxc_v4l2.h>
#include <sys/mman.h>
#include <string.h>
#include <malloc.h>
//#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <linux/videodev2.h>
#include <dirent.h> 
#include "h264encoder.h" 
  
struct buffer {
	void *start;
	size_t length;
};

struct camera {
	char *device_name;
	int fd;
	int width;
	int height;
	int display_depth;
	int image_size;
	int frame_number;
	struct v4l2_capability v4l2_cap;
	struct v4l2_cropcap v4l2_cropcap;
	struct v4l2_format v4l2_fmt;
	struct v4l2_crop crop;
	struct buffer *buffers;
}; 

typedef unsigned char uint8_t; 


 

#define TEST_BUFFER_NUM 3

struct testbuffer
{
    unsigned char *start;
    size_t offset;
    unsigned int length;
};

int g_in_width = 320;
int g_in_height = 240;
int g_out_width = 320;
int g_out_height = 240;
int g_top = 0;
int g_left = 0;
int g_input = 0;
int g_capture_count = 100;
int g_rotate = 0;
int g_cap_fmt = V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_YUYV;
int g_camera_framerate = 6;
int g_extra_pixel = 0;
int g_capture_mode = 0;
int g_usb_camera = 0;
char g_v4l_device[100] = "/dev/video0";
 
 
static void print_pixelformat(char *prefix, int val)
{
    printf("%s: %c%c%c%c\n", prefix ? prefix : "pixelformat",
           val & 0xff,
           (val >> 8) & 0xff,
           (val >> 16) & 0xff,
           (val >> 24) & 0xff);
}

int start_capturing(int fd_v4l)
{
    unsigned int i;
    
    return 0;
}

int stop_capturing(int fd_v4l)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    return ioctl (fd_v4l, VIDIOC_STREAMOFF, &type);
}

int v4l_capture_setup(char *dev)
{
    struct v4l2_format fmt;
    struct v4l2_control ctrl;
    struct v4l2_streamparm parm;
    struct v4l2_crop crop;
    int fd_v4l = 0;
//    struct v4l2_mxc_dest_crop of;
    struct v4l2_dbg_chip_ident chip;
    struct v4l2_frmsizeenum fsize;
    struct v4l2_fmtdesc ffmt; 
    struct v4l2_requestbuffers req;

    //打开设备
    if ((fd_v4l = open(dev, O_RDWR, 0)) < 0)
    {
        printf("Unable to open %s\n", dev);
        return 0;
    }
    /* UVC driver does not support this ioctl */
    if (g_usb_camera != 1) {
        if (ioctl(fd_v4l, VIDIOC_DBG_G_CHIP_IDENT, &chip))
        {
            printf("VIDIOC_DBG_G_CHIP_IDENT failed.\n");
            return -1;
        }
        printf("sensor chip is %s\n", chip.match.name);
    }

    printf("sensor supported frame size:\n");
    fsize.index = 0;
    while (ioctl(fd_v4l, VIDIOC_ENUM_FRAMESIZES, &fsize) >= 0) {
        printf(" %dx%d\n", fsize.discrete.width,
               fsize.discrete.height);
        fsize.index++;
    }

    ffmt.index = 0;
    while (ioctl(fd_v4l, VIDIOC_ENUM_FMT, &ffmt) >= 0) {
        print_pixelformat("sensor frame format", ffmt.pixelformat);
        ffmt.index++;
    }

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = g_camera_framerate;
    parm.parm.capture.capturemode = g_capture_mode;

    if (ioctl(fd_v4l, VIDIOC_S_PARM, &parm) < 0)
    {
        printf("VIDIOC_S_PARM failed\n");
        return -1;
    }

    if (ioctl(fd_v4l, VIDIOC_S_INPUT, &g_input) < 0)
    {
        printf("VIDIOC_S_INPUT failed\n");
        return -1;
    }

    /* UVC driver does not implement CROP */
    if (g_usb_camera != 1) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd_v4l, VIDIOC_G_CROP, &crop) < 0)
        {
            printf("VIDIOC_G_CROP failed\n");
            return -1;
        }

        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c.width = g_in_width;
        crop.c.height = g_in_height;
        crop.c.top = g_top;
        crop.c.left = g_left;
        if (ioctl(fd_v4l, VIDIOC_S_CROP, &crop) < 0)
        {
            printf("VIDIOC_S_CROP failed\n");
            return -1;
        }
    } 
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = g_cap_fmt;
    fmt.fmt.pix.width = g_out_width;
    fmt.fmt.pix.height = g_out_height;
    if (g_extra_pixel){
        fmt.fmt.pix.bytesperline = g_out_width + g_extra_pixel * 2;
        fmt.fmt.pix.sizeimage = (g_out_width + g_extra_pixel * 2 )
                * (g_out_height + g_extra_pixel * 2) * 3 / 2;
    } else {
        fmt.fmt.pix.bytesperline = g_out_width;
        fmt.fmt.pix.sizeimage = 0;
    }

    if (ioctl(fd_v4l, VIDIOC_S_FMT, &fmt) < 0)
    {
        printf("set format failed\n");
        return 0;
    }

    /*
     * Set rotation
     * It's mxc-specific definition for rotation.
     */
    if (g_usb_camera != 1) {
        ctrl.id = V4L2_CID_PRIVATE_BASE + 0;
        ctrl.value = g_rotate;
        if (ioctl(fd_v4l, VIDIOC_S_CTRL, &ctrl) < 0)
        {
            printf("set ctrl failed\n");
            return 0;
        }
    }

	
    memset(&req, 0, sizeof (req));
    req.count = TEST_BUFFER_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_v4l, VIDIOC_REQBUFS, &req) < 0)
    {
        printf("v4l_capture_setup: VIDIOC_REQBUFS failed\n");
        return 0;
    }

    return fd_v4l;
}

void encode_frame(uint8_t *yuv_frame, size_t yuv_length,Encoder *en,FILE *h264_fp,uint8_t *h264_buf) {
	int h264_length = 0;
	
	
	//这里有一个问题，通过测试发现前6帧都是0，所以这里我跳过了为0的帧
	if (yuv_frame[0] == '\0')
		return;

	h264_length = compress_frame(en, -1, yuv_frame, h264_buf);
	if (h264_length > 0) {
		//写h264文件
		printf("yuv_length %d ---> h264_length %d \n ",yuv_length,h264_length);
		fwrite(h264_buf, h264_length, 1, h264_fp); 
	} 
}

int v4l_capture_test(int fd_v4l,Encoder *en,FILE *h264_fp,uint8_t *h264_buf)
{
    struct v4l2_buffer buf;
#if TEST_OUTSYNC_ENQUE
    struct v4l2_buffer temp_buf;
#endif
    struct v4l2_format fmt; 
    int count = g_capture_count; 
    enum v4l2_buf_type type;
	int i=0;
	
	struct testbuffer buffers[TEST_BUFFER_NUM];
	
	
    
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;; 
	
	
    printf("v4l_capture_setup open -- 1\n");
    if ((ioctl(fd_v4l, VIDIOC_G_FMT, &fmt) < 0) )
    {
        printf("get format failed\n");
        return -1;
    }
    else
    {
        printf("\t Width = %d", fmt.fmt.pix.width);
        printf("\t Height = %d", fmt.fmt.pix.height);
        printf("\t Image size = %d\n", fmt.fmt.pix.sizeimage);
        print_pixelformat(0, fmt.fmt.pix.pixelformat);
    }
	

    for (i = 0; i < TEST_BUFFER_NUM; i++)
    {
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if ((ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) < 0) )
        {
            printf("VIDIOC_QUERYBUF error\n");
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].offset = (size_t) buf.m.offset;
        buffers[i].start = mmap (NULL, buffers[i].length,
                                 PROT_READ | PROT_WRITE, MAP_SHARED,
                                 fd_v4l, buffers[i].offset);
        memset(buffers[i].start, 0xFF, buffers[i].length);
    }

    for (i = 0; i < TEST_BUFFER_NUM; i++)
    {
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.m.offset = buffers[i].offset;
        if (g_extra_pixel){
            buf.m.offset += g_extra_pixel *
                    (g_out_width + 2 * g_extra_pixel) + g_extra_pixel;
        } 
		if (ioctl (fd_v4l, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF error\n");
			return -1;
		}  
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl (fd_v4l, VIDIOC_STREAMON, &type) < 0) {
        printf("VIDIOC_STREAMON error\n");
        return -1;
    } 
	 
	count = 0;
    while(1) {
		count++;
        memset(&buf, 0, sizeof (buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl (fd_v4l, VIDIOC_DQBUF, &buf) < 0)	{
            printf("VIDIOC_DQBUF failed.\n");
        } 
		
		encode_frame(buffers[buf.index].start, fmt.fmt.pix.sizeimage,en,h264_fp,h264_buf); 
        //fwrite(buffers[buf.index].start, fmt.fmt.pix.sizeimage, 1, fd_y_file);
 
        //if (count >= TEST_BUFFER_NUM) {
		if (ioctl (fd_v4l, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF failed\n");
			break;
		} 
        // else
            // printf("buf.index %d\n", buf.index);
    }

    if (stop_capturing(fd_v4l) < 0)
    {
        printf("stop_capturing failed\n");
        return -1;
    } 
    close(fd_v4l);
    return 0;
}
 
void main(void)
{ 
	int fd_v4l; 
	uint8_t *h264_buf;
	char h264_file_name[100] = "Vedio1.h264\0";
	FILE *h264_fp; 
	Encoder enV; 
	compress_begin(&enV, g_in_width, g_in_height);
	h264_buf = (uint8_t *) malloc(
			sizeof(uint8_t) * g_in_width * g_in_height * 3); // 设置缓冲区
   
	h264_fp = fopen(h264_file_name, "wb+");
    fd_v4l = v4l_capture_setup("/dev/video1");  
    v4l_capture_test(fd_v4l,&enV,h264_fp,h264_buf); 
	fclose(h264_fp); 
}