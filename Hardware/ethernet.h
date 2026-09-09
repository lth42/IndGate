#ifndef _ETHERNET_H
#define _ETHERNET_H

#include "gd32f4xx.h"                   // Device header

#define ETHERNET_CLOCK1					RCU_GPIOA
#define ETHERNET_PORT1					GPIOA
#define ETHERNET_PIN_CFG  			GPIO_PIN_2		//CFG
#define ETHERNET_PIN_CTS  			GPIO_PIN_3		//CTS
#define ETHERNET_PIN_TX				GPIO_PIN_9		//USART0_TX（PA10经短接桥接到模块RXD）

#define ETHERNET_CLOCK2					RCU_GPIOB
#define ETHERNET_PORT2					GPIOB
#define ETHERNET_PIN_RX				GPIO_PIN_3		//USART0_RX

#define ETHERNET_CLOCK3					RCU_GPIOC
#define ETHERNET_PORT3					GPIOC
#define ETHERNET_PIN_RTS_TNOW1  GPIO_PIN_2		//RTS/TNOW1
#define ETHERNET_PIN_TNOW2  		GPIO_PIN_3		//TNOW2

#define ETHERNET_CLOCK4					RCU_USART0

#define ETHERNET_RX_BUFFER_SIZE			256				//DMA接收缓冲区大小
#define ETHERNET_TX_BUFFER_SIZE			256				//发送环形缓冲区大小

extern volatile uint16_t ethernet_rx_write_pos;	//DMA写入位置（调试观察用）
extern volatile uint16_t ethernet_rx_read_pos;		//已取走数据的读位置（调试观察用）
extern volatile uint32_t ethernet_dbg_rx;		//调试：取到数据的次数

void ethernet_init(void);
uint16_t ethernet_senddata(uint8_t *send_buffer, uint16_t length);	//非阻塞发送：拷入内部缓冲后立即返回，返回实际入队长度（0表示缓冲满丢弃）
uint16_t ethernet_getdata(uint8_t *recv_buffer, uint16_t length);	//非阻塞接收：取走已接收数据，返回帧长度（0表示无数据）
uint8_t ethernet_sendbusy(void);	//查询发送是否忙（1忙，0空闲）

#endif
