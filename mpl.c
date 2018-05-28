#include "mpl.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/errno.h>


static char *dev_name="/dev/input/event2";
static int event_fd = -1;
struct input_event eventData;
int pressure_value = 0;
int tempteraure_value = 0;

/*
    功能：读取事件，获取压力值和温度值
    返回值：0-failure   1-successful
*/
static int read_event()
{
    int rd = 0;

    rd = read(event_fd, &eventData, sizeof(struct input_event));

    if(rd < sizeof(struct input_event))
        return 0;
    if(eventData.type == EV_ABS)
    { 
        if(eventData.code == ABS_PRESSURE) {//压力值
            pressure_value = eventData.value;
        } else if(eventData.code == ABS_MISC) {//温度值
            tempteraure_value = eventData.value;
        }else{
			printf("high:--------------------------------------------------------------------------------------------%d;\n\n",eventData.value); 
		}
    }
    return 1;
}

int Mpldec(void)
{
    int done = 1;
    //写入1,启用input2使能
    int ret = system("echo 1 > /sys/class/input/input2/enable");
    if(ret < 0)
    {
        printf("echo enable error ret is :%d\n",ret);
        return -1;
    }

    if((event_fd = open(dev_name, O_RDWR)) < 0)
    {
        printf("open input %s device error\n", dev_name);
        system("echo 0 > /sys/class/input/input2/enable");//写入0,关闭使能
        return -1;
    }

    while (done)
    {
        fd_set fds;
        struct timeval tv;
        int r;
		float temperature;
		float pressure;
        FD_ZERO (&fds);
        FD_SET (event_fd, &fds);
        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        r = select (event_fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) {
            if (EINTR == errno)
                continue;
        }
        done = read_event();
        if(done == 1)
        {
            if(eventData.type == EV_ABS)
            {
                //十六进制
//                printf("type:%d; code:%3d; pressure:%3x; tempteraure:%3x\n\n",
//                       eventData.type,eventData.code,pressure_value,tempteraure_value);
                //十进制
                /* printf("type:%d; code:%3d; pressure:%3d; tempteraure:%3d\n\n",
                       eventData.type,eventData.code,pressure_value,tempteraure_value); */
				temperature	= tempteraure_value;
				temperature = temperature/32;
				pressure = pressure_value;
				pressure = pressure/4000;
				printf("temperature:%f   pressure:%f\n\n",temperature,pressure);
            }
        }
        sleep(1);
    }
    if(event_fd > 0)
    {
        system("echo 0 > /sys/class/input/input2/enable");//写入0,关闭使能
        close(event_fd);
        event_fd = -1;
    }
    return 0;
}
