/*
 * uart.c
 *
 * Created: 12/21/2016 10:46:13 PM
 *  Author: Zsolt
 */ 

#include "uart.h"

#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

static USART_RXC_cb  USART_cb_     = NULL;
static void         *USART_cb_ctx_ = NULL;
static uint8_t       isr_set_      = 0;

static int USART_printf(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(USART_printf, NULL, _FDEV_SETUP_WRITE);

 uint8_t USART_init (uint32_t baud_rate, uint8_t isr_en) {
     // calculate from Baud rate the pre-scaler value for the microcontroller baud rate counter
     const uint16_t baud_prescale =  (((F_CPU / (baud_rate * 16UL))) - 1);

    cli();                                // disable all interrupts

    UCSRB = (1 << RXEN)  | (1 << TXEN);   // Turn on the transmission and reception circuitry

    if (isr_en) {
        if (!isr_set_) return 1;
        UCSRB |= (1 << RXCIE);                // interrupt in receive complete
    }

    UCSRC |= (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes
    UBRRH = baud_prescale >> 8;           // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
    UBRRL = baud_prescale;                // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
    stdout = &mystdout;                   // Required for printf initialization

    sei();                                // enable interrupts

    return 0;
}

void USART_char_send(char c) {
    while ((UCSRA & (1 << UDRE)) == 0) {}; // wait until the USATR Data register is empty
    UDR = c;                               // put the value from the pointed location in the USART Data register
}

static int USART_printf(char c, FILE *stream) {
    USART_char_send(c);
    return 0;
}

void regiter_USART_RXC_cb(USART_RXC_cb cb, void* ctx) {
    if (cb) {
        USART_cb_ = cb;
    } else {
        return;
    }

    USART_cb_ctx_ = ctx;
    isr_set_ = 1;
}

// Interrupt service routine for receive complete: each time a character is sent from serial terminal 
// the routine saves the character into global variable USART_ReadByte.
ISR(USART_RXC_vect) {
    USART_cb_(USART_cb_ctx_); // save the contain
}
