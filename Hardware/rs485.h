#ifndef _RS485_H
#define _RS485_H

#include "gd32f4xx.h"                   // Device header

// RS485收发器为自动收发方向，无需DE/RE控制脚

#define RS485_CLOCK1					RCU_GPIOD
#define RS485_PORT						GPIOD
#define RS485_PIN_TX					GPIO_PIN_5		//USART1_TX
#define RS485_PIN_RX					GPIO_PIN_6		//USART1_RX

#define RS485_CLOCK2					RCU_USART1

#define RS485_BAUDRATE					9600			//串口波特率（按对端设备修改）

#define RS485_RX_BUFFER_SIZE			256				//接收环形缓冲区大小
#define RS485_TX_BUFFER_SIZE			256				//发送环形缓冲区大小

extern volatile uint16_t rs485_rx_write_pos;	//DMA写入位置（调试观察用）

void rs485_init(void);
uint16_t rs485_senddata(uint8_t *send_buffer, uint16_t length);	//非阻塞发送：拷入内部缓冲后立即返回，返回实际入队长度（0表示缓冲满丢弃）
uint16_t rs485_getdata(uint8_t *recv_buffer, uint16_t length);	//非阻塞接收：取走已接收数据，返回帧长度（0表示无数据）
uint8_t rs485_sendbusy(void);	//查询发送是否忙（1忙，0空闲）

#endif
