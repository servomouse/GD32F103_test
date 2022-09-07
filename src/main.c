#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gd32f10x.h"
#include "systick.h"
#include "gd32f10x_rcu.h"
#include "gd32f10x_gpio.h"


#include "cdc_acm_core.h"
#include "usbd_hw.h"

#define ARRAYNUM(arr_nanme)      (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
#define TRANSMIT_SIZE            (ARRAYNUM(txbuffer) - 1)

uint8_t txbuffer[] = "\n\rUSART interrupt test\n\r";
uint8_t rxbuffer[32];
uint8_t tx_size = TRANSMIT_SIZE;
uint8_t rx_size = 32;
__IO uint8_t txcount = 0; 
__IO uint16_t rxcount = 0; 

usb_dev usbd_cdc;

/* Select CPU clock in file system_gd32f10x.c!! */

#define LED_PORT GPIOC
#define MYLED	GPIO_PIN_13

void RCU_Config(void);
void NVIC_Config(void);
void GPIO_Config(void);
void print(uint8_t *str, uint16_t len);

int main(void)  // UART
{
    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
	rcu_periph_clock_enable(RCU_GPIOC);
    /* configure COM0 */
    gd_eval_com_init(EVAL_COM0);
    /* enable USART TBE interrupt */  
    usart_interrupt_enable(USART0, USART_INT_TBE);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);
	gpio_bit_set(LED_PORT, MYLED);
    
    /* wait until USART send the transmitter_buffer */
    while(txcount < tx_size);
    
    while(RESET == usart_flag_get(USART0, USART_FLAG_TC));
    
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    systick_config();
    
    /* wait until USART receive the receiver_buffer */
    // while(rxcount < rx_size);
    // if(rxcount == rx_size)
    //     printf("\n\rUSART receive successfully!\n\r");
    while (1)
    {
        gpio_bit_set(LED_PORT, MYLED);
        delay_1ms(500);
        gpio_bit_reset(LED_PORT, MYLED);
        delay_1ms(500);
        print("\n\rHello world!\n\r", 16);
    }
}

void print(uint8_t *str, uint16_t len)
{
    if(len < sizeof(txbuffer))
    {
        memcpy(txbuffer, str, len);
        tx_size = len;
        usart_data_transmit(USART0, txbuffer[0]);
        usart_interrupt_enable(USART0, USART_INT_TBE);
        txcount = 1;
    }
}

int main_usb(void)
{
    /* system clocks configuration */
    rcu_config();
	rcu_periph_clock_enable(RCU_GPIOC);

    /* GPIO configuration */
    gpio_config();
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);
	gpio_bit_set(LED_PORT, MYLED);

    /* USB device configuration */
    usbd_init(&usbd_cdc, &cdc_desc, &cdc_class);

    /* NVIC configuration */
    nvic_config();

    /* enabled USB pull-up */
    usbd_connect(&usbd_cdc);

    while (USBD_CONFIGURED != usbd_cdc.cur_status);/* wait for standard USB enumeration is finished */

    gpio_bit_reset(LED_PORT, MYLED);
    while (1)
	{
        if (0U == cdc_acm_check_ready(&usbd_cdc))
            cdc_acm_data_receive(&usbd_cdc);
        else
            cdc_acm_data_send(&usbd_cdc);
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM0, (uint8_t)ch);
    while (RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
    return ch;
}

int main_blink(void)
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
