#include "serialcom.h"
#include "bdgpsdec.h" 
#include "hfdata.h"

pthread_mutex_t mymutex=PTHREAD_MUTEX_INITIALIZER;
static int fd_write;

int set_serial_attr(int sp_fd, int band_rate, char data_bits, char odd_even, char stop_bits)
{
	struct termios opt;

	if (-1 == sp_fd)
	{
		return -1;
	}

	bzero(&opt, sizeof(struct termios));

	if (-1 == tcgetattr(sp_fd, &opt))
	{
		return -1;
	}

	opt.c_cflag |= CLOCAL | CREAD;

	opt.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	opt.c_oflag &= ~OPOST;
	opt.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	switch (band_rate)
	{
	case 600:
		if (-1 == cfsetospeed(&opt, B600))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B600))
		{
			return -1;
		}
		break;
	case 1200:
		if (-1 == cfsetospeed(&opt, B1200))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B1200))
		{
			return -1;
		}
		break;
	case 2400:
		if (-1 == cfsetospeed(&opt, B2400))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B2400))
		{
			return -1;
		}
		break;
	case 4800:
		if (-1 == cfsetospeed(&opt, B4800))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B4800))
		{
			return -1;
		}
		break;
	case 9600:
		if (-1 == cfsetospeed(&opt, B9600))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B9600))
		{
			return -1;
		}
		break;
	case 19200:
		if (-1 == cfsetospeed(&opt, B19200))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B19200))
		{
			return -1;
		}
		break;
	case 38400:
		if (-1 == cfsetospeed(&opt, B38400))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B38400))
		{
			return -1;
		}
		break;
	case 57600:
		if (-1 == cfsetospeed(&opt, B57600))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B57600))
		{
			return -1;
		}
		break;
	case 115200:
		if (-1 == cfsetospeed(&opt, B115200))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B115200))
		{
			return -1;
		}
		break;
	default:
		if (-1 == cfsetospeed(&opt, B115200))
		{
			return -1;
		}
		if (-1 == cfsetispeed(&opt, B115200))
		{
			return -1;
		}
	}

	switch (data_bits)
	{
	case 5:
		opt.c_cflag &= ~CSIZE;
		opt.c_cflag |= CS5;
		break;
	case 6:
		opt.c_cflag &= ~CSIZE;
		opt.c_cflag |= CS6;
		break;
	case 7:
		opt.c_cflag &= ~CSIZE;
		opt.c_cflag |= CS7;
		break;
	case 8:
	default:
		opt.c_cflag &= ~CSIZE;
		opt.c_cflag |= CS8;
	}

	switch (odd_even)
	{
	case 'n':
	case 'N':
		opt.c_cflag &= ~PARENB;
		break;
	case 'o':
	case 'O':
		opt.c_cflag |= INPCK;
		opt.c_cflag |= PARENB;
		opt.c_cflag |= PARODD;
		break;
	case 'e':
	case 'E':
		opt.c_cflag |= INPCK;
		opt.c_cflag |= PARENB;
		opt.c_cflag &= ~PARODD;
		break;
	default:
		opt.c_cflag &= ~PARENB;
	}

	switch (stop_bits)
	{
	case 1:
		opt.c_cflag &= ~CSTOPB;
		break;
	case 2:
		opt.c_cflag |= CSTOPB;
		break;
	default:
		opt.c_cflag &= ~CSTOPB;
	}

	if (-1 == tcflush(sp_fd, TCIOFLUSH))
	{
		return -1;
	}

	if (-1 == tcsetattr(sp_fd, TCSANOW, &opt))
	{
		return -1;
	}
	return 0;
}
/******往串口写入数据*************************/
int writeComdata(char *buf,int length)
{
	int fd_w;
	pthread_mutex_lock (&mymutex);
	fd_w = write(fd_write,buf,length);
	pthread_mutex_unlock(&mymutex);   
	return  fd_w;
}
/************串口的初始化工作****************/
void serialcom_init(void)
{ 

	char *dev  =  "/dev/ttymxc2";
	struct timeval tv;//定义超时控制结构
    fd_set fds;//文件描述符集合变量
	char buf[50];
	char bufBD[250];
	int nread,fd_sel,i;
	int bigin = 0;
	//open write 
    fd_write = open(dev, O_RDWR);
    if (-1 == fd_write)
    {
        printf("Open %s -1...\n", dev);
        return;
    }
    
	printf("Open %s success!\n",dev);

    if((set_serial_attr(fd_write,BAUDRATE,DATABITS,PARITY,STOPBITS))!=0)
    {
        printf("Set fd_write Serial Falied\n");
        return;
    }
	
	tv.tv_sec=1;//设定超时值5s
    tv.tv_usec=5000000;
	
		
    while(1)	//通过GPSOne串口，发送GPSOne定位请求 
    {
				
		FD_ZERO(&fds);//初始化文件描述符集合
		
		FD_SET(fd_write,&fds);//设置文件描述符集合的相应位 		
		
        fd_sel=select(fd_write+1,&fds,NULL,NULL,&tv); 
		
		if(fd_sel < 0)
		{ 
			printf("select  Falied\n");
			return;
		}
		else if(fd_sel == 0)
		{ 
			//printf("select-- No data\n");
		}
		else if(FD_ISSET(fd_write,&fds))	//若GPS串口设备有数据可读
		{
			nread = read(fd_write,buf,50); 
			if(nread > 0)
			{
				for(i=0;i<nread;i++)
				{
					/************开始判断接收以$开头和‘*’号结尾的数据*******************/
					if(bigin == 0 && buf[i] == '$')	//判定起始字符
					{
						bigin = 0;
						bufBD[bigin++] = buf[i];
					}
					else if(bigin > 0)
					{
						bufBD[bigin++] = buf[i];
						if(bigin > 3 && bufBD[bigin-3] == '*')
						{
							Putbdgpsdata(bufBD,bigin);
							memset(bufBD, 0 , bigin); 
							sem_post(&GPSBD_sem);
							bigin = 0;
						}
						if(bigin > 249)	//判定超过某个值的时候清空
						{ 
							bigin = 0;
						}
					}
				}   
				memset(buf,0,nread);  
			}
		}	 
    }
}