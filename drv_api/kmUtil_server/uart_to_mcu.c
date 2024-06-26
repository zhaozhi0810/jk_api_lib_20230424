// main.c 
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/kd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>             // exit
#include <sys/ioctl.h>          // ioctl
#include <string.h>             // bzero
#include <pthread.h>
#include <semaphore.h>

#include <stdarg.h>

#include "ComFunc.h"
//#include "queue.h"
#include "uart_to_mcu.h"
#include "uinput_dev.h"
#include "uart_ipc_msgq.h"  //用于 串口程序内部通信的！！


//#include "Common.h"

/*global defines*/
//static QUEUE keyCmdQueue;
//static QUEUE mouseCmdQueue;

extern int server_in_debug_mode;   //服务端进入调试模式	



//static 	int p_opt = 0;   //打印调试信息，默认不打印
static 	int uart_fd = -1;

static volatile	unsigned short uart_recv_flag = 0;   //串口收到单片机数据


//#define FRAME_LENGHT (sizeof(com_frame_t)+1)    //Êý¾ÝÖ¡µÄ×Ö½ÚÊý
//#define FRAME_HEAD 0xa5   //×¢ÒâÓëµ¥Æ¬»ú±£³ÖÒ»ÖÂ



#define COM_DATLEN 4 //串口数据帧长度 0x5f + dat1 + dat2 + checksum
#define FRAME_HEAD 0xa5
//static unsigned char com_recv_data[COM_DATLEN*2];   //½ÓÊÕ»º´æ



// QQUEUE get_key_cmd_queue( void )
// {
// 	return (QQUEUE)&keyCmdQueue;
// }

// QQUEUE get_mouse_cmd_queue( void )
// {
// 	return (QQUEUE)&mouseCmdQueue;
// }





//串口数据接收处理
static void com_message_handle(unsigned char* com_recv_data)
{		
	uart_msgq_t msgbuf;
	if(com_recv_data[1]>0 && com_recv_data[1] < 37)   //按键上报
	{
		printf("key = %d %s\n",com_recv_data[1],com_recv_data[2]?"press":"release");
		send_a_button_ievent(com_recv_data[1]-1, com_recv_data[2]);
	}
	else  //除了按键之外的其他数据的处理
	{
		switch(com_recv_data[1])
		{
			case eMCU_LED_SETON_TYPE: //ÉèÖÃled ON			
			case eMCU_LED_SETOFF_TYPE: //ÉèÖÃled OFF		
			case eMCU_LCD_SETONOFF_TYPE:  //ÉèÖÃlcd ´ò¿ª»òÕß¹Ø±Õ		
			case eMCU_LEDSETALL_TYPE:  //ÉèÖÃËùÓÐµÄled ´ò¿ª»òÕß¹Ø±Õ			
			case eMCU_LEDSETPWM_TYPE:  //设置led的亮度
			case eMCU_GET_TEMP_TYPE:   //获得单片机的内部温度
			case eMCU_LED_STATUS_TYPE:		  //获得某个led的状态	
			case eMCU_HWTD_SETONOFF_TYPE:   //看门狗开启和关闭
			case eMCU_HWTD_FEED_TYPE:		//喂狗
			case eMCU_HWTD_SETTIMEOUT_TYPE:    //设置看门狗喂狗时间
			case eMCU_HWTD_GETTIMEOUT_TYPE:    //获取看门狗喂狗时间
			case eMCU_RESET_COREBOARD_TYPE:  //复位核心板
			case eMCU_RESET_LCD_TYPE:        //复位lcd 9211（复位引脚没有连通）
			case eMCU_RESET_LFBOARD_TYPE:    //复位底板，好像没有这个功能！！！
			case eMCU_MICCTRL_SETONOFF_TYPE:  //控制底板mic_ctrl引脚的电平
			case eMCU_LEDS_FLASH_TYPE:      //led键灯闪烁控制
			case eMCU_LSPK_SETONOFF_TYPE: //LSPK,2022-11-11 1.3新版增加
			case eMCU_V12_CTL_SETONOFF_TYPE:   //V12_CTL,2022-11-14 1.3新版增加
			case eMCU_GET_LCDTYPE_TYPE:  //,   上位机获得LCD类型的接口，之前是在3399，现在改为单片机实现，2022-12-12
			case eMCU_GET_MCUVERSION_TYPE:
				msgbuf.types = com_recv_data[1];  //这个是命令
				msgbuf.param = com_recv_data[2];  //这个是数据
				//uart_recv_flag = com_recv_data[1] | (com_recv_data[2] <<8);  //¸ß8Î»±íÊ¾×´Ì¬		
				Jc_uart_msgq_send_nowait(&msgbuf);  //把消息发出去
				break;
			default:
				//uart_recv_flag = 0;
				printf("ERROR:unknown uart recv\n");
				printf("com_recv_data[1] = %d com_recv_data[2] = %d\n",com_recv_data[1],com_recv_data[2]);
			break;
		}
		//调试信息
	//	printf("com_recv_data[1] = %d com_recv_data[2] = %d\n",com_recv_data[1],com_recv_data[2]);
	}
}


static unsigned char checksum(unsigned char *buf, unsigned char len)
{
	unsigned char sum;
	unsigned char i;

	for(i=0,sum=0; i<len; i++)
		sum += buf[i];
#ifdef PRINT_DEBUG
	printf("check sum = %x\n",sum);
#endif
	return sum;
}



//返回非0则错误，0则表示校验正确
static unsigned int verify_data(unsigned char *data,unsigned char len)
{
	unsigned char check;
	unsigned int ret = 1;
#ifdef PRINT_DEBUG
	int i;
#endif	
	if(data == NULL)
		return -1;
#ifdef PRINT_DEBUG	
	for(i = 0;i<len;i++)
		printf("data[%d] = %x \n",i,data[i]);
	printf("\n\n");
#endif	
	//¶ÁÈ¡Ô­Êý¾ÝÖÐµÄÐ£ÑéÖµ
	check = data[len - 1];
	
	//ÖØÐÂ¼ÆËãÐ£ÑéÖµ
	if(check == checksum(data,len - 1))
		ret = 0;
	
	return ret;
}




#if 0
static void ProcRecvKeyCmd(void)
{
	unsigned char length,i,j;
	static unsigned char datalen =COM_DATLEN,offset = 0;  //数据长度和缓存的偏移值

	while(1)  //这里不代表是死循环，考虑的是可能需要多处理几帧数据
	{		
		length = queue_length(&keyCmdQueue);	
		if(length < datalen)   //数据长度不够帧长
		{	
			return ;   //直接退出
		}	
		length = datalen;   //数据长度只要一帧的长度
		for(i=0;i<length;i++)
		{
			//从缓存中把数据读出来
			out_queue(&keyCmdQueue,com_recv_data+i+offset) ;  //读出来的同时，把缓存中的数据删除
			if(p_opt) //打印标志，非0则打印
				printf("com_recv_data[%d] = %x \n",i,com_recv_data[i+offset]);
		}
	//	com3_data[0] = FRAME_HEAD;  //¼ÓÈëÖ¡Í·½øÐÐÐ£ÑéºÍ¼ÆËã
		if((com_recv_data[0] == FRAME_HEAD) && (0 == verify_data(com_recv_data,COM_DATLEN)))   //µÚ¶þ²ÎÊýÊÇÊý¾Ý×Ü³¤¶È£¬°üÀ¨Ð£ÑéºÍ¹²7¸ö×Ö½Ú
		{
			//校验成功，处理接收的数据
			com_message_handle();		
		}	
		else  //检验失败，处理
		{
			for(i=1;i<COM_DATLEN;i++)   //在剩余的数据中找帧头
			{
				if(com_recv_data[i] == FRAME_HEAD)   //找到帧头
				{
					break;
				}
			}		
			if(i != COM_DATLEN) //在数据中找到帧头
			{
				datalen = i;   //下次需要读取数据的长度（i表示缺了几个字节）
				offset = COM_DATLEN-i;  //下次读取数据时，存储的偏移地址

				for(j=0;i<COM_DATLEN;i++,j++)   //将帧头和帧头之后的数据拷贝到缓存的开始位置
				{
					com_recv_data[j] = com_recv_data[i];   //拷贝数据
				}
			}
			else  //没有找到帧头
			{
				datalen = COM_DATLEN;  //下次需要读取数据的长度
				offset = 0;  //下次读取数据时，存储的偏移地址
			}
		}	
	}//end while 1
}
#endif


extern pthread_mutex_t mcu_update_mutex;




//串口接收数据处理线程
void* mcu_recvSerial_thread(void*arg)
{
	int i=0,j,readn=0; //
	unsigned char rBuf[COM_DATLEN*2]={0};
	unsigned char datalen =COM_DATLEN,offset = 0;  //数据长度和缓存的偏移值
	if(server_in_debug_mode)
    	printf("enter mcu_recvSerial_thread\n");              

	while(1)
	{	
		readn = PortRecv(uart_fd, rBuf+offset,datalen,1000);  //从串口读取数据，COM_DATLEN表示需要读的字节数
		if(server_in_debug_mode){

			printf("mcu_recvSerial_thread readn = %d\n",readn);
			//if(p_opt)  //打印标志，非0则打印
			printf("mcu_recvSerial_thread recive printf \n");
		}

		if(readn > 0)
		{
#if 0
			for (i=0; i<readn; i++)
			{
				if(p_opt)
					printf("rBuf[%d] = %x \n",i,rBuf[i]);
				if( 0 == add_queue( &keyCmdQueue, rBuf[i]))
				{
					printf("Error, cann't add queue to keyCmdQueue\n");	
				}
			
				//´¦Àí½ÓÊÕµÄÏûÏ¢
				ProcRecvKeyCmd();
			}	
#else
			if((rBuf[0] == FRAME_HEAD) && (0 == verify_data(rBuf,COM_DATLEN)))   //µÚ¶þ²ÎÊýÊÇÊý¾Ý×Ü³¤¶È£¬°üÀ¨Ð£ÑéºÍ¹²7¸ö×Ö½Ú
			{
				//校验成功，处理接收的数据
				com_message_handle(rBuf);		
			}	
			else  //检验失败，处理
			{
				for(i=1;i<COM_DATLEN;i++)   //在剩余的数据中找帧头
				{
					if(rBuf[i] == FRAME_HEAD)   //找到帧头
					{
						break;
					}
				}		
				if(i != COM_DATLEN) //在数据中找到帧头
				{
					datalen = i;   //下次需要读取数据的长度（i表示缺了几个字节）
					offset = COM_DATLEN-i;  //下次读取数据时，存储的偏移地址

					for(j=0;i<COM_DATLEN;i++,j++)   //将帧头和帧头之后的数据拷贝到缓存的开始位置
					{
						rBuf[j] = rBuf[i];   //拷贝数据
					}
				}
				else  //没有找到帧头
				{
					datalen = COM_DATLEN;  //下次需要读取数据的长度
					offset = 0;  //下次读取数据时，存储的偏移地址
				}
			
			}	
#endif	
		}
		else
			sleep(1);
		
	}//while(1)
	return NULL;
}



//¸ºÔð½ÓÊÕÊý¾ÝµÄÏß³Ì
/*
 * arg ´«³ö²ÎÊý£¬´®¿ÚÊÕµ½Êý¾Ý£¬ÒªÐÞ¸ÄÈ«¾Ö±äÁ¿£¬ËùÒÔargÖ¸ÏòÈ«¾ÖÊý¾Ý½á¹¹
 * 
 * ·µ»ØÖµ£º
 * 	¸ù¾ÝÏß³Ìº¯Êý¶¨Òå£¬ÎÞÒâÒå£¬¸Ãº¯Êý²»·µ»Ø£¡£¡£¡£¡
 * 
 * 
void* mcu_recvSerial_thread(void* arg)
{
	while(1)
	{
		info_recv_proc_func();
	}
}
*/




//
/*
 * 通过串口发送数据
 * 如果需要返回数据，从data【0】 读取
 * wait_time_50ms 表示等待50ms的整数倍【0-10000】
 * */
int send_mcu_data(const void* data,int isreply)//,unsigned int wait_time_50ms)
{	
	unsigned char buf[8];  	
//	int i;
	int ret;
	uart_msgq_t msgbuf;
	
	buf[0] = FRAME_HEAD;  //帧头	
	memcpy(buf+1,data,sizeof(com_frame_t)-1);    //数据中没有帧头

	buf[sizeof(com_frame_t)] = checksum(buf,sizeof(com_frame_t));  //计算校验和
	ret = PortSend(uart_fd, buf, sizeof(com_frame_t)+1) ;
	if(ret == 0)   //发送成功，等待应答	
	{
		if(!isreply) //不需要应答
		{
			return 0;
		}
	
		//data[0] 作为接收的类型
		if(Jc_uart_msgq_recv(((unsigned char*)data)[0],&msgbuf,20)==0)  //20表示1s
		{
			((unsigned char*)data)[0] = msgbuf.param;
			return 0;  //表示收到数据
		}
		printf("PortSend recv time out !!cmd = %d\n",((unsigned char*)data)[0]);
		return -1;
	}
	printf("Error, send_mcu_data PortSend failed ret= %d\n",ret);	
	return -1;
}




#if 0

static void show_version(char* name)
{
    printf( "%s Buildtime :"__DATE__" "__TIME__,name);
}

 

static void usage(char* name)
{
    show_version(name);
 
    printf("    -h,    short this help\n");
    printf("    -v,    show version\n");
    printf("    -d /dev/ttyS0, select com device\n");
    printf("    -p , printf recv data\n");
    printf("    -b , set baudrate\n");
    printf("    -n , set com nonblock mode\n");
    exit(0);
}
#endif


//static const char* my_opt = "Dvhpwb:d:";

/* This function will open the uInput device. Please make 
sure that you have inserted the uinput.ko into kernel. */ 
int uart_init(void) 
{
	int nonblock=0;
//	int i=0;
	char* com_port = "/dev/ttyS0";

//	int c;
	int baudrate = 115200;


	if(uart_fd >= 0)
		return uart_fd;

//	create_queue(&keyCmdQueue);//´´½¨¼üÅÌÏûÏ¢»·ÐÎ¶ÓÁÐ
//	create_queue(&mouseCmdQueue);//´´½¨Êó±êÏûÏ¢»·ÐÎ¶ÓÁÐ
	//if(argv)
	//	printf("Program %s is running,argc = %d\n", argv[0],argc);
#if 0	
    if(argc > 1)
	{
	//	printf("usage: ./kmUtil keyboardComName mouseComName\n");		
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
	        case 'p':
	        		p_opt = 1;
	                //debug_level = atoi(optarg);
	                printf("p_opt = 1\n");
	                break;
	        case 'd':
	        	//	com_port = 	
	                if(strncmp(optarg,"/dev/tty",8) == 0)
	             		com_port = optarg;
	             	else
	             		printf("select device error ,start with /dev/tty please!\n");
	                printf("com_port = %s.\n\n",com_port);
	                break;
	        case 'b':
	        		baudrate = atoi(optarg);
	        		if(baudrate < 1200)
	        			baudrate = 115200;
	                printf("set baudrate is: %d\n\n", baudrate);
	             //   p1 = optarg;
	                break;
	        case 'n':
	                printf("set com nonblock mode\n\n");
	                nonblock = 1;
	                break;
	        case ':':
	                fprintf(stderr, "miss option char in optstring.\n");
	                break;
	        case 'D':
	        	break;
	        case '?':
	        case 'h':

	        default:
	             usage(argv[0]);
	                break;
	                //return 0;
	        }
	    }
	    if (optind == 1)
	    {
	        usage(argv[0]);
	    }
	}
#endif	
	
	uart_fd = PortOpen(com_port,nonblock);   //²ÎÊý2Îª0±íÊ¾Îª×èÈûÄ£Ê½£¬·Ç0Îª·Ç×èÈûÄ£Ê½
	if( uart_fd < 0 )
	{
		//uinput_device_close();  //2024-04-02
		printf("Error: open serial port(%s) error.\n", com_port);	
		exit(1);
	}

	//2022-12-19 做一个串口的消息队列，应对多线程的串口数据发送
	Jc_uart_msgq_init();
	//printf("11233\n");
	return PortSet(uart_fd,baudrate,1,'N');    //设置波特率等	，返回0表示成功，-1表示失败
}


//程序退出时，串口部分的处理
void uart_exit(void) 
{
	if(uart_fd >= 0)
	{
//	uinput_device_close();
		close(uart_fd);
		uart_fd = -1;
	}
}


#if 0

/*
*********************************************************************************************************
*	函 数 名: UART_ReceiveByte
*	功能说明: 接收发送端发来的字符         
*	形    参：c  字符
*             timeout  溢出时间
*	返 回 值: 0 接收成功， -1 接收失败
*********************************************************************************************************
*/
int UART_ReceiveByte (uint8_t *c, uint32_t timeout)
{
	int ret;
	ret = PortRecv(uart_fd, c, 1,timeout);

	if(ret<=0)
		printf("ERROR:UART_ReceiveByte\n");

	return (ret>0)?0:-1;
}

/*
*********************************************************************************************************
*	函 数 名: UART_ReceivePacket
*	功能说明: 接收发送端发来的字符         
*	形    参：data  数据
*             timeout  溢出时间
*	返 回 值: 0 接收成功， -1 接收失败
*********************************************************************************************************
*/
int UART_ReceivePacket (uint8_t *data, uint16_t length, uint32_t timeout)
{
	uint8_t i;
	int ret;

	ret = PortRecv(uart_fd, data, length,timeout);   //返回读到的字节数

	if(ret <= 0)
		printf("ERROR:UART_ReceivePacket,length = %d,ret = %d\n",length,ret);
	else
	{
		printf("UART_ReceivePacket,length = %d,ret = %d\n",length,ret);
		for(i=0;i<ret;i++)
			printf("%#x ",data[i]);
		printf("\n");


	}	
	return (length == ret)?0:-1;
}






/*
*********************************************************************************************************
*	函 数 名: Uart_SendByte
*	功能说明: 发送一个字节数据         
*	形    参：c  字符
*	返 回 值: 0
*********************************************************************************************************
*/
void UART_SendByte (uint8_t c)
{
	PortSend(uart_fd,&c, 1);
}




/*
*********************************************************************************************************
*	函 数 名: UART_SendPacket
*	功能说明: 发送一串数据
*	形    参: data  数据
*             length  数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void UART_SendPacket(uint8_t *data, uint16_t length)
{
	PortSend(uart_fd,data, length);
}

#endif

#if 0

void show_version(char* name)
{
    printf( "%s Buildtime :"__DATE__" "__TIME__,name);
}
 
void usage(char* name)
{
    show_version(name);
 
    printf("    -h,    short help\n");
    printf("    -v,    show version\n");
    printf("    -d /dev/ttyS0, select com device\n");
    printf("    -p , printf recv data\n");
    printf("    -b , set baudrate\n");
    printf("    -n , set com nonblock mode\n");
    exit(0);
}



const char* my_opt = "vhpwb:d:";

/* This function will open the uInput device. Please make 
sure that you have inserted the uinput.ko into kernel. */ 
int main(int argc, char *argv[]) 
{
	int nonblock=0;
	int i=0;
	char* com_port = "/dev/ttyS0";
	int uart_fd;
	int c;
	int baudrate = 115200;
	int p_opt = 0;   //ÊÇ·ñÒª´òÓ¡³ö½ÓÊÕµ½µÄ×Ö·û£¿
	    // const char* p1 = NULL;
	    // const char* p2 = NULL;
	    // const char* p3 = NULL;


	create_queue(&keyCmdQueue);//´´½¨¼üÅÌÏûÏ¢»·ÐÎ¶ÓÁÐ
//	create_queue(&mouseCmdQueue);//´´½¨Êó±êÏûÏ¢»·ÐÎ¶ÓÁÐ
 


	printf("Program %s is running\n", argv[0]);
    if(argc != 1)
	{
	//	printf("usage: ./kmUtil keyboardComName mouseComName\n");		
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
	        case 'p':
	        		p_opt = 1;
	                //debug_level = atoi(optarg);
	                printf("p_opt = 1\n");
	                break;
	        case 'd':
	        	//	com_port = 	
	                if(strncmp(optarg,"/dev/tty",8) == 0)
	             		com_port = optarg;
	             	else
	             		printf("select device error ,start with /dev/tty please!\n");
	                printf("com_port = %s.\n\n",com_port);
	                break;
	        case 'b':
	        		baudrate = atoi(optarg);
	        		if(baudrate < 1200)
	        			baudrate = 115200;
	                printf("set baudrate is: %d\n\n", baudrate);
	             //   p1 = optarg;
	                break;
	        case 'n':
	                printf("set com nonblock mode\n\n");
	                nonblock = 1;
	                break;
	        case ':':
	                fprintf(stderr, "miss option char in optstring.\n");
	                break;
	        case '?':
	        case 'h':
	        default:
	                usage(argv[0]);
	                break;
	                //return 0;
	        }
	    }
	    if (optind == 1)
	    {
	        usage(argv[0]);
	    }
	}
	// Return an error if device not found. 
	if (setup_uinput_device() < 0) 
	{ 
		printf("Unable to find uinput device \n"); 
		return -1; 
	} 
	
	uart_fd = PortOpen(com_port,nonblock);   //²ÎÊý2Îª0±íÊ¾Îª×èÈûÄ£Ê½£¬·Ç0Îª·Ç×èÈûÄ£Ê½
	if( uart_fd < 0 )
	{
		ioctl(uinp_fd, UI_DEV_DESTROY);
		close(uinp_fd); 
		printf("Error: open serial port(%s) error.\n", com_port);
		exit(1);
	}

	PortSet(uart_fd,baudrate,1,'N');

	//Ñ­»·´¦Àí½ÓÊÕµÄÊý¾Ý
    info_recv_proc_func(uart_fd,p_opt);	//p_opt Îª´òÓ¡Ñ¡Ïî£¬1±íÊ¾½ÓÊÕµÄÊý¾Ý½«´òÓ¡³öÀ´


    //³ÌÐòÍË³öµÄ´¦Àí
	/* Close the UINPUT device */
	ioctl(uinp_fd, UI_DEV_DESTROY);
	close(uinp_fd);       
    close(uart_fd);
	
}

#endif





