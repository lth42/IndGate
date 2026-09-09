#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "main.h"
#include "ethernet.h"
#include "rs232.h"
#include <stdio.h>


static void default_task(void *pvParameters);
static void rs232_test_task(void *pvParameters);


int main(void)
{
	// 中断优先级分组：所有位都作抢占优先级（FreeRTOS要求）
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	led_init();
	ethernet_init();
	rs232_init();

	// 创建默认任务
	xTaskCreate(default_task, "default_task", 128, NULL, 1, NULL);

	// 创建RS232测试任务
	xTaskCreate(rs232_test_task, "rs232_test", 128, NULL, 1, NULL);

	// 启动调度器，不会返回
	vTaskStartScheduler();
	while(1)
	{

	}
}

// 默认任务：ethernet测试，回发收到的数据，每2秒发送一次时间戳；LED慢闪指示任务运行
// 收发全部非阻塞：发送拷入内部缓冲立即返回，接收轮询环形缓冲
static void default_task(void *pvParameters)
{
	static uint8_t rxbuf[256];	//static避免占用任务栈
	static uint8_t tsbuf[32];
	uint16_t len;
	uint32_t blink = 0;
	uint32_t ts_cnt = 0;
	uint8_t led_state = 0;

	(void)pvParameters;

	for( ;; ) {
		// 取走数据并原样回发
		len = ethernet_getdata(rxbuf, sizeof(rxbuf));
		if(len > 0) {
			ethernet_senddata(rxbuf, len);
		}

		// 每2秒发送一次时间戳（开机秒数）
		ts_cnt++;
		if(ts_cnt >= 200) {
			ts_cnt = 0;
			len = snprintf((char *)tsbuf, sizeof(tsbuf), "ts:%lu\r\n", (unsigned long)(xTaskGetTickCount() / 1000));
			ethernet_senddata(tsbuf, len);
		}

		// LED每500ms翻转一次
		blink++;
		if(blink >= 50) {
			blink = 0;
			if(led_state) {
				led_off(LED_CAN);
				led_state = 0;
			} else {
				led_on(LED_CAN);
				led_state = 1;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

// RS232诊断任务：轮询方式直接写USART5发送（绕开DMA），每1秒发一条
static void rs232_test_task(void *pvParameters)
{
	static const uint8_t msg[] = "polling tx test\r\n";
	uint16_t i;

	(void)pvParameters;

	for( ;; ) {
		// 轮询发送：绕开DMA，直接写数据寄存器
		for(i = 0; i < sizeof(msg) - 1; i++) {
			usart_data_transmit(USART5, msg[i]);
			while(RESET == usart_flag_get(USART5, USART_FLAG_TBE));
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
