#ifndef __LED_H
#define __LED_H

#include "gd32f4xx.h"

#define	LED_CLOCK	RCU_GPIOE
#define	LED_PORT	GPIOE
#define LED_CAN		GPIO_PIN_8
#define LED_RS485	GPIO_PIN_9
#define LED_RS232	GPIO_PIN_10
#define LED_NET		GPIO_PIN_11


void led_init(void);
void led_on(uint32_t LED_x);
void led_off(uint32_t LED_x);

#endif
