/*!
    \file    main.c
    \brief   GD32F407VGT6 FreeRTOS demo: led running light test
*/

#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "led_test.h"
#include "main.h"

/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* configure led gpio */
    led_init();

    /* create led running light test task */
    if(pdPASS != xTaskCreate(led_test_task, "led_test", 128U, NULL, 2U, NULL)) {
        goto fault;
    }

    /* start scheduler, never return */
    vTaskStartScheduler();

fault:
    /* create failed or scheduler returned: indicate fault */
    led_all_off();
    while(1) {
    }
}

/*!
    \brief      malloc failed hook (FreeRTOS heap exhausted)
*/
void vApplicationMallocFailedHook(void)
{
    led_all_off();
    while(1) {
    }
}

/*!
    \brief      stack overflow hook (configCHECK_FOR_STACK_OVERFLOW = 2)
*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    led_all_off();
    while(1) {
    }
}

/*!
    \brief      assert hook (configASSERT)
*/
void vAssertCalled(const char *pcFile, int ulLine)
{
    (void)pcFile;
    (void)ulLine;

    led_all_off();
    while(1) {
    }
}
