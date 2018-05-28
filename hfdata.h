#ifndef _HFDATA_H
#define _HFDATA_H
#include <pthread.h>  
#include <unistd.h>  
#include <stdio.h>  
#include "sms4g.h"
#include <semaphore.h>
#include <time.h>
#define BDGPSReCSize 13 //����һ�����յĻ������Ĵ�С
#define BDGPSReCBufSize 250 //����һ����������������С
#define HfDataBufSize 5 //����һ�������̨��Ҫ�����ݵ������С

#define CENTERBDCARD  455874	//�����ϱ����Ŀ���
//#define CENTERBDCARD  458735	//����Ժ�ϱ����Ŀ���

#define LOCALBD  207885//����BD
#define DEVICEID 13072901//�豸ID


extern sem_t GPSBD_sem; //���ձ���һ�������ź���
extern char Is4GVliad;//char Is4GVliad;

/************************************************/
/*********������Ϣ����Ķ���*********************/
/************************************************/
struct HFPOSTION
{
  char IsValid;//�Ƿ���Ч
  unsigned long time; //ʱ��
  long longitude; //����
  long latitude;	//γ�� 
  int speed;//�ٶ�
  int dirspeed;//���� 
  char dirlongitude; //���ȷ��� 
  char dirlatitude;	//γ�ȷ��� 
  int high;//�߶�
  char  pmhigh;//�߶�����ֵ
};

struct HFPDATA
{
   int Point;
   struct HFPOSTION mHfBuf[7];
};
  
/************************************************/
/*********�����������������Ϣ������*************/
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
/*********�����������������Ƶ�����ݽṹ*********/
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

extern unsigned int BDID;//�ն˿���
extern unsigned int DeViceID;//�豸����
extern unsigned int Centeraddr;//���Ŀ���



#endif