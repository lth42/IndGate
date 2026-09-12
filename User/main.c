#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "main.h"
#include "ethernet.h"
#include "rs232.h"
#include "rs485.h"
#include "can1.h"
#include <stdio.h>


static void default_task(void *pvParameters);
static void rs232_test_task(void *pvParameters);
static void rs485_test_task(void *pvParameters);
static void can_test_task(void *pvParameters);


int main(void)
{
	// 中断优先级分组：所有位都作抢占优先级（FreeRTOS要求）
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
	led_init();
	ethernet_init();
	rs232_init();
	rs485_init();
	can1_init();

	// 创建默认任务
	xTaskCreate(default_task, "default_task", 128, NULL, 1, NULL);

	// 创建RS232测试任务
	xTaskCreate(rs232_test_task, "rs232_test", 128, NULL, 1, NULL);

	// 创建RS485测试任务
	xTaskCreate(rs485_test_task, "rs485_test", 128, NULL, 1, NULL);

	// 创建CAN测试任务
	xTaskCreate(can_test_task, "can_test", 128, NULL, 1, NULL);

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

// RS485测试任务：回发收到的数据 + 每1秒发送一帧测试数据
static void rs485_test_task(void *pvParameters)
{
	static uint8_t rxbuf[256];	//static避免占用任务栈
	static uint8_t txbuf[32];
	uint16_t len;
	uint32_t poll_cnt = 0;
	static uint32_t tx_cnt = 0;	//测试发送计数

	(void)pvParameters;

	for( ;; ) {
		// 回发收到的数据
		len = rs485_getdata(rxbuf, sizeof(rxbuf));
		if(len > 0) {
			rs485_senddata(rxbuf, len);
		}

		// 每1秒发送一帧测试数据
		poll_cnt++;
		if(poll_cnt >= 100) {
			poll_cnt = 0;
			len = snprintf((char *)txbuf, sizeof(txbuf), "rs485 tx test %lu\r\n", (unsigned long)(++tx_cnt));
			rs485_senddata(txbuf, len);
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

// CAN测试任务：回发收到的CAN帧（同ID同数据）+ 每1秒发送一帧测试数据（ID=0x123）
static void can_test_task(void *pvParameters)
{
	uint32_t frame_id;
	uint8_t data[8];
	uint8_t len;
	uint32_t poll_cnt = 0;
	static uint32_t tx_cnt = 0;	//测试发送计数

	(void)pvParameters;

	for( ;; ) {
		// 收到一帧就原样回发
		if(can1_getdata(&frame_id, data, &len)) {
			can1_senddata(frame_id, data, len);
		}

		// 每1秒发送一帧测试数据，data前4字节为递增计数
		poll_cnt++;
		if(poll_cnt >= 100) {
			poll_cnt = 0;
			tx_cnt++;
			data[0] = (uint8_t)(tx_cnt >> 24);
			data[1] = (uint8_t)(tx_cnt >> 16);
			data[2] = (uint8_t)(tx_cnt >> 8);
			data[3] = (uint8_t)tx_cnt;
			if(0 == can1_senddata(0x123, data, 4)) {
				tx_cnt--;	//邮箱满未入队，计数回退，下次重发
			}
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
