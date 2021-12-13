/*
 * uart.h
 *
 * Created: 12/21/2016 10:47:01 PM
 *  Author: Zsolt
 */ 

#ifndef USART_TOOLS_H_
#define USART_TOOLS_H_

#include <stdint.h>

/**
 * Initialize UART related registers
 * @param [in] baud_rate  USART baud rate,
 * @param [in] isr_en     enable ISR,
 *                        ISR callback must be registered before init is called
 * @return 0 for success
 *         other in case of fail
 * this value will be identical with the baud rate of the COM port
 * for datails on Windows see Device Manager
 */
uint8_t USART_init(uint32_t baud_rate, uint8_t isr_en);

/**
 * Send out the character trough USART.
 * @param [in] c one character to send over UART (UDR register)
 */
void USART_char_send(char c);

/**
 * USART RX interrupt callback type
 * @param [inout] ctx user data for interrupt callback
 * When ISR occurs USART_RXC_cb will be called with ctx as parameter
 */
typedef void (*USART_RXC_cb)(void* ctx);

/**
 * Register callback and context for USART RX interrupt
 * @param [in] cb callback for USART RX isr; must not be NULL
 * @param [in] ctx user defined callback context
 */
void regiter_USART_RXC_cb(USART_RXC_cb cb, void* ctx);

#endif /* USART_TOOLS_H_ */
