#ifndef _HFDATA_H
#define _HFDATA_H
#include <pthread.h>  
#include <unistd.h>  
#include <stdio.h>  
#include "sms4g.h"
#include <semaphore.h>
#include <time.h>
#define BDGPSReCSize 13 //定义一个接收的缓冲区的大小
#define BDGPSReCBufSize 250 //定义一个缓冲区存的数组大小
#define HfDataBufSize 5 //定义一个海丰后台需要的数据的数组大小

#define CENTERBDCARD  455874	//海丰上报中心卡号
//#define CENTERBDCARD  458735	//航科院上报中心卡号

#define LOCALBD  207885//本地BD
#define DEVICEID 13072901//设备ID


extern sem_t GPSBD_sem; //接收北斗一代数据信号量
extern char Is4GVliad;//char Is4GVliad;

/************************************************/
/*********海丰信息构体的定义*********************/
/************************************************/
struct HFPOSTION
{
  char IsValid;//是否有效
  unsigned long time; //时间
  long longitude; //经度
  long latitude;	//纬度 
  int speed;//速度
  int dirspeed;//航向 
  char dirlongitude; //经度方向 
  char dirlatitude;	//纬度方向 
  int high;//高度
  char  pmhigh;//高度正负值
};

struct HFPDATA
{
   int Point;
   struct HFPOSTION mHfBuf[7];
};
  
/************************************************/
/*********定义接收用来处理消息缓冲区*************/
/************************************************/
struct BDGPSBUF
{
  int length;
  char Buf[BDGPSReCBufSize];
};

struct BDGPSReC
{
	int HeadPoint;
	int TailPoint;
	struct BDGPSBUF  BDGPSReCBUF[BDGPSReCSize];
};
/************************************************/
/*********定义接收用来处理视频的数据结构*********/
/************************************************/
typedef struct VideoBuffer
{
		void *start;
		size_t length;
}VideoBuffer;
typedef struct data
{
		unsigned int datasize;
		char buf[];
}buf_t;

extern unsigned int BDID;//终端卡号
extern unsigned int DeViceID;//设备卡号
extern unsigned int Centeraddr;//中心卡号



#endif