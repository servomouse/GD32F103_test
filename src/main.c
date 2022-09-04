#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gd32f10x.h"
#include "systick.h"
#include "gd32f10x_rcu.h"
#include "gd32f10x_gpio.h"

/* Select CPU clock in file system_gd32f10x.c!! */

#define LED_PORT GPIOC
#define MYLED	GPIO_PIN_13

void RCU_Config(void);
void NVIC_Config(void);
void GPIO_Config(void);

int main(void)
{
    RCU_Config();
	NVIC_Config();
	GPIO_Config();
    systick_config();

    while(1)
    {
        gpio_bit_set(LED_PORT, MYLED);
        delay_1ms(500);
        gpio_bit_reset(LED_PORT, MYLED);
        delay_1ms(500);
    }
}

void RCU_Config(void)
{
	rcu_periph_clock_sleep_enable(RCU_FMC_SLP);
	rcu_periph_clock_sleep_enable(RCU_SRAM_SLP);

	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_USART0);
}

void NVIC_Config(void)
{
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	nvic_irq_enable(USART0_IRQn, 3, 1);
}

void GPIO_Config(void)
{
	// PA9  = TXD USART0
	// PA10 = RXD USART0
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_10);

	// PC13 = LED2
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);
}