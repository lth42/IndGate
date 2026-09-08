/*!
    \file    led_test.h
    \brief   led test header
*/

#ifndef __LED_TEST_H
#define __LED_TEST_H

#include "FreeRTOS.h"
#include "task.h"

/* running light test task */
void led_test_task(void *pvParameters);

#endif /* __LED_TEST_H */
