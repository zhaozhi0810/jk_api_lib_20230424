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
#include "mcuupdate_ipc_msgq.h"
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
 /* Ô´ÎÄ¼þÂ·¾¶ */
//char SourceFile[] = "./app.bin";    
static const char* my_opt = "f:";



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






int main(int argc,char* argv[]) 
{
	char* filename = "./app.bin";
	int get_name = 0,c;

	mcuupdate_msgq_t msgbuf; 

    if(argc != 1)
	{	
	    while(1)
	    {
	        c = getopt(argc, argv, my_opt);
	        //printf("optind: %d\n", optind);
	        if (c < 0)
	        {
	            break;
	        }
	        //printf("option char: %x %c\n", c, c);
	        switch(c)
	        {
	        	case 'f':
	        		filename = optarg;
	        		get_name = 1;
	                //debug_level = atoi(optarg);
	                printf("filename = %s\n",filename);
	                break;	       
	       	 	default:	       
	                break;
	                //return 0;
	        }
	        if(get_name)  //Ìø³ö´óÑ­»·
	        	break;
	    }
	}


	if(mcuupdate_msgq_init())
	{
		printf("ERROR: mcuupdate_msgq_init!!\n");
		return -1;
	}
	memset(&msgbuf,0,sizeof(msgbuf));
	msgbuf.types = TYPE_SENDTO_SERVER;
	msgbuf.cmd = eMCUUPDATE_FILENAME;
	strcpy(msgbuf.buf,filename);

	if(1 == is_server_process_start("drv722_22134_server"))  //存在server进程
	{
		printf("drv722_22134_server is running!!\n");

		if(0 == mcuupdate_msgq_send(TYPE_RECVFROM_SERVER,&msgbuf,1000))  //1000,最长等待50s
		{
			printf("mcuupdate_msgq_send success!! msgbuf.buf = %s\n",msgbuf.buf);
		}
		else
		{
			printf("mcuupdate_msgq_send failed!! msgbuf.buf = %s\n",msgbuf.buf);
		}

	}
	else
	{
		printf("ERROR: drv722_22134_server is not running!!\n");
		return -1;
	}	



    return 0;
}


