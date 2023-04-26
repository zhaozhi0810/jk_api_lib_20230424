/*
* @Author: dazhi
* @Date:   2022-07-27 09:57:14
* @Last Modified by:   dazhi
* @Last Modified time: 2022-07-27 10:19:48
*/


#ifndef __MCUUPDATE_IPC_SMGQ_H__
#define __MCUUPDATE_IPC_SMGQ_H__


//用户创建消息队列的关键字
#define MCUUPDATE_KEY_PATH "/proc/meminfo"
#define MCUUPDATE_KEY_ID 654321

//#define TYPE_SENDTO_API 234   //发    必须跟api是反的！！！！，不要随意改动！！！！
//#define TYPE_RECVFROM_API 678   //收





typedef struct  //消息结构体
{
    long types;    //指定消息的类型
    int cmd;
    char buf[128];   //存放文件名，绝对路径
}mcuupdate_msgq_t; 



enum MCUUPDATE_CMD_types{
	eMCUUPDATE_FILENAME = 0,   //文件名
	eMCUUPDATE_STATUS    //状态信息
};





//返回值0表示成功，其他表示失败
int mcuupdate_msgq_init(void);


//返回值0表示成功，其他表示失败
int mcuupdate_msgq_exit(void);

//接收消息
//参数 timeout_50ms 大于0时，表示非阻塞模式，等待的时间为timeout_50ms*50ms的值，等于0表示阻塞模式
//     types 接收消息的类型
//     msgbuf 接收消息的缓存，1次接收一条消息
//返回0表示成功，其他表示失败
int mcuupdate_msgq_recv(long types,mcuupdate_msgq_t *msgbuf,unsigned int timeout_50ms);



//发送消息，并且要等待应答。
////参数 timeout 大于0时，设置接收应答超时时间，等于0表示阻塞模式
//     ack_types 指定应答的类型，之前是TYPE_RECV+cmd
//     msgbuf 发送消息的缓存，1次发送一条消息
//返回0表示成功，其他表示失败
//注意，函数使用时，需要指定msgbuf->types ！！！！
int mcuupdate_msgq_send(long ack_types,mcuupdate_msgq_t *msgbuf,int timeout);


//发送应答消息消息，不等待应答。
int mcuupdate_msgq_send_ack(mcuupdate_msgq_t *msgbuf);



#endif


