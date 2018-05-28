#include "serialcom.h"
#include "bdgpsdec.h" 
#include "hfdata.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> 
//#include "vediov4l2.h" 
#include "sms4g.h" 
//#include "mpl.h"

sem_t GPSBD_sem;//���ݲ������ź����ź���  
/****************************************************/
/****************�����������Ҫ�ǽ���4Gģ��Ĳ���****/
/****************************************************/
void *GMS_thread(void *str)  
{  
	
    while(1)
    {  
        sleep(1);  
		sms4g_init();
    }  
    return NULL;  
} 
/****************************************************/
/****************�����������Ǵ���ģ��Ĳ���**********/
/****************************************************/
void *Serial_thread(void *str)  
{  
    serialcom_init(); 
    return NULL;  
} 
/****************************************************/
/*******�����������Ҫ�ǽ������ݴ���Ĳ���***********/
/****************************************************/
void *GPSBD_thread(void *str)  
{  
    while(1)
    {     
        sem_wait(&GPSBD_sem); 
		Lookgpsdata(); 
    }  
    return NULL;  
} 
/****************************************************/
/*****�����������Ҫ�ǽ�����Ƶ���ݴ���Ĳ���*********/
/****************************************************/
/* void *VEDIO_thread(void *str)  
{     
    Vedio0_do();  
    return NULL;  
} 
void *VEDIO1_thread(void *str)  
{     
    Vedio1_do();  
    return NULL;  
}  */
/****************************************************/
/*****�����������Ҫ�ǽ�����ѹ�����ݴ���Ĳ���*******/
/****************************************************/
/* void *Mpl_thread(void *str)  
{     
    Mpldec();  
    return NULL;  
}   */
 
/****************************************************/
/****************�����������й���********************/
/****************************************************/  
int main()  
{  
    pthread_t GMS_thread_pth,Serial_thread_pth,GPSBD_thread_pth,VEDIO_thread_pth,VEDIO1_thread_pth,Mpl_thread_pth; //�����4Gģ��ĳ�ʼ�߳�  
    int GMS_ret, Serial_ret,GPSBD_ret,VEDIO_ret,VEDIO1_ret,Mpl_ret,i; 
	
	int res = sem_init(&GPSBD_sem, 0, 0); 
	system("mount /dev/mmcblk1p1  /media/"); 
    if(res == -1)  
    {  
        printf("GPSBD_sem failed\n");  
        //exit(EXIT_FAILURE);  
    }  
	GMS_ret = pthread_create(&GMS_thread_pth, NULL, GMS_thread, NULL);  
	Serial_ret = pthread_create(&Serial_thread_pth, NULL, Serial_thread, NULL);  
	GPSBD_ret = pthread_create(&GPSBD_thread_pth, NULL, GPSBD_thread, NULL);
	//VEDIO_ret = pthread_create(&VEDIO_thread_pth, NULL, VEDIO_thread, NULL); 
	//VEDIO1_ret = pthread_create(&VEDIO1_thread_pth, NULL, VEDIO1_thread, NULL); 
	//Mpl_ret = pthread_create(&Mpl_thread_pth, NULL, Mpl_thread, NULL);
    //pthread_join(pth, NULL);  
	i=0; 
	while(1)  
    {   
		i++; 
		if(i%3 == 0)
		{
			sendBDIC();
			sleep(1);
			sendQucs();
//			printf("sendQucs %d\n",i);
		}
//		if(i%30 == 0){
//			SendLogin();
//		}
        sleep(1); 
    }   
    return 0;  
}  