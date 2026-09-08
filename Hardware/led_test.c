/*!
    \file    led_test.c
    \brief   led test: running light on LED1~LED4 (FreeRTOS task)
*/

#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "led_test.h"

#define LED_TEST_DELAY_MS   200U

/*!
    \brief      running light test task, leds light up one by one in loop
    \param[in]  pvParameters: task parameter (unused)
    \param[out] none
    \retval     none
*/
void led_test_task(void *pvParameters)
{
    led_e led = LED1;

    (void)pvParameters;

    led_all_off();

    for(;;) {
        led_all_off();
        led_on(led);
        vTaskDelay(LED_TEST_DELAY_MS / portTICK_PERIOD_MS);

        led++;
        if(led >= LED_MAX) {
            led = LED1;
        }
    }
}
