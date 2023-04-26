/*
*********************************************************************************************************
*
*	Ä£¿éÃû³Æ : Ö÷º¯Êý
*	ÎÄ¼þÃû³Æ : main.c
*	°æ    ±¾ : V1.0
*	Ëµ    Ã÷ : Ö÷¹¦ÄÜ
*
*	ÐÞ¸Ä¼ÇÂ¼ 
*		°æ±¾ºÅ    ÈÕÆÚ         ×÷Õß      ËµÃ÷
*		V1.0    2023-04-06     dazhi    Ê×·¢
*
*	Copyright (C), 2022-2030, htjc by dazhi
*
*
*    aarch64-linux-gnu-gcc *.c -o xyzmodem_send
*********************************************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "../kmUtil_server/uart_to_mcu.h"
#include "xyzmodem.h"
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
 /* Ô´ÎÄ¼þÂ·¾¶ */
//char SourceFile[] = "./app.bin";    
static const char* my_opt = "f:";

//70.需要server关闭串口，防止升级失败
//val: 0 表示临时关闭，1表示开启
int drvControlttyS0(int val);  //因为要升级单片机程序，需要临时关闭串口。








//ps -ef | grep drv722_22134_server | grep -v grep | wc -l
//尝试启动server进程
//返回0表示没有该进程，1表示存在进程了,-1表示出错
static int is_server_process_start(char * cmd_name)
{
	FILE *ptr = NULL;
	char cmd[256] = "ps -ef | grep %s | grep -v grep | wc -l";
//	int status = 0;
	char buf[64];
	int count;

//	printf("server DEBUG:cmd_name = %s\n",cmd_name);
	snprintf(cmd,sizeof cmd,"ps -ef | grep %s | grep -v grep | wc -l",cmd_name);
	printf("ServerDEBUG: api check serverProcess is running, cmd = %s\n",cmd);

	if((ptr = popen(cmd, "r")) == NULL)
	{
		printf("is_server_process_start popen err\n");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数
	{
//		printf("server: buf = %s\n",buf);
		count = atoi(buf);
		if(count < 1)//当进程数小于等于2时，说明进程不存在, 1表示有一个，是grep 进程的
		{
			pclose(ptr);
			printf("ServerDEBUG: check serverProcess: no server process,ready to start serverProcess!!\n");
			return 0;  //系统中没有该进程	
		}
		
		printf("ServerDEBUG: check serverProcess: server process is running!!\n");
	}
	pclose(ptr);
	return 1;
}






// int main_mcu_update(int argc,char* argv[]) 
// {
// 	unsigned char* filename = (unsigned char*)"./app.bin";
	

    

//     return 0;
// }


