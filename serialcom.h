#ifndef __SERIALCOM_H
#define __SERIALCOM_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include "hfdata.h"

#define  DATA_LEN   100
#define  BAUDRATE   115200
#define  DATABITS   '8'
#define  PARITY     'n'
#define  STOPBITS   '1'

void serialcom_init(void);
extern int writeComdata(char *buf,int length);

#endif