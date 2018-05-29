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
char Is4GVliad = 0;//0表示不可用  1表示可用
int LED4GTime = 0;	//4G灯传输次数
/*客户端*/

//海丰服务器地址
char *host_name ="114.215.188.97";//需要搜寻服务端IP地址
int port = 8745;

//航科院服务器地址
//char *host_name ="119.40.52.117";//需要搜寻服务端IP地址
//int port = 9000;

//调试服务器地址
//char *host_name ="39.108.63.92";//需要搜寻服务端IP地址
//int port = 1234;
int socket_descriptor; 

int socket_desUdp; 
int sendOK = 1;
unsigned int BDID = LOCALBD;//北斗卡号
unsigned int DeViceID = DEVICEID;

/*******初始化4G模块***************/
void sms4g_init()
{ 
//	printf("sms4g_init\n");
//	printf("Is4GVliad: %d\n",Is4GVliad);
	char buf[1024] = {0}; 
   
    struct sockaddr_in pin;//处理网络通信的地址
    /*
     * hostent记录主机的信息，包括主机名、别名、地址类型、地址长度和地址列表
     * struct hostent {
　　   char *h_name;地址的正式名称
　　   char **h_aliases;空字节-地址的预备名称的指针
　　   int h_addrtype;地址类型; 通常是AF_INET。
　     int h_length;地址的比特长度。
　     char **h_addr_list;零字节-主机网络地址指针。网络字节顺序。
　　   };
　　   #define h_addr h_addr_list[0] //h_addr_list中的第一地址
     */
    struct hostent *server_host_name; 
	int length;
	if(Is4GVliad == 1)
	{
//		printf("4G 1\n");
		//开启华为4G模块闪灯功能
		system("echo echo\"AT^LEDCTRL=1\" > /dev/ttyUSB2");
//		printf("4G 4\n");
		//原有tcp所要
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
     * gethostbyname()返回对应于给定主机名的包含主机名字和地址信息的
     * hostent结构指针。结构的声明与gethostaddr()中一致。
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
                          //htonl()将主机的无符号长整形数转换成网络字节顺序
    pin.sin_addr.s_addr=htonl(INADDR_ANY);//s_addr按照网络字节顺序存储IP地址
                      //in_addr 32位的IPv4地址  h_addr_list中的第一地址
    pin.sin_addr.s_addr=((struct in_addr *)(server_host_name->h_addr))->s_addr;// 跟书上不一样 必须是h_addr

    pin.sin_port=htons(port); */
    /*申请一个通信端口*/
    /* if((socket_descriptor =socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("Error opening socket \n");
		Is4GVliad = 0;
        return;
    } */
	//printf("opening socket Successed\n");
    //pin 定义跟服务端连接的 IP 端口
   /*  if(connect(socket_descriptor,(void *)&pin,sizeof(pin))==-1)
    {
        perror("Error connecting to socket \n");////
        Is4GVliad = 0;
        return;
    }   */
	bzero(&pin,sizeof(pin));  
    pin.sin_family=AF_INET;  
    pin.sin_addr.s_addr=inet_addr(host_name);//这里不一样  
    pin.sin_port=htons(port); 
    //创建一个 TCP socket  
    if((socket_descriptor =socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("Error opening socket \n");
		Is4GVliad = 0;
        return;
    } 
    //pin 定义跟服务端连接的 IP 端口
    if(connect(socket_descriptor,(void *)&pin,sizeof(pin))==-1)
    {
        perror("Error connecting to socket \n"); ////
        Is4GVliad = 0;
        return;
    }
	Is4GVliad = 1;
} 

/*******使用4G模块发送数据***************/
void sms4g_senddata(char *str,int length)
{ 
	/* struct sockaddr_in pin; 
	bzero(&pin,sizeof(pin));
    pin.sin_family =AF_INET;
                          //htonl()将主机的无符号长整形数转换成网络字节顺序
    pin.sin_addr.s_addr=inet_addr(host_name);//s_addr按照网络字节顺序存储IP地址
                      //in_addr 32位的IPv4地址  h_addr_list中的第一地址
    //pin.sin_addr.s_addr=((struct in_addr *)(server_host_name->h_addr))->s_addr;// 跟书上不一样 必须是h_addr

    pin.sin_port=htons(port);
	if(sendto(socket_desUdp,str,length,0,(struct sockaddr *)&pin,sizeof(pin)) == -1)
    {
        perror("Error in send\n");
        Is4GVliad = 0;
    } 
	printf("sending Successed\n"); */
}

/*******使用4G模块发送数据***************/
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
/***************发送登陆信息**********************/
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

/* {{{ 查找字符串首次出现的位置，没有找到返回 -1，两个字符串相等返回 0
   在GCC下使用C99：
   int strpos(const char *haystack,const char *needle, _Bool ignorecase = 0)
   _Bool ignorecase =1 忽略大小写
   时间：2012-08-17 By Dewei 
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
 

