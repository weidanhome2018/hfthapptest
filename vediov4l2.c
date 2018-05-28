#include "vediov4l2.h"

/*
 * Copyright 2004-2016 Freescale Semiconductor, Inc. All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*
 * @file mxc_v4l2_capture.c
 *
 * @brief Mxc Video For Linux 2 driver test application
 *
 */

#ifdef __cplusplus
extern "C"{
#endif

/*=======================================================================
                                        INCLUDE FILES
=======================================================================*/
/* Standard Include Files */
#include <errno.h>

/* Verification Test Environment Include Files */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <linux/videodev2.h>
//#include <linux/mxc_v4l2.h>
#include <sys/mman.h>
#include <string.h>
#include <malloc.h>
//#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <linux/videodev2.h>
#include <dirent.h>   

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
 
#include "h264encoder.h" 
 

int readFileList(char *basePath,char *dirname,int type)
{
    DIR *dir;
    struct dirent *ptr;
    char base[100];
	char Number[100] = {0},numbermidbuf[10] = {0},deletname[100]; 	
	int count = 0,length,lengthdir,countdir=0,nc=0;
	int mincount = 0,maxcount = 0,i,midcount = 0;
	
    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }
	
	memset(base,'\0',100);
	memset(dirname,'\0',100);
	length = sprintf(dirname,"Vedio%01d_",type);
	
	
    while ((ptr=readdir(dir)) != NULL)
    { 
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;  
		if(strncmp(ptr->d_name,dirname,length) == 0){ 
			
			memset(Number,'\0',100);
			memset(Number,'\0',10);
			lengthdir = sprintf(Number,"%s",ptr->d_name); 
			nc = 0;
			countdir = 0;
			for(i=0;i<lengthdir;i++){
				if(countdir == 1)numbermidbuf[nc++] = Number[i];
				if((Number[i] == '_') || (Number[i] == '.')){
					countdir++;
					numbermidbuf[nc-1] = 0;
					if(countdir == 2)break;
				}
			}  
			midcount = atoi(numbermidbuf); 
			if(count == 0)mincount = atoi(numbermidbuf);
			if(midcount < mincount)mincount = midcount;
			if(midcount > maxcount)maxcount = midcount;
			count++;	
		}  		
    }
	memset(dirname,'\0',100);
	sprintf(dirname,"%sVedio%01d_%d.h264\0",basePath,type,maxcount+1);
	sprintf(deletname,"%sVedio%01d_%d.h264\0",basePath,type,mincount);
	printf("deletname: --------------------------------- %s\n",deletname);
	if(count > 100)remove(deletname);
    closedir(dir);
    return 1;
} 
 
void Vedio0_do(void)
{   
	char h264_file_name[100] = ""; 

	char Commdbuf[100] = "";
	 
	readFileList("/media/",h264_file_name,0);
	 
	sprintf(Commdbuf,"./mxc_vpu.out  -E \"-x 0 -o %s   -f 2 -w 640 -h 480  & \"\n\r",h264_file_name);
	
	printf("Commdbuf: --------------------------------- %s\n",Commdbuf); 
	
	system(Commdbuf);
}

void Vedio1_do(void)
{ 
	char h264_file_name[100] = ""; 

	char Commdbuf[100] = "";
	 
	readFileList("/media/",h264_file_name,1);
	 
	sprintf(Commdbuf,"./mxc_vpu.out  -E \"-x 1 -o %s   -f 2 -w 640 -h 480  & \"\n\r",h264_file_name);
	
	printf("Commdbuf: --------------------------------- %s\n",Commdbuf); 
	
	system(Commdbuf);
}