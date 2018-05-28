#ifndef _BDGPSDEC_H
#define _BDGPSDEC_H

#include "hfdata.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include "serialcom.h"

extern struct BDGPSReC mBDGPSReC;
extern void Putbdgpsdata(char *buf,int length);//当收到下面消息使用该函数
extern void Lookgpsdata(void);
void sendQucs(void);
#endif