#include "sms4g.h"   
#include <stdio.h>
#include <stdlib.h>
#include "hfdata.h" 
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int strpos(const char *haystack,const char *needle, _Bool ignorecase);
char Is4GVliad = 0;//0��ʾ������  1��ʾ����
int LED4GTime = 0;	//4G�ƴ������
/*�ͻ���*/

//�����������ַ
char *host_name ="114.215.188.97";//��Ҫ��Ѱ�����IP��ַ
int port = 8745;

//����Ժ��������ַ
//char *host_name ="119.40.52.117";//��Ҫ��Ѱ�����IP��ַ
//int port = 9000;

//���Է�������ַ
//char *host_name ="39.108.63.92";//��Ҫ��Ѱ�����IP��ַ
//int port = 1234;
int socket_descriptor; 

int socket_desUdp; 
int sendOK = 1;
unsigned int BDID = LOCALBD;//��������
unsigned int DeViceID = DEVICEID;

/*******��ʼ��4Gģ��***************/
void sms4g_init()
{ 
//	printf("sms4g_init\n");
//	printf("Is4GVliad: %d\n",Is4GVliad);
	char buf[1024] = {0}; 
   
    struct sockaddr_in pin;//��������ͨ�ŵĵ�ַ
    /*
     * hostent��¼��������Ϣ����������������������ַ���͡���ַ���Ⱥ͵�ַ�б�
     * struct hostent {
����   char *h_name;��ַ����ʽ����
����   char **h_aliases;���ֽ�-��ַ��Ԥ�����Ƶ�ָ��
����   int h_addrtype;��ַ����; ͨ����AF_INET��
��     int h_length;��ַ�ı��س��ȡ�
��     char **h_addr_list;���ֽ�-���������ַָ�롣�����ֽ�˳��
����   };
����   #define h_addr h_addr_list[0] //h_addr_list�еĵ�һ��ַ
     */
    struct hostent *server_host_name; 
	int length;
	if(Is4GVliad == 1)
	{
//		printf("4G 1\n");
		//������Ϊ4Gģ�����ƹ���
		system("echo echo\"AT^LEDCTRL=1\" > /dev/ttyUSB2");
//		printf("4G 4\n");
		//ԭ��tcp��Ҫ
		length = recv(socket_descriptor,buf,1024,0);
		if(length == -1)
		{
			Is4GVliad = 0;
			perror("Error resolving \n");
			close(socket_descriptor);
		}
		else if(length > 0)
		{
			printf("4G LED %s\n",buf);
		} 
		return;
//		printf("4G 3\n");
	}
	sleep(10);
	system("wvdial & ls"); 
    /*
     * gethostbyname()���ض�Ӧ�ڸ����������İ����������ֺ͵�ַ��Ϣ��
     * hostent�ṹָ�롣�ṹ��������gethostaddr()��һ�¡�
	 */
    if((server_host_name = gethostbyname(host_name))==0)
    {
        perror("Error resolving local host \n");
		Is4GVliad = 0;
		return;
    }
//	printf("resolving local host Successed\n");
    /* bzero(&pin,sizeof(pin));
    pin.sin_family =AF_INET;
                          //htonl()���������޷��ų�������ת���������ֽ�˳��
    pin.sin_addr.s_addr=htonl(INADDR_ANY);//s_addr���������ֽ�˳��洢IP��ַ
                      //in_addr 32λ��IPv4��ַ  h_addr_list�еĵ�һ��ַ
    pin.sin_addr.s_addr=((struct in_addr *)(server_host_name->h_addr))->s_addr;// �����ϲ�һ�� ������h_addr

    pin.sin_port=htons(port); */
    /*����һ��ͨ�Ŷ˿�*/
    /* if((socket_descriptor =socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("Error opening socket \n");
		Is4GVliad = 0;
        return;
    } */
	//printf("opening socket Successed\n");
    //pin �������������ӵ� IP �˿�
   /*  if(connect(socket_descriptor,(void *)&pin,sizeof(pin))==-1)
    {
        perror("Error connecting to socket \n");////
        Is4GVliad = 0;
        return;
    }   */
	bzero(&pin,sizeof(pin));  
    pin.sin_family=AF_INET;  
    pin.sin_addr.s_addr=inet_addr(host_name);//���ﲻһ��  
    pin.sin_port=htons(port); 
    //����һ�� TCP socket  
    if((socket_descriptor =socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("Error opening socket \n");
		Is4GVliad = 0;
        return;
    } 
    //pin �������������ӵ� IP �˿�
    if(connect(socket_descriptor,(void *)&pin,sizeof(pin))==-1)
    {
        perror("Error connecting to socket \n"); ////
        Is4GVliad = 0;
        return;
    }
	Is4GVliad = 1;
} 

/*******ʹ��4Gģ�鷢������***************/
void sms4g_senddata(char *str,int length)
{ 
	/* struct sockaddr_in pin; 
	bzero(&pin,sizeof(pin));
    pin.sin_family =AF_INET;
                          //htonl()���������޷��ų�������ת���������ֽ�˳��
    pin.sin_addr.s_addr=inet_addr(host_name);//s_addr���������ֽ�˳��洢IP��ַ
                      //in_addr 32λ��IPv4��ַ  h_addr_list�еĵ�һ��ַ
    //pin.sin_addr.s_addr=((struct in_addr *)(server_host_name->h_addr))->s_addr;// �����ϲ�һ�� ������h_addr

    pin.sin_port=htons(port);
	if(sendto(socket_desUdp,str,length,0,(struct sockaddr *)&pin,sizeof(pin)) == -1)
    {
        perror("Error in send\n");
        Is4GVliad = 0;
    } 
	printf("sending Successed\n"); */
}

/*******ʹ��4Gģ�鷢������***************/
void sms4g_Udpdata(char *str,int length)
{
	if(send(socket_descriptor,str,length,0) == -1)
    {
        perror("Error in send\n");
        Is4GVliad = 0;
    }
	printf("sending Successed\n");

}
extern unsigned int sendnumber;
/***************���͵�½��Ϣ**********************/
void SendLogin()
{
	char sendhead[200] = {0}; 
	char sendBuf[250] = {0};
	int length,i;
	char sum = 0;
	if(Is4GVliad == 0)return;
	length = sprintf(sendhead,"$NWLGN,%2x,%s",sendnumber,"13015751310");
	for(i=1;i<length;i++){
		sum^=sendhead[i];
	}
	length = sprintf(sendBuf,"%s*%02x\r\n",sendhead,sum);
//	printf("%s",sendBuf);
	sms4g_Udpdata(sendBuf,length);

}

/* {{{ �����ַ����״γ��ֵ�λ�ã�û���ҵ����� -1�������ַ�����ȷ��� 0
   ��GCC��ʹ��C99��
   int strpos(const char *haystack,const char *needle, _Bool ignorecase = 0)
   _Bool ignorecase =1 ���Դ�Сд
   ʱ�䣺2012-08-17 By Dewei 
*/
int strpos(const char *haystack,const char *needle, _Bool ignorecase)  
{  
	register unsigned char c, needc;
	unsigned char const *from, *end;
	int i;
	int len = strlen(haystack);
	int needlen = strlen(needle);
	from = (unsigned char *)haystack;
	end = (unsigned char *)haystack + len;
	const char *findreset = needle;
	for (i = 0; from < end; ++i) 
	{
		c = *from++;
		needc = *needle;
		if (ignorecase) 
		{
			if (c >= 65 && c < 97)
				c += 32;
			if (needc >= 65 && needc < 97)
				needc += 32;
		}
		if(c == needc) 
		{
			++needle;
			if(*needle == '\0') 
			{
				if (len == needlen) 
					return 0;
				else
					return i - needlen+1;
			}
		}  
		else 
		{  
			if(*needle == '\0' && needlen > 0)
				return i - needlen +1;
			needle = findreset;  
		}
	}  
	return  -1;  
}  
/* }}} */
 

