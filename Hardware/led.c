#include "led.h"

void led_init(void)
{
	rcu_periph_clock_enable(LED_CLOCK);
	
	gpio_mode_set(LED_PORT,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,LED_CAN|LED_RS485|LED_RS232|LED_NET);
	gpio_output_options_set(LED_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_2MHZ,LED_CAN|LED_RS485|LED_RS232|LED_NET);
	gpio_bit_set(GPIOE,LED_CAN|LED_RS485|LED_RS232|LED_NET);
}
void led_on(uint32_t LED_x)
{
	gpio_bit_reset(LED_PORT,LED_x);
}
void led_off(uint32_t LED_x)
{
	gpio_bit_set(LED_PORT,LED_x);
}