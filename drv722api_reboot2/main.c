/*
* @Author: dazhi
* @Date:   2022-10-10 16:12:38
* @Last Modified by:   dazhi
* @Last Modified time: 2023-05-10 19:40:10
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "drv_22134_api.h"
#include <utmp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>


//aarch64-linux-gnu-gcc main.c  -o rk3399_22134_reboot

static int my_system(char* cmd)

{

	pid_t fpid; //fpid表示fork函数返回的值



	if(cmd == NULL)  //空指针

 		return -1;



 	int ret = access(cmd, F_OK | X_OK);  //文件存在并且可执行吗？

 	if(ret)  //返回0成功，-1失败

 	{

 		printf("access %s return -1\n",cmd);

 		return -1;

 	}



    fpid = fork();

    if (fpid < 0)

    {    

    	printf("error in fork!");

    	return -1;

	}

    else if (fpid == 0) {

       execl(cmd, cmd, NULL);

       return -1;

    } 

    //else {   //父进程

    return 0;

}





//ps -ef | grep drv_22134_server | grep -v grep | wc -l

//尝试启动server进程

//返回0表示server进程已经存在，-1表示出错

static int start_server_process(void)

{

	FILE *ptr = NULL;

	char cmd[] = "ps -ef | grep drv722_22134_server | grep -v grep | wc -l";

//	int status = 0;

	char buf[64];

	int count;



	if((ptr = popen(cmd, "r")) == NULL)

	{

		printf("popen err\n");

		return -1;

	}

	memset(buf, 0, sizeof(buf));

	if((fgets(buf, sizeof(buf),ptr))!= NULL)//获取进程和子进程的总数

	{

		count = atoi(buf);

		if(count <= 0)//当进程数小于等于0时，说明进程不存在

		{

			my_system("/root/drv722_22134_server");    //system 会启动两个进程，会导致判断出现一点问题

			printf("api start drv722_22134_server \n");

			usleep(500000);  //等待一下

		}

	}

	pclose(ptr);

	return 0;

}


int main(int argc,char*argv[])
{
	//确保server进程存在！！
	start_server_process();

 	if(0 == drvCoreBoardInit())
 		printf("enter API exe reboot2023\n");

	//drvIfBrdReset(void)
	drvDisableLcdScreen();
	printf("单片机执行成功，系统即将重启 \n");
//		execlp("systemctl","systemctl", "poweroff", NULL);   

	execvp("systemctl",argv);  		  
//	system("systemctl reboot");  //改为系统重启命令
}


