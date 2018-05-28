#include "bdgpsdec.h"
#include "stdlib.h"
#include<stdio.h>
#include<math.h>
#include "sms4g.h" 
#include "hfdata.h"


struct BDGPSReC mBDGPSReC;//接收到的缓存数据

struct HFPOSTION mHfpostion;//位置信息

struct HFPDATA mHfdata;//海丰数据信息的缓存

struct HFPDATA mHFPdata[7] = {0};

int sendpre = 60;//上报位置频度


void sendDwbuf(void);
void Sms4GsendDwbuf(void);

void sendSOSBD(void);
void Sms4GSOSbuf(void);
extern int sendOK;
int oldpoint = 0;
unsigned int Centeraddr = CENTERBDCARD;

char soskey = 0;
int mid = 0;

//将数据放入缓冲区
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

//采集RNSS信息
//通过传入位置信息，改变mHfdata.Point，来采集不同时刻的信息
//所以在调用函数时需要注意改变mHfdata.Point的值
void collectgpsdata()
{
	printf("collectgpsdata\n");
	//获取5个点的位置信息 在RD发射前获取 否则可能受到RD发射干扰
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

	//北斗数据有效
	if(mHfpostion.IsValid == 'A')
	{ 
//		printf("mHfpostion.IsValid == 'A' mid: %d\n",mid);
		//有4G网络时使用4G网络上报
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
				//如果登陆成功 
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
		//无4G网络时使用北斗上报
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
** 函数名称:  BDgpsdatadec
** 功能描述:  从缓冲区解析相应的数据
** 输入参数:  NULL
** 输出参数:  NULL
** 返 回 值:  NULL
** 作    者:  Donny
** 日    期:  2017年7月12日
**************************************************************/
 void BDgpsdatadec(void)
 { 
// 	printf("BDgpsdatadec\n");
	char *token;
	int count = 0; 	//计数值

	int intpos;		//中间运算位置信息值
	long timemid;	//中间运算时间信息值
	float Lat;		//中间运算纬度信息值
	float Lon;		//中间运算经度信息值
	float Dir;		//中间运算方向信息值
	float High;		//中间运算高度信息值
	float Speed;	//中间运算速度信息值
	char strnc;		//中间运算比较信息值
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
				//时
				NewTime.tm_hour = timemid/10000;
				//分
				NewTime.tm_min = timemid/100%100;
				//秒
				NewTime.tm_sec = timemid%100; 
//				printf("BDGPSReC -->  time Hms %d\t\n",timemid);
//				printf("BDGPSReC -->  NewTime  %d %d %d\t\n",NewTime.tm_hour,NewTime.tm_min,NewTime.tm_sec);
			}
			//取出有效位置
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
				//得到度
				intpos = Lat/100;
				//得到分
				Lat = Lat - intpos*100;
				//分转为度
				Lat = Lat*10/6; 
				//重新加回去
				Lat = Lat + intpos*100;
				//获取纬度
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
				//得到度
				intpos = Lon/100;
				//得到分
				Lon = Lon - intpos*100;
				 //分转为度
				Lon =  Lon*10/6;
				//重新加回去
				Lon = Lon + intpos*100;
				//获取经度
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
				//日期
				NewTime.tm_mday =  timemid/10000;
				//月数
				NewTime.tm_mon  =   timemid/100%100 - 1;
				//年数
				NewTime.tm_year =  (timemid%100 + 100);
				printf("NewTime %d %d %d %d %d %d ",NewTime.tm_year,NewTime.tm_mon,NewTime.tm_mday,NewTime.tm_hour,NewTime.tm_min,NewTime.tm_sec);
				t1time = mktime(&Oldtime);
				t2time = mktime(&NewTime);
				t1 = t2time - t1time;
//				printf("t1:%d\n",t1);
				//sendpre 发射间隔
				mid = (t1 % sendpre);
//				printf("sendpre:%d\n",sendpre);
				mHfdata.Point = floor(mid/(sendpre * 1.0/5));
//				printf("mHfdata.Point:%d\n",mHfdata.Point);
				//获得总秒数 
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
				//真北航向
				mHfpostion.dirspeed = Dir;
//				printf("BDGPSReC -->  southdir %s  %f\t\n",token,Dir); 
			}
			else if(count == 4)
			{
				Speed = atof(token);
				//对地速度
				mHfpostion.speed = Speed *1.852;	//速度从节转为Km/h
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
				//0为正  1为负
				if(High > 0)
				{
					mHfpostion.pmhigh = 0;
				}
				else
				{
					mHfpostion.pmhigh = 1;         
				}
				//高度
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
** 函数名称:  Lookgpsdata
** 功能描述:  查看缓冲区数据
** 输入参数:  NULL
** 输出参数:  NULL
** 返 回 值:  NULL
** 作    者:  Donny	
** 日    期:  2017年7月13日
**************************************************************/
void Lookgpsdata(void)
{
	if(mBDGPSReC.HeadPoint != mBDGPSReC.TailPoint)//判段收到北斗一代
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
  * 进行数值的转换											  、 
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
** 函数名称:  Sms4GSOSbuf
** 功能描述:  4G发送SOS
** 输入参数:  NULL
** 输出参数:  NULL
** 返 回 值:  NULL
** 作    者:  Donny	
** 日    期:  2017年7月12日
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
	buf[i++] = 0x0E;//消息头
	buf[i++] = sendnumber++;//包计数
	buf[i++] = 0x03;//消息类型编号
	buf[i++] = 0;//消息长度
	
	buf[i++] = BDID/256/256;
	buf[i++] = BDID/256%256;
	buf[i++] = BDID%256;//北斗卡号
	buf[i++] = DeViceID/256/256/256;
	buf[i++] = DeViceID/256/256%256;
	buf[i++] = DeViceID/256%256;    
	buf[i++] = DeViceID%256;//设备ID
	buf[i++] = mHfdata.mHfBuf[0].time/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].time/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].time/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].time%256;//时间 
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].longitude)%256;//经度度  
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].latitude)%256;//纬度
	
	//高度值
	if(mHfdata.mHfBuf[0].pmhigh == 0)
	{ 
		buf[i++] = mHfdata.mHfBuf[0].high/256%256;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;//高度值
	}
	else
	{
		buf[i++] = (mHfdata.mHfBuf[0].high/256%256)|0x80;    
		buf[i++] = mHfdata.mHfBuf[0].high%256;
	}
	//速度值
	buf[i++] = (mHfdata.mHfBuf[0].speed/256%256);    
	buf[i++] = mHfdata.mHfBuf[0].speed%256;
	//航向值
	buf[i++] = (mHfdata.mHfBuf[0].dirspeed/256%256); 
	buf[i++] = mHfdata.mHfBuf[0].dirspeed%256;	

	buf[i++] = 'H';
	buf[i++] = 'E';
	buf[i++] = 'L';
	buf[i++] = 'P';
	buf[4] = i-5;//长度值  
	sum = 0; 
	for(j=0;j<i;j++)sum ^= buf[j]; 
	buf[i++] = sum;
	sms4g_Udpdata(buf,i); 
//	printf("4Gtoser -> %s \n", buf);	
}

/************************************************************
** 函数名称:  Sms4GsendDwbuf
** 功能描述:  4G发送PRM信息
** 输入参数:  NULL
** 输出参数:  NULL
** 返 回 值:  NULL
** 作    者:  Donny
** 日    期:  2017年7月12日
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
	//数量5
	buf[i++] = 1;
	//设备ID
	buf[i++] = 0x00;
	//时间
	buf[i++] = mHfdata.mHfBuf[0].time/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].time/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].time/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].time%256;
	//纬度
	buf[i++] = mHfdata.mHfBuf[0].latitude/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].latitude/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].latitude/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].latitude%256;
	//经度
	buf[i++] = mHfdata.mHfBuf[0].longitude/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].longitude/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].longitude/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].longitude%256;
	//高度值
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
	//速度值
	buf[i++] = (mHfdata.mHfBuf[0].speed/256%256);    
	buf[i++] = mHfdata.mHfBuf[0].speed%256;
	//航向值
	buf[i++] = (mHfdata.mHfBuf[0].dirspeed/256%256); 
	buf[i++] = mHfdata.mHfBuf[0].dirspeed%256;										 
	if(mHfdata.mHfBuf[0].dirlatitude == 'S')
		buf[i-2] = buf[i-2]|0x40;
	else 
		buf[i-2] = buf[i-2]&0xbf;  //纬度方向
	
	if(mHfdata.mHfBuf[0].dirlongitude == 'W')
		buf[i-2]= buf[i-2]|0x80;
	else 
		buf[i-2] = buf[i-2]&0x7f;//经度方向
	 
	sum = 0;
	sendi = 0;//发送缓冲区
	sendi = sprintf(SendBuf,"$NWWZI,%2x,0,",sendnumber++);
    for(j=0;j<i;j++){//填写通信内容
		HextoStr(hex2str, buf[j]);
		SendBuf[sendi ++ ]	= hex2str[0];
		SendBuf[sendi ++ ]	= hex2str[1];
	}	 
	SendBuf[sendi ++ ] = '*';//计算检验值
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
  *北斗下发送sos
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
	unsigned int FscID = CENTERBDCARD;//飞行服务终端
	buf[i++] = 0xBF;
	buf[i++] = 0x02;//信息字段
	buf[i++] = DeViceID/256/256/256;
	buf[i++] = DeViceID/256/256%256;
	buf[i++] = DeViceID/256%256;    
	buf[i++] = DeViceID%256;//设备ID
	buf[i++] = mHfdata.mHfBuf[0].time/256/256/256;
	buf[i++] = mHfdata.mHfBuf[0].time/256/256%256;
	buf[i++] = mHfdata.mHfBuf[0].time/256%256;    
	buf[i++] = mHfdata.mHfBuf[0].time%256;//时间 
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].longitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].longitude)%256;//经度度  
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256/256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256/256%256;
	buf[i++] = (mHfdata.mHfBuf[0].latitude)/256%256;    
	buf[i++] = (mHfdata.mHfBuf[0].latitude)%256;//纬度
	//高度值	 
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
	buf[i++] = (0x0ffff & (mHfdata.mHfBuf[0].speed%256));//速度值
	buf[i++] = (0x0ffff &(mHfdata.mHfBuf[0].dirspeed/256%256)); 
	buf[i++] = (0x0ffff & (mHfdata.mHfBuf[0].dirspeed%256));//航向值 
	buf[i++] = 'H';
	buf[i++] = 'E';
	buf[i++] = 'L';
	buf[i++] = 'P';    
	sum = 0;
	sendi = 0;//发送缓冲区
	/*发送信息头部值*/
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
	SendBuf[sendi ++ ] = ','; // 1-普通
	SendBuf[sendi ++ ] = '1';
	SendBuf[sendi ++ ] = ','; // 1-代码
	for(j=0;j<i;j++)
	{//填写通信内容
		HextoStr(hex2str, buf[j]);
		SendBuf[sendi ++ ]	= hex2str[0];
		SendBuf[sendi ++ ]	= hex2str[1];
	}
	SendBuf[sendi ++ ] = '*';//计算检验值
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
** 函数名称:  sendDwbuf
** 功能描述:  北斗发送PRM信息
** 输入参数:  NULL
** 输出参数:  NULL
** 返 回 值:  NULL
** 作    者:  Donny	
** 日    期:  2017年7月12日
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
	buf[i++] = 5;//数量5
	buf[i++] = 0x00;//设备ID
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time/256/256/256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time/256/256%256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time/256%256));    
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].time%256));//时间
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude/256/256/256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude/256/256%256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude/256%256));    
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].latitude%256));//纬度
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude/256/256/256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude/256/256%256));
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude/256%256));    
	buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].longitude%256));//经度
	if(mHfdata.mHfBuf[0].pmhigh == 0){ //高度值
		buf[i++] = mHfdata.mHfBuf[0].high/256%256;    buf[i++] = mHfdata.mHfBuf[0].high%256;//高度值
	}else{
		buf[i++] = (mHfdata.mHfBuf[0].high/256%256)|0x80;    buf[i++] = mHfdata.mHfBuf[0].high%256;//高度值
	}
	buf[i++] = (0x0ff &(mHfdata.mHfBuf[0].speed/256%256));    buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].speed%256));//速度值
	buf[i++] = (0x0ff &(mHfdata.mHfBuf[0].dirspeed/256%256)); buf[i++] = (0x0ff & (mHfdata.mHfBuf[0].dirspeed%256));//航向值										 
	if(mHfdata.mHfBuf[0].dirlatitude == 'S')	buf[i-2] = buf[i-2]|0x40;
	else buf[i-2] = buf[i-2]&0xbf;  //纬度方向
	if(mHfdata.mHfBuf[0].dirlongitude == 'W')	buf[i-2]= buf[i-2]|0x80;
	else  buf[i-2] = buf[i-2]&0xef;	   //经度方向
	for(j=1;j<=4;j++){//填后面四个点的值
		buf[i++] = 0x00;//设备ID										 
		buf[i++] = (mHfdata.mHfBuf[j].time - mHfdata.mHfBuf[0].time)%256;//时间差值
		printf("mHfdata.mHfBuf[%d].time - mHfdata.mHfBuf[0].time:%d\n",j,(mHfdata.mHfBuf[j].time - mHfdata.mHfBuf[0].time)%256);
		if((mHfdata.mHfBuf[j].latitude - mHfdata.mHfBuf[0].latitude) > 0){
			buf[i++] = (mHfdata.mHfBuf[j].latitude - mHfdata.mHfBuf[0].latitude)/256%256; 
			buf[i++] = (mHfdata.mHfBuf[j].latitude - mHfdata.mHfBuf[0].latitude)%256;//纬度差值
		}else{
			buf[i++] = ((mHfdata.mHfBuf[0].latitude - mHfdata.mHfBuf[j].latitude)/256%256) |0x80; 
			buf[i++] = (mHfdata.mHfBuf[0].latitude - mHfdata.mHfBuf[j].latitude)%256;//纬度差值
		}
		if((mHfdata.mHfBuf[j].longitude - mHfdata.mHfBuf[0].longitude) > 0){
			buf[i++] = (mHfdata.mHfBuf[j].longitude - mHfdata.mHfBuf[0].longitude)/256%256; 
			buf[i++] = (mHfdata.mHfBuf[j].longitude - mHfdata.mHfBuf[0].longitude)%256;//经度差值
		}else{
			buf[i++] = ((mHfdata.mHfBuf[0].longitude - mHfdata.mHfBuf[j].longitude)/256%256) |0x80; 
			buf[i++] = (mHfdata.mHfBuf[0].longitude - mHfdata.mHfBuf[j].longitude)%256;//经度差值
		}																								   
		if(mHfdata.mHfBuf[j].pmhigh == 0){ //高度值
			buf[i++] = mHfdata.mHfBuf[j].high/256%256;    buf[i++] = mHfdata.mHfBuf[j].high%256;//高度值
		}else{
			buf[i++] = (mHfdata.mHfBuf[j].high/256%256)|0x80;    buf[i++] = mHfdata.mHfBuf[j].high%256;//高度值
		}
			buf[i++] = (mHfdata.mHfBuf[0].speed/256%256);    buf[i++] = mHfdata.mHfBuf[0].speed%256;//速度值
			buf[i++] = (mHfdata.mHfBuf[0].dirspeed/256%256); buf[i++] = mHfdata.mHfBuf[0].dirspeed%256;//航向值
			
		if(mHfdata.mHfBuf[j].dirlatitude == 'S')
			buf[i-2] = buf[i-2]|0x40;
		else 
			buf[i-2] = buf[i-2]&0xbf;  //经度方向
		
		if(mHfdata.mHfBuf[j].dirlongitude == 'W')
			buf[i-2]= buf[i-2]|0x80;
		else 
			buf[i-2] = buf[i-2]&0x7f;//纬度方向
	}
	for(j=2;j<i;j++){
		sum ^=  buf[j];
	}
	buf[i++] = sum;//计算报文的校验和
	sum = 0;
	sendi = 0;//发送缓冲区
	/*发送信息头部值*/
	SendBuf[sendi ++ ] = '$'; SendBuf[sendi ++ ] = 'C'; SendBuf[sendi ++ ] = 'C';
	SendBuf[sendi ++ ] = 'T'; SendBuf[sendi ++ ] = 'X'; SendBuf[sendi ++ ] = 'A';
	SendBuf[sendi ++ ] = ',';
	addr = 455874;
	sprintf(address,"%07d",addr);	
	for(j=0;j<7;j++){
		SendBuf[sendi ++ ] = address[j];
	}
	SendBuf[sendi ++ ] = ',';
	SendBuf[sendi ++ ] = '1';SendBuf[sendi ++ ] = ','; // 1-普通
	SendBuf[sendi ++ ] = '2';SendBuf[sendi ++ ] = ','; // 2-混合传输 ]
	for(j=0;j<i;j++){//填写通信内容
		HextoStr(hex2str, buf[j]);
		SendBuf[sendi ++ ]	= hex2str[0];
		SendBuf[sendi ++ ]	= hex2str[1];
	}
	SendBuf[sendi ++ ] = '*';//计算检验值
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]	= hex2str[0];
	SendBuf[sendi ++ ]	= hex2str[1];  
	SendBuf[sendi ++ ]	= 0x0D;
	SendBuf[sendi ++ ]	= 0x0A; 
	
	writeComdata(SendBuf,sendi);
	printf("bdtoser -> %s \n", SendBuf);	
}
/*************发送读取参数信息*************************/
void sendQucs()
{
	char SendBuf[100] = {0};
	char hex2str[4] = {0},sum=0;
	int sendi = 0,j=0;
	sendi = sprintf(SendBuf,"$DQCS,1");
	SendBuf[sendi ++ ] = '*';//计算检验值
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]	= hex2str[0];
	SendBuf[sendi ++ ]	= hex2str[1];  
	SendBuf[sendi ++ ]	= 0x0D;
	SendBuf[sendi ++ ]	= 0x0A;  
	writeComdata(SendBuf,sendi); 
}


/************************************************************
** 函数名称:  sendBDIC
** 功能描述:  发送读卡指令
** 输入参数:  NULL
** 输出参数:  NULL
** 返 回 值:  NULL
** 作    者:  Donny
** 日    期:  2017年8月02日
**************************************************************/
void sendBDIC()
{
	char SendBuf[100] = {0};
	char hex2str[4] = {0},sum=0;
	int sendi = 0,j=0;
	sendi = sprintf(SendBuf,"$BDIC,1");
	SendBuf[sendi ++ ] = '*';//计算检验值
	for(j=1;j<sendi-1;j++)sum ^= SendBuf[j];
	HextoStr(hex2str, sum);
	SendBuf[sendi ++ ]  = hex2str[0];
	SendBuf[sendi ++ ]  = hex2str[1];  
	SendBuf[sendi ++ ]  = 0x0D;
	SendBuf[sendi ++ ]  = 0x0A;  
	writeComdata(SendBuf,sendi); 
}

