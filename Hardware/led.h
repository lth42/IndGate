/*!
    \file    led.h
    \brief   led driver header (PE8~PE11, active low)
*/

#ifndef __LED_H
#define __LED_H

#include "gd32f4xx.h"

/* led index */
typedef enum {
    LED1 = 0,
    LED2,
    LED3,
    LED4,
    LED_MAX
} led_e;

/* configure led gpio */
void led_init(void);
/* turn one led on (active low) */
void led_on(led_e led);
/* turn one led off */
void led_off(led_e led);
/* toggle one led */
void led_toggle(led_e led);
/* turn all leds on */
void led_all_on(void);
/* turn all leds off */
void led_all_off(void);

#endif /* __LED_H */
