#ifndef _RS232_H
#define _RS232_H

#include "gd32f4xx.h"                   // Device header

// TD301D232H：隔离型RS232收发模块，MCU侧为TTL串口（USART5），无方向控制引脚

#define RS232_CLOCK1					RCU_GPIOC
#define RS232_PORT						GPIOC
#define RS232_PIN_TX					GPIO_PIN_6		//USART5_TX
#define RS232_PIN_RX					GPIO_PIN_7		//USART5_RX

#define RS232_CLOCK2					RCU_USART5

#define RS232_BAUDRATE					115200			//串口波特率（按对端设备修改）

#define RS232_RX_BUFFER_SIZE			256				//接收环形缓冲区大小
#define RS232_TX_BUFFER_SIZE			256				//发送环形缓冲区大小

extern volatile uint16_t rs232_rx_write_pos;	//DMA写入位置（调试观察用）

void rs232_init(void);
uint16_t rs232_senddata(uint8_t *send_buffer, uint16_t length);	//非阻塞发送：拷入内部缓冲后立即返回，返回实际入队长度（0表示缓冲满丢弃）
uint16_t rs232_getdata(uint8_t *recv_buffer, uint16_t length);	//非阻塞接收：取走已接收数据，返回帧长度（0表示无数据）
uint8_t rs232_sendbusy(void);	//查询发送是否忙（1忙，0空闲）

#endif
