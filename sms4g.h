#ifndef __SMS4G_H
#define __SMS4G_H

void sms4g_init(void);

void sms4g_senddata(char *str,int length);

void sms4g_Udpdata(char *str,int length);

void SendLogin(void);

extern char Is4GVliad;
 
#endif