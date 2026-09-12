#ifndef _CAN1_H
#define _CAN1_H

#include "gd32f4xx.h"                   // Device header

#define CAN1_CLOCK						RCU_CAN1
#define CAN1_PORT						GPIOB
#define CAN1_PIN_TX						GPIO_PIN_6		//CAN1_TX
#define CAN1_PIN_RX						GPIO_PIN_5		//CAN1_RX

// 波特率：500kbps（APB1=42MHz，prescaler=6，1+10+3=14tq，采样点78.6%）

void can1_init(void);
uint8_t can1_senddata(uint32_t frame_id, uint8_t *data, uint8_t len);	//非阻塞发送：标准数据帧（len<=8），返回1成功入队邮箱，0=邮箱满丢弃
uint8_t can1_getdata(uint32_t *frame_id, uint8_t *data, uint8_t *len);	//非阻塞接收：从FIFO0取一帧，返回1收到，0无数据

#endif
