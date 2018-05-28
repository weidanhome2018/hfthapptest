#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define MPU9250_READ_CMD				0x01
#define MPU9250_WRITE_CMD				0x02

//--------------------9axis  reg addr-----------------------//
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40 

#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
		
#define MAG_XOUT_L		0x03
#define MAG_XOUT_H		0x04
#define MAG_YOUT_L		0x05
#define MAG_YOUT_H		0x06
#define MAG_ZOUT_L		0x07
#define MAG_ZOUT_H		0x08

// 定义MPU9250内部地址
/*****************************************************************/
#define	SMPLRT_DIV		                      0x19	//陀螺仪采样率
#define	CONFIG			                        0x1A	
#define	GYRO_CONFIG		                      0x1B	
#define	ACCEL_CONFIG	                      0x1C	
#define	ACCEL_CONFIG_2                      0x1D 

#define INT_PIN_CFG                         0x37 //中断配置
#define USER_CTRL                           0x6a
#define I2C_MST_CTRL                        0x24
#define I2C_MST_DELAY_CTRL                  0x67
//--------------------i2c slv0-------------------------------//
#define I2C_SLV0_ADDR                       0x25  
#define I2C_SLV0_REG                        0x26
#define I2C_SLV0_CTRL                       0x27 
#define I2C_SLV0_DO                         0x63 //output reg

//--------------------AK8963 reg addr------------------------//
#define MPU9250_AK8963_ADDR                 0x0C  //AKM addr
#define AK8963_WHOAMI_REG                   0x00  //AKM ID addr
#define AK8963_WHOAMI_ID                    0x48  //ID
#define AK8963_ST1_REG                      0x02  //Data Status1
#define AK8963_ST2_REG                      0x09  //Data reading end register & check Magnetic sensor overflow occurred
#define AK8963_ST1_DOR                      0x02
#define AK8963_ST1_DRDY                     0x01 //Data Ready
#define AK8963_ST2_BITM                     0x10
#define AK8963_ST2_HOFL                     0x08 // Magnetic sensor overflow 
#define AK8963_CNTL1_REG                    0x0A
#define AK8963_CNTL2_REG                    0x0B
#define AK8963_CNTL2_SRST                   0x01 //soft Reset
#define AK8963_ASAX                         0x10 //X-axis sensitivity adjustment value 
#define AK8963_ASAY                         0x11 //Y-axis sensitivity adjustment value
#define AK8963_ASAZ                         0x12 //Z-axis sensitivity adjustment value

#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)

typedef struct{
	short Accel[3];//Accel X,Y,Z
	short Gyro[3];//Gyro X,Y,Z
	short Mag[3];	//Mag X,Y,Z	
}MPU_value;

MPU_value mpu_value;          //9轴数据

unsigned char BUF[6];       //接收数据缓存区	

int fd; 

/*************读出地址数据********************************/
char ReadMPu9250data(int address){
	char rd_data[3]; 
    lseek(fd, address, SEEK_SET);
	read(fd,rd_data,1);
    return rd_data[0];
}

/*************写入地址数据********************************/
int WriteMPu9250data(int address,int value){
	char witedata[3];
	lseek(fd, 0, SEEK_SET); 
	witedata[0] = value;
    return  write(fd,witedata,address);
}

/***************初始化地址数据*************************************/
void InitMPu9250(){
	fd = open("/dev/mpu9250", O_RDWR);
	if (-1 == fd)
	{
        printf("Open %s -1...\n");
		return;
	} 
	WriteMPu9250data(PWR_MGMT_1, 0x00);	//解除休眠状态
	WriteMPu9250data(CONFIG, 0x07);      //低通滤波频率，典型值：0x07(3600Hz)此寄存器内决定Internal_Sample_Rate==8K
	WriteMPu9250data(SMPLRT_DIV, 0x07);  
	
	WriteMPu9250data(INT_PIN_CFG ,0x30);// INT Pin / Bypass Enable Configuration  
	WriteMPu9250data(I2C_MST_CTRL,0x4d);//I2C MAster mode and Speed 400 kHz
	WriteMPu9250data(USER_CTRL ,0x20); // I2C_MST _EN 
	WriteMPu9250data(I2C_MST_DELAY_CTRL ,0x01);//延时使能I2C_SLV0 _DLY_ enable 	
	WriteMPu9250data(I2C_SLV0_CTRL ,0x81); //enable IIC	and EXT_SENS_DATA==1 Byte
	
	//陀螺仪采样率，典型值：0x07(1kHz) (SAMPLE_RATE= Internal_Sample_Rate / (1 + SMPLRT_DIV) )
	WriteMPu9250data(GYRO_CONFIG, 0x18); //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
	WriteMPu9250data(ACCEL_CONFIG, 0x18);//加速计自检、测量范围及高通滤波频率，典型值：0x18(不自检，16G)
	WriteMPu9250data(ACCEL_CONFIG_2, 0x08);//加速计高通滤波频率 典型值 ：0x08  （1.13kHz）	
	
	/**********************Init MAG **********************************/
	WriteMPu9250data(AK8963_CNTL2_REG,AK8963_CNTL2_SRST); // Reset AK8963
	WriteMPu9250data(AK8963_CNTL1_REG,0x12); // use i2c to set AK8963 working on Continuous measurement mode1 & 16-bit output
}
//************************加速度读取**************************/
void READ_MPU9250_ACCEL(void)//
{  
   BUF[0]=ReadMPu9250data(ACCEL_XOUT_L); 
   BUF[1]=ReadMPu9250data(ACCEL_XOUT_H);
   mpu_value.Accel[0]=	(BUF[1]<<8)|BUF[0];
   mpu_value.Accel[0]/=164; 						   //读取计算X轴数据
   BUF[2]=ReadMPu9250data(ACCEL_YOUT_L);
   BUF[3]=ReadMPu9250data(ACCEL_YOUT_H);
   mpu_value.Accel[1]=	(BUF[3]<<8)|BUF[2];
   mpu_value.Accel[1]/=164; 						   //读取计算Y轴数据
   BUF[4]=ReadMPu9250data(ACCEL_ZOUT_L); 
   BUF[5]=ReadMPu9250data(ACCEL_ZOUT_H);
   mpu_value.Accel[2]=  (BUF[5]<<8)|BUF[4];
   mpu_value.Accel[2]/=164; 					      //读取计算Z轴数据 
}
/**********************陀螺仪读取*****************************/
void READ_MPU9250_GYRO(void)
{ 
   BUF[0]=ReadMPu9250data(GYRO_XOUT_L); 
   BUF[1]=ReadMPu9250data(GYRO_XOUT_H);
   mpu_value.Gyro[0]=	BUF[1];
   mpu_value.Gyro[0]=	(mpu_value.Gyro[0]<<8)|BUF[0];
   mpu_value.Gyro[0]/=16.4; 						   //读取计算X轴数据

   BUF[2]=ReadMPu9250data(GYRO_YOUT_L);
   BUF[3]=ReadMPu9250data(GYRO_YOUT_H);
   mpu_value.Gyro[1]=	BUF[3];
   mpu_value.Gyro[1]=	(mpu_value.Gyro[1]<<8)|BUF[2];
   mpu_value.Gyro[1]/=16.4; 						   //读取计算Y轴数据
   BUF[4]=ReadMPu9250data(GYRO_ZOUT_L);
   BUF[5]=ReadMPu9250data(GYRO_ZOUT_H);
   mpu_value.Gyro[2]=	BUF[5];
   mpu_value.Gyro[2]=	(mpu_value.Gyro[2]<<8)|BUF[4];
   mpu_value.Gyro[2]/=16.4; 					       //读取计算Z轴数据
}
/**********************磁力计读取***************************/
//ReadMPu9250data(AK8963_ST2_REG) 此步读取不可省略
//数据读取结束寄存器，reading this register means data reading end
//AK8963_ST2_REG 同时具有数据非正常溢出检测功能
//详情参考 MPU9250 PDF
/**********************************************************/
void READ_MPU9250_MAG(void)
{ 	
	unsigned char x_axis,y_axis,z_axis; 
	
	x_axis=ReadMPu9250data(AK8963_ASAX);// X轴灵敏度调整值
	y_axis=ReadMPu9250data(AK8963_ASAY);
	z_axis=ReadMPu9250data(AK8963_ASAZ);
	
	if((ReadMPu9250data(AK8963_ST1_REG)&AK8963_ST1_DOR)==0)//data ready
	{
		//读取计算X轴数据
		BUF[0]=ReadMPu9250data(MAG_XOUT_L); //Low data	 
		BUF[1]=ReadMPu9250data(MAG_XOUT_H); //High data	 
		mpu_value.Mag[0]=((BUF[1]<<8)|BUF[0])*(((x_axis-128)>>8)+1);		//灵敏度纠正 公式见/RM-MPU-9250A-00 PDF/ 5.13	
		 
		//读取计算Y轴数据
		BUF[2]=ReadMPu9250data(MAG_YOUT_L); //Low data	 	 
		BUF[3]=ReadMPu9250data(MAG_YOUT_H); //High data	 
		mpu_value.Mag[1]=((BUF[3]<<8)|BUF[2])*(((y_axis-128)>>8)+1);	
		 
		//读取计算Z轴数据
		BUF[4]=ReadMPu9250data(MAG_ZOUT_L); //Low data	  
		BUF[5]=ReadMPu9250data(MAG_ZOUT_H); //High data	 
		mpu_value.Mag[2]=((BUF[5]<<8)|BUF[4])*(((z_axis-128)>>8)+1);	
	}					       
}

int main(int argc, char **argv)
{
	InitMPu9250();
	while(1){
		sleep(1);
		READ_MPU9250_ACCEL();
		READ_MPU9250_GYRO();
		READ_MPU9250_MAG(); 
		printf("Accel %d-%d-%d\n",mpu_value.Accel[0],mpu_value.Accel[1],mpu_value.Accel[2]);
		printf("Gyro %d-%d-%d\n",mpu_value.Gyro[0],mpu_value.Gyro[1],mpu_value.Gyro[2]);
		printf("Mag %d-%d-%d\n",mpu_value.Mag[0],mpu_value.Mag[1],mpu_value.Mag[2]);
	}	
    close(fd);
	return 0;
}


