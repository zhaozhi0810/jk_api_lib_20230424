/*
* @Author: dazhi
* @Date:   2022-07-27 09:57:14
* @Last Modified by:   dazhi
* @Last Modified time: 2022-07-27 10:19:48
*/


#ifndef __MY_IPC_SMGQ_H__
#define __MY_IPC_SMGQ_H__


//用户创建消息队列的关键字
#define KEY_PATH "/proc/cpuinfo"
#define KEY_ID -123321

typedef struct  //消息结构体
{
    long types;    //指定消息的类型
    int cmd;       //消息的命令
    int param1;    //参数1
    int param2;    //参数2
    int ret;       //参数3，可用于返回值
}msgq_t; 



enum API_CMD_types{
	eAPI_LEDSET_CMD = 0,   //设置led
	eAPI_LEDGET_CMD,         //1.获取led状态
	eAPI_LEDSETALL_CMD,      //2.设置所有的led
	eAPI_LEDGETALL_CMD,      //3.获取所有的led
	eAPI_BTNGET_CMD,         //4.获取按键（没有使用）
	eAPI_BTNEVENT_CMD,       //5.等待按键事件（没有使用）
	eAPI_LCDONOFF_CMD,       //6.lcd开启关闭事件，2022-12-13
	eAPI_LEDSETPWM_CMD,      //7.led设置亮度
	eAPI_BOART_TEMP_GET_CMD,  //8.获得单片机的温度
	eAPI_CHECK_APIRUN_CMD,  //9.检测api是否已经运行
	eAPI_HWTD_SETONOFF_CMD,   //10.开门狗设置开关
	eAPI_HWTD_FEED_CMD ,      //11.看门狗喂狗
	eAPI_HWTD_SETTIMEOUT_CMD,    //12.设置看门狗喂狗时间
	eAPI_HWTD_GETTIMEOUT_CMD,    //13.获取看门狗喂狗时间
	eAPI_RESET_COREBOARD_CMD,  //14.复位核心板
	eAPI_RESET_LCD_CMD,        //15.复位lcd 9211（复位引脚没有连通）
	eAPI_RESET_LFBOARD_CMD,    //16.复位底板，好像没有这个功能！！！
	eAPI_MICCTRL_SETONOFF_CMD,    //17.控制底板mic_ctrl引脚的电平
	eAPI_LEDS_FLASH_CMD ,  // 18.led键灯闪烁控制
	eAPI_LSPK_SETONOFF_CMD  , //19.LSPK,2022-11-11 1.3新版增加
	eAPI_V12_CTL_SETONOFF_CMD ,  //20.V12_CTL,2022-11-14 1.3新版增加
	eAPI_GET_LCDTYPE_CMD  ,  // 21.上位机获得LCD类型的接口，之前是在3399，现在改为单片机实现，2022-12-12
	eAPI_SET_7INCHPWM_CMD ,  //22.7inch lcd的pwm值调整
	eAPI_GET_MCUVERSION_CMD ,  //23.获取单片机版本，2023-01-15
	eAPI_CONTROL_TTYS0_CMD   //24.控制ttys0的打开和关闭，2023-04-24，用于单片机串口升级
//	eAPI_5INLCD_SETONOFF_CMD  //,  //5寸背光使能控制，2022-12-13
};





//返回值0表示成功，其他表示失败
int Jc_msgq_init(void);


//返回值0表示成功，其他表示失败
int Jc_msgq_exit(void);

//接收消息
//参数 timeout_50ms 大于0时，表示非阻塞模式，等待的时间为timeout_50ms*50ms的值，等于0表示阻塞模式
//     types 接收消息的类型
//     msgbuf 接收消息的缓存，1次接收一条消息
//返回0表示成功，其他表示失败
int Jc_msgq_recv(long types,msgq_t *msgbuf,unsigned int timeout_50ms);



//发送消息，并且要等待应答。
////参数 timeout 大于0时，设置接收应答超时时间，等于0表示阻塞模式
//     ack_types 指定应答的类型，之前是TYPE_RECV+cmd
//     msgbuf 发送消息的缓存，1次发送一条消息
//返回0表示成功，其他表示失败
//注意，函数使用时，需要指定msgbuf->types ！！！！
int Jc_msgq_send(long ack_types,msgq_t *msgbuf,int timeout);


//发送应答消息消息，不等待应答。
int Jc_msgq_send_ack(msgq_t *msgbuf);



#endif


