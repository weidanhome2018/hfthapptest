2017.7.25    上报地址，上报协议改为海丰自己服务器
2017.8.16    修改4G 北斗上报时间不正确的问题


编译命令:
arm-fsl-linux-gnueabi-gcc -o main main.c serialcom.c sms4g.c bdgpsdec.c   vediov4l2.c    -lpthread   -lm
注意事项:
拷贝后记得卸载sd卡(umount)，否则可能会造成拷贝不成功的问题


上报地址：
	 航科院      GPRS服务器119.40.52.117 :9000  指挥机卡号 458735
         海丰服务器  GRPS服务器114.215.188.97:8745  指挥机卡号 455874