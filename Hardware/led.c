/*!
    \file    led.c
    \brief   led driver for GD32F407VGT6

    4 leds on PE8~PE11, common anode to 3.3V, low level lights the led
*/

#include "led.h"

#define LED_RCU         RCU_GPIOE
#define LED_PORT        GPIOE

static const uint32_t led_pin[LED_MAX] = {
    GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11
};

/*!
    \brief      configure all led gpios, leds default off
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_init(void)
{
    uint32_t all_pins = 0U;
    led_e led;

    for(led = LED1; led < LED_MAX; led++) {
        all_pins |= led_pin[led];
    }

    /* enable the led clock */
    rcu_periph_clock_enable(LED_RCU);
    /* configure led pins as push-pull output */
    gpio_mode_set(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, all_pins);
    gpio_output_options_set(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, all_pins);
    /* pins high = leds off (active low) */
    GPIO_BOP(LED_PORT) = all_pins;
}

/*!
    \brief      turn one led on
    \param[in]  led: led index (LED1~LED4)
    \param[out] none
    \retval     none
*/
void led_on(led_e led)
{
    if(led >= LED_MAX) {
        return;
    }
    gpio_bit_reset(LED_PORT, led_pin[led]);
}

/*!
    \brief      turn one led off
    \param[in]  led: led index (LED1~LED4)
    \param[out] none
    \retval     none
*/
void led_off(led_e led)
{
    if(led >= LED_MAX) {
        return;
    }
    gpio_bit_set(LED_PORT, led_pin[led]);
}

/*!
    \brief      toggle one led
    \param[in]  led: led index (LED1~LED4)
    \param[out] none
    \retval     none
*/
void led_toggle(led_e led)
{
    if(led >= LED_MAX) {
        return;
    }
    gpio_bit_toggle(LED_PORT, led_pin[led]);
}

/*!
    \brief      turn all leds on
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_all_on(void)
{
    uint32_t all_pins = 0U;
    led_e led;

    for(led = LED1; led < LED_MAX; led++) {
        all_pins |= led_pin[led];
    }
    GPIO_BC(LED_PORT) = all_pins;
}

/*!
    \brief      turn all leds off
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_all_off(void)
{
    uint32_t all_pins = 0U;
    led_e led;

    for(led = LED1; led < LED_MAX; led++) {
        all_pins |= led_pin[led];
    }
    GPIO_BOP(LED_PORT) = all_pins;
}
