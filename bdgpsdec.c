#include "bdgpsdec.h"
#include "stdlib.h"
#include<stdio.h>
#include<math.h>
#include "sms4g.h" 
#include "hfdata.h"


struct BDGPSReC mBDGPSReC;//���յ��Ļ�������

struct HFPOSTION mHfpostion;//λ����Ϣ

struct HFPDATA mHfdata;//����������Ϣ�Ļ���

struct HFPDATA mHFPdata[7] = {0};

int sendpre = 60;//�ϱ�λ��Ƶ��


void sendDwbuf(void);
void Sms4GsendDwbuf(void);

void sendSOSBD(void);
void Sms4GSOSbuf(void);
extern int sendOK;
int oldpoint = 0;
unsigned int Centeraddr = CENTERBDCARD;

char soskey = 0;
int mid = 0;

//�����ݷ��뻺����
void Putbdgpsdata(char *buf,int length){
	int i;
	if( (mBDGPSReC.HeadPoint + 1) % BDGPSReCSize !=  mBDGPSReC.TailPoint ){
		mBDGPSReC.BDGPSReCBUF[mBDGPSReC.HeadPoint].length = length;
		for(i = 0; i < length; i++){
			mBDGPSReC.BDGPSReCBUF[mBDGPSReC.HeadPoint].Buf[i] = *(buf + i);
		}
		mBDGPSReC.HeadPoint = (mBDGPSReC.HeadPoint + 1) % BDGPSReCSize;
	} 
}

//�ɼ�RNSS��Ϣ
//ͨ������λ����Ϣ���ı�mHfdata.Point�����ɼ���ͬʱ�̵���Ϣ
//�����ڵ��ú���ʱ��Ҫע��ı�mHfdata.Point��ֵ
void collectgpsdata()
{
	printf("collectgpsdata\n");
	//��ȡ5�����λ����Ϣ ��RD����ǰ��ȡ ��������ܵ�RD�������
	oldpoint = mHfdata.Point; 
	mHfdata.mHfBuf[mHfdata.Point].time = mHfpostion.time;
	printf("mHfdata.mHfBuf[%d].time: %d\n",mHfdata.Point,mHfdata.mHfBuf[mHfdata.Point].time);
	if(mHfpostion.dirlongitude == 'W') 
		mHfdata.mHfBuf[mHfdata.Point].longitude = -mHfpostion.longitude;
	else
		mHfdata.mHfBuf[mHfdata.Point].longitude = mHfpostion.longitude;
		
	if(mHfpostion.dirlatitude == 'S')
		mHfdata.mHfBuf[mHfdata.Point].latitude = -mHfpostion.latitude;
	else
		mHfdata.mHfBuf[mHfdata.Point].latitude = mHfpostion.latitude;
		
	mHfdata.mHfBuf[mHfdata.Point].speed = mHfpostion.speed;
	mHfdata.mHfBuf[mHfdata.Point].dirspeed = mHfpostion.dirspeed;
	mHfdata.mHfBuf[mHfdata.Point].dirlongitude = mHfpostion.dirlongitude;
	mHfdata.mHfBuf[mHfdata.Point].dirlatitude = mHfpostion.dirlatitude;
	mHfdata.mHfBuf[mHfdata.Point].high = mHfpostion.high;
	mHfdata.mHfBuf[mHfdata.Point].pmhigh = mHfpostion.pmhigh; 
//		printf("collectgpsdata -->	 %d\t\n",mHfdata.Point);

	//����������Ч
	if(mHfpostion.IsValid == 'A')
	{ 
//		printf("mHfpostion.IsValid == 'A' mid: %d\n",mid);
		//��4G����ʱʹ��4G�����ϱ�
		if(Is4GVliad == 1)
		{
//			printf("Is4GVliad == 1\n");
			mHfdata.mHfBuf[0].time = mHfpostion.time;
			if(mHfpostion.dirlongitude == 'W') 
				mHfdata.mHfBuf[0].longitude = -mHfpostion.longitude;
			else
				mHfdata.mHfBuf[0].longitude = mHfpostion.longitude;
				
			if(mHfpostion.dirlatitude == 'S')
				mHfdata.mHfBuf[0].latitude = -mHfpostion.latitude;
			else
				mHfdata.mHfBuf[0].latitude = mHfpostion.latitude;
				
			mHfdata.mHfBuf[0].speed = mHfpostion.speed;
			mHfdata.mHfBuf[0].dirspeed = mHfpostion.dirspeed;
			mHfdata.mHfBuf[0].dirlongitude = mHfpostion.dirlongitude;
			mHfdata.mHfBuf[0].dirlatitude = mHfpostion.dirlatitude;
			mHfdata.mHfBuf[0].high = mHfpostion.high;
			mHfdata.mHfBuf[0].pmhigh = mHfpostion.pmhigh; 
			SendLogin();
			sendOK = 5;
			while(sendOK > 0)
			{
				sendOK--; 
				sleep(1);
				//�����½�ɹ� 
				if(sendOK = 1)
				{
					if(soskey == 1)
						Sms4GSOSbuf();
					else
						Sms4GsendDwbuf(); 
						
					sendOK = 0;
				}
			} 
		}
		//��4G����ʱʹ�ñ����ϱ�
		else if(Is4GVliad == 0 && mid == sendpre-1)
		{
//			printf("Is4GVliad == 0\n");
			if(soskey == 1)
				sendSOSBD();		
			else
				sendDwbuf();
		}  

	}  	
}

/************************************************************
** ��������:  BDgpsdatadec
** ��������:  �ӻ�����������Ӧ������
** �������:  NULL
** �������:  NULL
** �� �� ֵ:  NULL
** ��    ��:  Donny
** ��    ��:  2017��7��12��
**************************************************************/
 void BDgpsdatadec(void)
 { 
// 	printf("BDgpsdatadec\n");
	char *token;
	int count = 0; 	//����ֵ

	int intpos;		//�м�����λ����Ϣֵ
	long timemid;	//�м�����ʱ����Ϣֵ
	float Lat;		//�м�����γ����Ϣֵ
	float Lon;		//�м����㾭����Ϣֵ
	float Dir;		//�м����㷽����Ϣֵ
	float High;		//�м�����߶���Ϣֵ
	float Speed;	//�м������ٶ���Ϣֵ
	char strnc;		//�м�����Ƚ���Ϣֵ
	struct tm Oldtime; 
	unsigned long t1time,t2time,t1;	  
	struct tm NewTime;
	Oldtime.tm_sec	= 0;
	Oldtime.tm_min	= 0;
	Oldtime.tm_hour	= 0;
	Oldtime.tm_mday	= 1;
	Oldtime.tm_mon  = 0;
	Oldtime.tm_year = 70;
	//printf("%s\t\n",mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf);
	if( mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'R' &&
		mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[4] == 'M' &&
		mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[5] == 'C')
	{ 
		token=strtok(mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf,",");
		while(token != NULL)
		{
			if(count == 1)
			{
				timemid = atol(token);
				//ʱ
				NewTime.tm_hour = timemid/10000;
				//��
				NewTime.tm_min = timemid/100%100;
				//��
				NewTime.tm_sec = timemid%100; 
//				printf("BDGPSReC -->  time Hms %d\t\n",timemid);
//				printf("BDGPSReC -->  NewTime  %d %d %d\t\n",NewTime.tm_hour,NewTime.tm_min,NewTime.tm_sec);
			}
			//ȡ����Чλ��
			else if(count == 2)
			{
//				printf("BDID:%d\n",BDID);
				strnc = strncmp("A", token, 1); 
				if(strnc != 0)
				{
					mHfpostion.IsValid = 'V'; 
					printf("BDGPSReC -->  V\n");
					return;
				}
				mHfpostion.IsValid = 'A'; 
				printf("BDGPSReC -->  A\n");
//				printf("BDGPSReC -->  isactive %s   %d\t\n",token,strnc);

			}
			else if(count == 3)
			{
				Lat = atof(token);
				//�õ���
				intpos = Lat/100;
				//�õ���
				Lat = Lat - intpos*100;
				//��תΪ��
				Lat = Lat*10/6; 
				//���¼ӻ�ȥ
				Lat = Lat + intpos*100;
				//��ȡγ��
				mHfpostion.latitude = Lat*10000 ;
//				printf("BDGPSReC -->  Lat %d\t\n",mHfpostion.latitude);
			}
			else if(count == 4)
			{
				strnc = strncmp("N", token, 1); 
				if(strnc == 0)
					mHfpostion.dirlatitude = 'N';
				else 
					mHfpostion.dirlatitude = 'S';
//				printf("BDGPSReC -->  Latdir %s   %d\t\n",token,strnc); 
			}
			else if(count == 5)
			{
				Lon = atof(token);
				//�õ���
				intpos = Lon/100;
				//�õ���
				Lon = Lon - intpos*100;
				 //��תΪ��
				Lon =  Lon*10/6;
				//���¼ӻ�ȥ
				Lon = Lon + intpos*100;
				//��ȡ����
				mHfpostion.longitude = Lon*10000;
//				printf("BDGPSReC -->  Lon %d\t\n",mHfpostion.longitude );
			}
			else if(count == 6)
			{
				strnc = strncmp("E", token, 1); 
				if(strnc == 0)
					mHfpostion.dirlongitude = 'E';
				else 
					mHfpostion.dirlongitude = 'W';
//				printf("BDGPSReC -->  Londir %s   %d\t\n",token,strnc); 
			}
			else if(count == 9)
			{
				timemid = atol(token); 
				//����
				NewTime.tm_mday =  timemid/10000;
				//����
				NewTime.tm_mon  =   timemid/100%100 - 1;
				//����
				NewTime.tm_year =  (timemid%100 + 100);
				printf("NewTime %d %d %d %d %d %d ",NewTime.tm_year,NewTime.tm_mon,NewTime.tm_mday,NewTime.tm_hour,NewTime.tm_min,NewTime.tm_sec);
				t1time = mktime(&Oldtime);
				t2time = mktime(&NewTime);
				t1 = t2time - t1time;
//				printf("t1:%d\n",t1);
				//sendpre ������
				mid = (t1 % sendpre);
//				printf("sendpre:%d\n",sendpre);
				mHfdata.Point = floor(mid/(sendpre * 1.0/5));
//				printf("mHfdata.Point:%d\n",mHfdata.Point);
				//��������� 
				mHfpostion.time = t1 + 8*3600;
				collectgpsdata();
//				printf("BDGPSReC -->  time allsecond %d\t\n",timemid);
			}
			count++; 
			token=strtok(NULL,",");
		}  
	}
	else if(  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'V' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[4] == 'T' &&
		      mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[5] == 'G')
	{ 
		token=strtok(mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf,",");
		while(token!=NULL)
		{
			if(count == 1)
			{
				Dir = atof(token);
				//�汱����
				mHfpostion.dirspeed = Dir;
//				printf("BDGPSReC -->  southdir %s  %f\t\n",token,Dir); 
			}
			else if(count == 4)
			{
				Speed = atof(token);
				//�Ե��ٶ�
				mHfpostion.speed = Speed *1.852;	//�ٶȴӽ�תΪKm/h
//				printf("BDGPSReC -->  speed %s   %f\t\n",token,Speed);
			}  
			count++; 
			token=strtok(NULL,",");
		} 
	}
	else if(  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'G' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[4] == 'G' &&
		      mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[5] == 'A')
	{ 
		token=strtok(mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf,",");
		while(token!=NULL)
		{ 
			if(count == 9)
			{
				High = atof(token);
				//0Ϊ��  1Ϊ��
				if(High > 0)
				{
					mHfpostion.pmhigh = 0;
				}
				else
				{
					mHfpostion.pmhigh = 1;         
				}
				//�߶�
				mHfpostion.high = High;
//		ss		printf("BDGPSReC -->  High %s   %f\t\n",token,High);
			} 
			count++; 
			token=strtok(NULL,",");
		} 
	}
	else if(  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'I' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[4] == 'C' &&
		      mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[5] == 'I')
	{ 
//		printf("BDGPSReC --> ici %s \t\n",mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf);  
	}
	else if(  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'F' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[4] == 'K' &&
		      mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[5] == 'I')
	{ 
//		printf("BDGPSReC --> fki %s \t\n",mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf);  
	}
	else if(  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[1] == 'C' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[2] == 'S' &&
		      mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'F')
	{ 
		token=strtok(mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf,",");
		while(token!=NULL)
		{ 
			if(count == 2)
			{ 
				
			}
			else if(count == 3)
			{
				sendpre = atoi(token);
			}
			else if(count == 4)
			{
				soskey = atoi(token);
			}
			else if(count == 5)
			{
				Is4GVliad = atoi(token);
			} 
			count++; 
			token=strtok(NULL,","); 
		}
	}	
	else if(  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[1] == 'B' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[2] == 'D' &&
			  mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[3] == 'I' &&
		      mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf[4] == 'D')
	{
//		printf("BDGPSReC --> BDID");
		token=strtok(mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf,",");
		while(token!=NULL)
		{ 
			if(count == 1)
			{ 
				BDID = atoi(token);
			}
			count++; 
			token=strtok(NULL,","); 
		}
	}
}  

/************************************************************
** ��������:  Lookgpsdata
** ��������:  �鿴����������
** �������:  NULL
** �������:  NULL
** �� �� ֵ:  NULL
** ��    ��:  Donny	
** ��    ��:  2017��7��13��
**************************************************************/
void Lookgpsdata(void)
{
	if(mBDGPSReC.HeadPoint != mBDGPSReC.TailPoint)//�ж��յ�����һ��
	{	
		BDgpsdatadec();
		memset(mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].Buf, 0 , mBDGPSReC.BDGPSReCBUF[mBDGPSReC.TailPoint].length);
		mBDGPSReC.TailPoint = (mBDGPSReC.TailPoint+1) % BDGPSReCSize; 		
	}  
}

char numTransLate(char hex)
{
	char a = hex;
	switch (a)
	{
		case 10:return 'A';
		case 11:return 'B';
		case 12:return 'C';
		case 13:return 'D';  
		case 14:return 'E'; 
		case 15:return 'F';   
		default:return '0' + a; 
	} 
}
/**
  * ������ֵ��ת��											  �� 
  */
void HextoStr(char *buf, char hex) {
	char mhex;
	mhex = numTransLate(hex/16);
	*(buf + 0)	= mhex;
	mhex = numTransLate(hex%16);
	*(buf + 1)	= mhex;
}
unsigned char sendnumber = 0;

/************************************************************
** ��������:  Sms4GSOSbuf
** ��������:  4G����SOS
** �������:  NULL
** �������:  NULL
** �� �� ֵ:  NULL
** ��    ��:  Donny	
** ��    ��:  2017��7��12��
**************************************************************/
void Sms4GSOSbuf()
{  
	char buf[200] = {0};
	char SendBuf[250] = {0};  
	char hex2str[2] = {0};
	char delaytime[4] = {0};
	char sum = 0;
	unsigned int  addr;
	char address[7] = {0};
	int i=0,j=0,sendi = 0;
	int mintime = 60;
	 
	buf[i++] = 0x0E;
	buf[i++] = 0x0E;//��Ϣͷ
	buf[i++] = sendnumber++;//������
	buf[i++] = 0x03;//��Ϣ���ͱ��
	buf[i++] = 0;//��Ϣ����
	
	buf[i++] = BDID/256/256;
	buf[i++] = BDID/256%256;
	buf[i++] = BDID%256;//��������
	buf[i++] = DeViceID/256/256/256;
	buf[i++] = DeViceID/256/256%256;
	buf[i++] = DeViceID/256%256;    
	buf[i++] = DeViceID%256;//�豸ID
	buf[i++] = mHfdata.mHfBuf[0].time/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].time/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].time/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].time%256;//ʱ�� 
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].longitude)%256;//���ȶ�  
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].latitude)%256;//γ��
	
	//�߶�ֵ
	if(mHfdata.mHfBuf[0].pmhigh == 0)
	{ 
		buf[i++] = mHfdata.mHfBuf[0].high/256%256;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;//�߶�ֵ
	}
	else
	{
		buf[i++] = (mHfdata.mHfBuf[0].high/256%256)|0x80;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;
	}
	//�ٶ�ֵ
	buf[i++] = (mHfdata.mHfBuf[0].speed/256%256);    
	buf[i++] = mHfdata.mHfBuf[0].speed%256;
	//����ֵ
	buf[i++] = (mHfdata.mHfBuf[0].dirspeed/256%256); 
	buf[i++] = mHfdata.mHfBuf[0].dirspeed%256;	

	buf[i++] = 'H';
	buf[i++] = 'E';
	buf[i++] = 'L';
	buf[i++] = 'P';
	buf[4] = i-5;//����ֵ  
	sum = 0; 
	for(j=0;j<i;j++)sum ^= buf[j]; 
	buf[i++] = sum;
	sms4g_Udpdata(buf,i); 
//	printf("4Gtoser -> %s \n", buf);	
}

/************************************************************
** ��������:  Sms4GsendDwbuf
** ��������:  4G����PRM��Ϣ
** �������:  NULL
** �������:  NULL
** �� �� ֵ:  NULL
** ��    ��:  Donny
** ��    ��:  2017��7��12��
**************************************************************/
void Sms4GsendDwbuf()
{  
//	printf("Sms4GsendDwbuf\n");
	char buf[200] = {0};
	char SendBuf[250] = {0};  
	char hex2str[2] = {0};
	char delaytime[4] = {0};
	char sum = 0;
	unsigned int  addr;
	char address[7] = {0};
	int i=0,j=0,sendi = 0;
	int mintime = 60;
	//����5
	buf[i++] = 1;
	//�豸ID
	buf[i++] = 0x00;
	//ʱ��
	buf[i++] = mHfdata.mHfBuf[0].time/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].time/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].time/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].time%256;
	//γ��
	buf[i++] = mHfdata.mHfBuf[0].latitude/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].latitude/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].latitude/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].latitude%256;
	//����
	buf[i++] = mHfdata.mHfBuf[0].longitude/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].longitude/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].longitude/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].longitude%256;
	//�߶�ֵ
	if(mHfdata.mHfBuf[0].pmhigh == 0)
	{ 
		buf[i++] = mHfdata.mHfBuf[0].high/256%256;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;
	}
	else
	{
		buf[i++] = (mHfdata.mHfBuf[0].high/256%256)|0x80;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;
	}
	//�ٶ�ֵ
	buf[i++] = (mHfdata.mHfBuf[0].speed/256%256);    
	buf[i++] = mHfdata.mHfBuf[0].speed%256;
	//����ֵ
	buf[i++] = (mHfdata.mHfBuf[0].dirspeed/256%256); 
	buf[i++] = mHfdata.mHfBuf[0].dirspeed%256;										 
	if(mHfdata.mHfBuf[0].dirlatitude == 'S')
		buf[i-2] = buf[i-2]|0x40;
	else 
		buf[i-2] = buf[i-2]&0xbf;  //γ�ȷ���
	
	if(mHfdata.mHfBuf[0].dirlongitude == 'W')
		buf[i-2]= buf[i-2]|0x80;
	else 
		buf[i-2] = buf[i-2]&0x7f;//���ȷ���
	 
	sum = 0;
	sendi = 0;//���ͻ�����
	sendi = sprintf(SendBuf,"$NWWZI,%2x,0,",sendnumber++);
    for(j=0;j<i;j++){//��дͨ������
		HextoStr(hex2str, buf[j]);
		SendBuf[sendi ++ ]	= hex2str[0];
		SendBuf[sendi ++ ]	= hex2str[1];
	}	 
	SendBuf[sendi ++ ] = '*';//�������ֵ
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]	= hex2str[0];
	SendBuf[sendi ++ ]	= hex2str[1];  
	SendBuf[sendi ++ ]	= 0x0D;
	SendBuf[sendi ++ ]	= 0x0A;  

	sms4g_Udpdata(SendBuf,sendi); 
	printf("4Gtoser ->\n");	
}
/**
  *�����·���sos
  */
void sendSOSBD()
{  
	char buf[200] = {0};
	char SendBuf[250] = {0};  
	char hex2str[2] = {0};
	char delaytime[4] = {0};
	char sum = 0;
	unsigned int  addr;
	char address[7] = {0};
	int i=0,j=0,sendi = 0;
	int mintime = 60;
	unsigned int FscID = CENTERBDCARD;//���з����ն�
	buf[i++] = 0xBF;
	buf[i++] = 0x02;//��Ϣ�ֶ�
	buf[i++] = DeViceID/256/256/256;
	buf[i++] = DeViceID/256/256%256;
	buf[i++] = DeViceID/256%256;    
	buf[i++] = DeViceID%256;//�豸ID
	buf[i++] = mHfdata.mHfBuf[0].time/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].time/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].time/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].time%256;//ʱ�� 
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].longitude)%256;//���ȶ�  
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].latitude)%256;//γ��
	//�߶�ֵ	 
	if(mHfdata.mHfBuf[0].pmhigh == 0)
	{ 
		buf[i++] = mHfdata.mHfBuf[0].high/256%256;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;
	}
	else
	{
		buf[i++] = (mHfdata.mHfBuf[0].high/256%256)|0x80;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;
	}
	buf[i++] = (0x0ffff &(mHfdata.mHfBuf[0].speed/256%256));    
	buf[i++] = (0x0ffff & (mHfdata.mHfBuf[0].speed%256));//�ٶ�ֵ
	buf[i++] = (0x0ffff &(mHfdata.mHfBuf[0].dirspeed/256%256)); 
	buf[i++] = (0x0ffff & (mHfdata.mHfBuf[0].dirspeed%256));//����ֵ 
	buf[i++] = 'H';
	buf[i++] = 'E';
	buf[i++] = 'L';
	buf[i++] = 'P';    
	sum = 0;
	sendi = 0;//���ͻ�����
	/*������Ϣͷ��ֵ*/
	SendBuf[sendi ++ ] = '$';
	SendBuf[sendi ++ ] = 'C'; 
	SendBuf[sendi ++ ] = 'C';
	SendBuf[sendi ++ ] = 'T'; 
	SendBuf[sendi ++ ] = 'X'; 
	SendBuf[sendi ++ ] = 'A';
	SendBuf[sendi ++ ] = ','; 
	sprintf(address,"%07d",Centeraddr);	
	for(j=0;j<7;j++){
		SendBuf[sendi ++ ] = address[j];
	}
	SendBuf[sendi ++ ] = ',';
	SendBuf[sendi ++ ] = '1';
	SendBuf[sendi ++ ] = ','; // 1-��ͨ
	SendBuf[sendi ++ ] = '1';
	SendBuf[sendi ++ ] = ','; // 1-����
	for(j=0;j<i;j++)
	{//��дͨ������
		HextoStr(hex2str, buf[j]);
		SendBuf[sendi ++ ]	= hex2str[0];
		SendBuf[sendi ++ ]	= hex2str[1];
	}
	SendBuf[sendi ++ ] = '*';//�������ֵ
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]	= hex2str[0];
	SendBuf[sendi ++ ]	= hex2str[1];  
	SendBuf[sendi ++ ]	= 0x0D;
	SendBuf[sendi ++ ]	= 0x0A; 
	writeComdata(SendBuf,sendi);
//	printf("bdtoser -> %s \n", SendBuf);	
}

/************************************************************
** ��������:  sendDwbuf
** ��������:  ��������PRM��Ϣ
** �������:  NULL
** �������:  NULL
** �� �� ֵ:  NULL
** ��    ��:  Donny	
** ��    ��:  2017��7��12��
**************************************************************/
void sendDwbuf()
{  
//	printf("sendDwbuf\n");
	char buf[200] = {0};
	char SendBuf[250] = {0};  
	char hex2str[2] = {0};
	char delaytime[4] = {0};
	char sum = 0;
	unsigned int  addr;
	char address[7] = {0};
	int i=0,j=0,sendi = 0;
	int mintime = 60;
	buf[i++] = 0xA4;
	buf[i++] = 0xA0;
	buf[i++] = 5;//����5
	buf[i++] = 0x00;//�豸ID
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time/256/256/256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time/256/256%256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time/256%256));    
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time%256));//ʱ��
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude/256/256/256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude/256/256%256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude/256%256));    
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude%256));//γ��
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude/256/256/256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude/256/256%256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude/256%256));    
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude%256));//����
	if(mHfdata.mHfBuf[0].pmhigh == 0){ //�߶�ֵ
		buf[i++] = mHfdata.mHfBuf[0].high/256%256;    buf[i++] = mHfdata.mHfBuf[0].high%256;//�߶�ֵ
	}else{
		buf[i++] = (mHfdata.mHfBuf[0].high/256%256)|0x80;    buf[i++] = mHfdata.mHfBuf[0].high%256;//�߶�ֵ
	}
	buf[i++] = (0x0ff &(mHfdata.mHfBuf[0].speed/256%256));    buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].speed%256));//�ٶ�ֵ
	buf[i++] = (0x0ff &(mHfdata.mHfBuf[0].dirspeed/256%256)); buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].dirspeed%256));//����ֵ										 
	if(mHfdata.mHfBuf[0].dirlatitude == 'S')	buf[i-2] = buf[i-2]|0x40;
	else buf[i-2] = buf[i-2]&0xbf;  //γ�ȷ���
	if(mHfdata.mHfBuf[0].dirlongitude == 'W')	buf[i-2]= buf[i-2]|0x80;
	else  buf[i-2] = buf[i-2]&0xef;	   //���ȷ���
	for(j=1;j<=4;j++){//������ĸ����ֵ
		buf[i++] = 0x00;//�豸ID										 
		buf[i++] = (mHfdata.mHfBuf[j].time - mHfdata.mHfBuf[0].time)%256;//ʱ���ֵ
		printf("mHfdata.mHfBuf[%d].time - mHfdata.mHfBuf[0].time:%d\n",j,(mHfdata.mHfBuf[j].time - mHfdata.mHfBuf[0].time)%256);
		if((mHfdata.mHfBuf[j].latitude - mHfdata.mHfBuf[0].latitude) > 0){
			buf[i++] = (mHfdata.mHfBuf[j].latitude - mHfdata.mHfBuf[0].latitude)/256%256; 
			buf[i++] = (mHfdata.mHfBuf[j].latitude - mHfdata.mHfBuf[0].latitude)%256;//γ�Ȳ�ֵ
		}else{
			buf[i++] = ((mHfdata.mHfBuf[0].latitude - mHfdata.mHfBuf[j].latitude)/256%256) |0x80; 
			buf[i++] = (mHfdata.mHfBuf[0].latitude - mHfdata.mHfBuf[j].latitude)%256;//γ�Ȳ�ֵ
		}
		if((mHfdata.mHfBuf[j].longitude - mHfdata.mHfBuf[0].longitude) > 0){
			buf[i++] = (mHfdata.mHfBuf[j].longitude - mHfdata.mHfBuf[0].longitude)/256%256; 
			buf[i++] = (mHfdata.mHfBuf[j].longitude - mHfdata.mHfBuf[0].longitude)%256;//���Ȳ�ֵ
		}else{
			buf[i++] = ((mHfdata.mHfBuf[0].longitude - mHfdata.mHfBuf[j].longitude)/256%256) |0x80; 
			buf[i++] = (mHfdata.mHfBuf[0].longitude - mHfdata.mHfBuf[j].longitude)%256;//���Ȳ�ֵ
		}																								   
		if(mHfdata.mHfBuf[j].pmhigh == 0){ //�߶�ֵ
			buf[i++] = mHfdata.mHfBuf[j].high/256%256;    buf[i++] = mHfdata.mHfBuf[j].high%256;//�߶�ֵ
		}else{
			buf[i++] = (mHfdata.mHfBuf[j].high/256%256)|0x80;    buf[i++] = mHfdata.mHfBuf[j].high%256;//�߶�ֵ
		}
			buf[i++] = (mHfdata.mHfBuf[0].speed/256%256);    buf[i++] = mHfdata.mHfBuf[0].speed%256;//�ٶ�ֵ
			buf[i++] = (mHfdata.mHfBuf[0].dirspeed/256%256); buf[i++] = mHfdata.mHfBuf[0].dirspeed%256;//����ֵ
			
		if(mHfdata.mHfBuf[j].dirlatitude == 'S')
			buf[i-2] = buf[i-2]|0x40;
		else 
			buf[i-2] = buf[i-2]&0xbf;  //���ȷ���
		
		if(mHfdata.mHfBuf[j].dirlongitude == 'W')
			buf[i-2]= buf[i-2]|0x80;
		else 
			buf[i-2] = buf[i-2]&0x7f;//γ�ȷ���
	}
	for(j=2;j<i;j++){
		sum ^=  buf[j];
	}
	buf[i++] = sum;//���㱨�ĵ�У���
	sum = 0;
	sendi = 0;//���ͻ�����
	/*������Ϣͷ��ֵ*/
	SendBuf[sendi ++ ] = '$'; SendBuf[sendi ++ ] = 'C'; SendBuf[sendi ++ ] = 'C';
	SendBuf[sendi ++ ] = 'T'; SendBuf[sendi ++ ] = 'X'; SendBuf[sendi ++ ] = 'A';
	SendBuf[sendi ++ ] = ',';
	addr = 455874;
	sprintf(address,"%07d",addr);	
	for(j=0;j<7;j++){
		SendBuf[sendi ++ ] = address[j];
	}
	SendBuf[sendi ++ ] = ',';
	SendBuf[sendi ++ ] = '1';SendBuf[sendi ++ ] = ','; // 1-��ͨ
	SendBuf[sendi ++ ] = '2';SendBuf[sendi ++ ] = ','; // 2-��ϴ��� ]
	for(j=0;j<i;j++){//��дͨ������
		HextoStr(hex2str, buf[j]);
		SendBuf[sendi ++ ]	= hex2str[0];
		SendBuf[sendi ++ ]	= hex2str[1];
	}
	SendBuf[sendi ++ ] = '*';//�������ֵ
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]	= hex2str[0];
	SendBuf[sendi ++ ]	= hex2str[1];  
	SendBuf[sendi ++ ]	= 0x0D;
	SendBuf[sendi ++ ]	= 0x0A; 
	
	writeComdata(SendBuf,sendi);
	printf("bdtoser -> %s \n", SendBuf);	
}
/*************���Ͷ�ȡ������Ϣ*************************/
void sendQucs()
{
	char SendBuf[100] = {0};
	char hex2str[4] = {0},sum=0;
	int sendi = 0,j=0;
	sendi = sprintf(SendBuf,"$DQCS,1");
	SendBuf[sendi ++ ] = '*';//�������ֵ
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]	= hex2str[0];
	SendBuf[sendi ++ ]	= hex2str[1];  
	SendBuf[sendi ++ ]	= 0x0D;
	SendBuf[sendi ++ ]	= 0x0A;  
	writeComdata(SendBuf,sendi); 
}


/************************************************************
** ��������:  sendBDIC
** ��������:  ���Ͷ���ָ��
** �������:  NULL
** �������:  NULL
** �� �� ֵ:  NULL
** ��    ��:  Donny
** ��    ��:  2017��8��02��
**************************************************************/
void sendBDIC()
{
	char SendBuf[100] = {0};
	char hex2str[4] = {0},sum=0;
	int sendi = 0,j=0;
	sendi = sprintf(SendBuf,"$BDIC,1");
	SendBuf[sendi ++ ] = '*';//�������ֵ
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]  = hex2str[0];
	SendBuf[sendi ++ ]  = hex2str[1];  
	SendBuf[sendi ++ ]  = 0x0D;
	SendBuf[sendi ++ ]  = 0x0A;  
	writeComdata(SendBuf,sendi); 
}

