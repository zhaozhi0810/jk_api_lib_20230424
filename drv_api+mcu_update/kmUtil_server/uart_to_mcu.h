




#ifndef __UART_TO_MCU_H__
#define __UART_TO_MCU_H__

//单片机发给cpu和cpu发给单片机都是4个字节！！！  2022-07-27


//注意单片机与cpu保持一致  2022-07-28
//#pragma pack(1) 这个会设置全局的，注释掉
//数据总共4个字节，这里不含帧头
typedef struct
{
	unsigned char data_type;   //led的控制，状态的获取，lcd的熄灭
	unsigned char data;
//	mcu_data_t data;
	unsigned char crc;     //校验和
}__attribute__((packed))com_frame_t;    //注意对齐方式



typedef enum
{	
	eMCU_LED_STATUS_TYPE=50,  //获得led的状态
	eMCU_KEY_STATUS_TYPE,    //51.获得按键的状态
	eMCU_LED_SETON_TYPE,    //52.设置对应的led亮
	eMCU_LED_SETOFF_TYPE,    //53.设置对应的led灭
	eMCU_LCD_SETONOFF_TYPE,  //54.lcd打开关闭
	eMCU_KEY_CHANGE_TYPE,    //55.按键被修改上报
    eMCU_LEDSETALL_TYPE,     //56.对所有led进行控制，点亮或者熄灭
	eMCU_LEDSETPWM_TYPE,     //57.设置所有led的亮度 
	eMCU_GET_TEMP_TYPE,      //58.获得单片机内部温度	
	eMCU_HWTD_SETONOFF_TYPE,   //59.开门狗设置开关
	eMCU_HWTD_FEED_TYPE,       //60.看门狗喂狗
	eMCU_HWTD_SETTIMEOUT_TYPE,    //61.设置看门狗喂狗时间
	eMCU_HWTD_GETTIMEOUT_TYPE,    //62.获取看门狗喂狗时间
	eMCU_RESET_COREBOARD_TYPE,  //63.复位核心板
	eMCU_RESET_LCD_TYPE,        //64.复位lcd 9211（复位引脚没有连通）
	eMCU_RESET_LFBOARD_TYPE,    //65.复位底板，单片机重启
	eMCU_MICCTRL_SETONOFF_TYPE,  //66.MICCTRL 引脚的控制,1.3版本改到3399控制了！！！
	eMCU_LEDS_FLASH_TYPE  ,//67.led闪烁控制
	eMCU_LSPK_SETONOFF_TYPE  , //68.LSPK,2022-11-11 1.3新版增加
	eMCU_V12_CTL_SETONOFF_TYPE ,  //69.V12_CTL,2022-11-14 1.3新版增加
	eMCU_GET_LCDTYPE_TYPE  ,   //70.上位机获得LCD类型的接口，之前是在3399，现在改为单片机实现，2022-12-12
	eMCU_SET_7INCHPWM_TYPE ,  //71.7inch lcd的pwm值调整,2022-12-13
	eMCU_5INLCD_SETONOFF_TYPE  ,  //72.5英寸背光使能引脚的控制,2022-12-13
	eMCU_GET_MCUVERSION_TYPE       //73.获取单片机版本

}mcu_data_type;

/*
	注意要 处理com_message_handle(uart_to_mcu.c) 的接收部分，不然会导致应答超时

 */




int uart_init(int argc, char *argv[]) ;

//负责接收数据的线程
/*
 * arg 传出参数，串口收到数据，要修改全局变量，所以arg指向全局数据结构
 * 
 * 返回值：
 * 	根据线程函数定义，无意义，该函数不返回！！！！
 * 
 * */
void* mcu_recvSerial_thread(void* arg);


//发送数据，不由单独的线程处理了。data只需要包含数据类型和数据。头部和crc由该函数完成。
/*
 * data 用于发送的数据，不需要包括帧头和校验和，只要包括数据类型和数据（共2个字节）
 * 返回值
 * 	0表示成功，其他表示失败
 2023-01-14 设置指令均不应答！！！
 * */
int send_mcu_data(const void* data,int isreply);  //只需要两个字节的数据就行


//程序退出时，串口部分的处理
void uart_exit(void) ;



/*
*********************************************************************************************************
*	函 数 名: UART_ReceiveByte
*	功能说明: 接收发送端发来的字符         
*	形    参：c  字符
*             timeout  溢出时间
*	返 回 值: 0 接收成功， -1 接收失败
*********************************************************************************************************
*/
int UART_ReceiveByte (uint8_t *c, uint32_t timeout);


/*
*********************************************************************************************************
*	函 数 名: UART_ReceivePacket
*	功能说明: 接收发送端发来的字符         
*	形    参：data  数据
*             timeout  溢出时间
*	返 回 值: 0 接收成功， -1 接收失败
*********************************************************************************************************
*/
int UART_ReceivePacket (uint8_t *data, uint16_t length, uint32_t timeout);



/*
*********************************************************************************************************
*	函 数 名: Uart_SendByte
*	功能说明: 发送一个字节数据         
*	形    参：c  字符
*	返 回 值: 0
*********************************************************************************************************
*/
void UART_SendByte (uint8_t c);


/*
*********************************************************************************************************
*	函 数 名: UART_SendPacket
*	功能说明: 发送一串数据
*	形    参: data  数据
*             length  数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void UART_SendPacket(uint8_t *data, uint16_t length);




#endif
