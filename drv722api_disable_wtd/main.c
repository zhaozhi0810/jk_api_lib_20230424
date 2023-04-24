/*
* @Author: dazhi
* @Date:   2022-10-10 16:12:38
* @Last Modified by:   dazhi
* @Last Modified time: 2022-10-10 16:21:54
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include "drv_22134.h"
#include <utmp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>



typedef struct  //消息结构体
{
    long types;    //指定消息的类型
    int cmd;       //消息的命令
    int param1;    //参数1
    int param2;    //参数2
    int ret;       //参数3，可用于返回值
}msgq_t; 

//用户创建消息队列的关键字
#define KEY_PATH "/proc/cpuinfo"
#define KEY_ID -123321

#define eAPI_HWTD_SETONOFF_CMD 10
#define TYPE_API_SENDTO_SERVER 678   //发 改到Jc_msgq.c中去了
#define TYPE_API_RECFROM_SERVER 234

static int msgid = -1;  //消息队列的描述符


static int MsgClear(void)
{
	msgq_t msgbuf;

	if(msgid == -1)
		return -EPERM;

	while( msgrcv(msgid, &msgbuf, sizeof(msgbuf)-sizeof(long),0,IPC_NOWAIT) >0) ;  //关闭的时候把所有的消息都清除掉
    
	return 0;
}



//返回值0表示成功，其他表示失败
static int Jc_msgq_init(void)
{
	key_t key;
    
    key = ftok(KEY_PATH,KEY_ID);  //获取key值
    if(key < 0)
    {
        printf("ftok fail \n");
        fflush(stdout);
        return (-EAGAIN);
    }
    // 创建消息队列，如果消息队列存在，errno 会提示 eexist
    // 错误，此时只需要直接打开消息队列即可
    msgid = msgget(key,IPC_CREAT|IPC_EXCL|0666);
    if(msgid < 0)
    {
        if(errno == EEXIST) //文件存在错误提示
        {
            msgid = msgget(key,0666);//打开消息队列
            MsgClear();
        }
        else //其他错误退出
        {
            printf("msgget fail \n");
            return(-EAGAIN);
        }
    }
    //printf("msgget success!! msgid = %d\n",msgid);
    return 0;
}


//接收消息
//参数 timeout_50ms 大于0时，表示非阻塞模式，等待的时间为timeout_50ms*50ms的值，等于0表示阻塞模式
//     types 接收消息的类型
//     msgbuf 接收消息的缓存，1次接收一条消息
//返回0表示成功，其他表示失败
static int Jc_msgq_recv(long types,msgq_t *msgbuf,unsigned int timeout_50ms)
{
	int recv_flg = 0;	
	int ret;
//	int retry = 0;

	if(msgbuf == NULL){
		printf("recv error,msgbuf == NULL!!!\n");
		return -EPERM;
	}

	if(msgid == -1) //未初始化
	{
		printf("recv error,msgq_init first!!!\n");
		fflush(stdout);
		return -EPERM;
	}

	if(timeout_50ms > 0)
		recv_flg = IPC_NOWAIT;

    //接收消息
    do{
	    ret = msgrcv(msgid, msgbuf, sizeof(msgq_t)-sizeof(long),types,recv_flg);  //防止死循环
	    if(ret >= 0)
	    	break;
	    else if(0 == recv_flg) //阻塞堵塞模式出问题
	    {
			if(errno == EINTR)
				continue;
			return -EINTR;
		}
	    else if(errno != ENOMSG)
	    {
	    	printf("msgrcv\n");
	    	fflush(stdout);
	    	return -ENOMSG;
	    }	

	    usleep(50000);   //休眠50ms	   
    }while(--timeout_50ms);
    
    if(recv_flg && (0 == timeout_50ms))   //超时,可能设置为等待模式
    {
    	printf("ERROR:msg recv wait timeout,recv type = %ld!!!\n",types);
		fflush(stdout);		
    	return -EAGAIN;
    }
    
//    printf("type = %ld cmd = %d b = %d c = %d rt = %d\n",msgbuf->types,msgbuf->cmd,msgbuf->b,msgbuf->c,msgbuf->ret);
//	fflush(stdout);
	return 0;
}




//发送消息，并且要等待应答。
////参数 timeout 大于0时，设置接收应答超时时间（50ms的整数倍），等于0表示阻塞模式
//     ack_types 指定应答的类型，之前是TYPE_RECV+cmd
//     msgbuf 发送消息的缓存，1次发送一条消息
//返回0表示成功，其他表示失败
//注意，函数使用时，需要指定msgbuf->types ！！！！
static int Jc_msgq_send(long ack_types,msgq_t *msgbuf,int timeout)
{
	int ret = -EBUSY;  //初始值
//	int cmd = msgbuf->cmd;

	if(msgbuf == NULL){
		printf("send error,msgbuf == NULL!!!\n");
		return -EPERM;
	}

	if(msgid == -1) //未初始化
	{
		printf("send error,msgq_init first!!!\n");
		fflush(stdout);
		return -EPERM;
	}

	//msgbuf->types = TYPE_SEND; //给消息结构赋值，发送的类型是固定的TYPE_SEND
	//printf("debug: send type = %ld cmd = %d b = %d c = %d rt = %d\n",msgbuf->types,msgbuf->cmd,msgbuf->param1,msgbuf->param2,msgbuf->ret);
    //发送消息
    if(msgsnd(msgid, msgbuf, sizeof(msgq_t)-sizeof(long),0) == 0)
    {
    //	printf("debug:msgsnd ok \n");
    	//等待应答，等待的类型跟命令有关！！！！！
    	ret =  Jc_msgq_recv(ack_types,msgbuf,timeout);
    	if(ret == 0)    //正常在这返回0，仍然可能不是0. 
    	{
    //		printf("msgq_send and recv ok!!msgbuf.ret = %d\n",msgbuf->ret);
    		//用param1返回数据
			return 0;
		}
    }  
    printf("msgsnd error!\n"); //这条路应该还是有问题的。打印一下错误提示
    fflush(stdout);
    return ret;   // 返回错误值
}




//api发送数据给服务器，并且等待服务器应答，超时时间1s
//第二个参数可以用于返回数据，无数据时可以为NULL
static int api_send_and_waitack(int cmd,int param1,int *param2)
{
	msgq_t msgbuf;  //用于应答
	int ret;	
	msgbuf.types = TYPE_API_SENDTO_SERVER;  //发送的信息类型
	msgbuf.cmd = cmd;      //结构体赋值
	msgbuf.param1 = param1;
//	printf("API DEBUG: param1 = %d msgbuf.param1 = %d\n",param1,msgbuf.param1);
	if(param2)
		msgbuf.param2 = *param2;
	else //空指针
		msgbuf.param2 = 0;
	msgbuf.ret = 0;   //一般没有使用
//	printf("DEBUG: cmd = %d\n",cmd);
	//3.做出应答
	ret = Jc_msgq_send(TYPE_API_RECFROM_SERVER+cmd,&msgbuf,20); //数据发出后，需要等待 20表示1s
	if(0!= ret)
	{		
		printf("error : Jc_msgq_send ,ret = %d\n",ret);
		return ret;
	} 
//	printf("DEBUG: Jc_msgq_send ok\n");
	//判断是否有返回数据
	if(param2 && (msgbuf.ret>0))  //ret >0 表示有数据回来,小于0表示出错，等于0表示应答
		*param2 = msgbuf.param1;   //用param1返回数据

	return 0;	
}

//3. 去使能看门狗函数
//返回值
//0：设置成功
//1：设置失败
static int drvWatchDogDisable(void)
{
	int param = 0;

	if(api_send_and_waitack(eAPI_HWTD_SETONOFF_CMD,param,&param))  //发送的第2个参数0 表示禁止
	{
		printf("error : drvWatchDogDisable\n");
		return 1;
	}

	return 0;
}


int main(void)
{
	printf("关闭看门狗\n");
	Jc_msgq_init();
	
	if(drvWatchDogDisable()==0)
		printf("执行成功，看门狗被关闭\n");
	else
		printf("ERROR:看门狗关闭失败\n");
}


