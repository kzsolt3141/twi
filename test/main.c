#include "uart.h"
#include "twi.h"

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

typedef struct USART_RXC_cb_ctx_t {
    uint8_t rx;
    int cnt;
} USART_RXC_cb_ctx;

/**
 * USART RX interrupt callback handle
 * @param[inout] ctx user data for interrupt callback
 * When ISR occurs USART_RXC_cb will be called with ctx as parameter
 * UART RX data (UDR) should be saved in this function
 */
static void USART_RXC_cb_handle(void* ctx) {
    USART_RXC_cb_ctx* t_ctx = (USART_RXC_cb_ctx*)ctx;

    t_ctx->rx = UDR;
    t_ctx->cnt++;
}

typedef struct TWI_cb_ctx_t {
    int cnt;
}TWI_cb_ctx;

/**
 * TWI interrupt callback handle
 * @param[inout] ctx user data for interrupt callback
 */
static void TWI_cb_handle(void* ctx) {
    TWI_cb_ctx* t_ctx = (TWI_cb_ctx*)ctx;

    t_ctx->cnt++;
}

int main(void) {

    // UART INIT
    //-------------------------------
    const uint16_t baud_rate = 38400;

    uint8_t sts = 0;
    struct USART_RXC_cb_ctx_t USART_RXC_ctx = {};

    regiter_USART_RXC_cb(USART_RXC_cb_handle, &USART_RXC_ctx);

    USART_init(baud_rate);

    printf("Init Done UART baud: %u\n", (uint16_t)baud_rate);

    // TWI init
    //-------------------------------
    uint8_t slave_addr;
    TWI_cb_ctx twi_ctx = {};
    regiter_TWI_isr_cb(TWI_cb_handle, &twi_ctx);

    TWI_init(TWI_PS_1, 2);

    printf("TWI init done\n");

    uint8_t start = 0x00;
    const uint8_t end = 0x7F;

    do {
        sts = TWI_sniff(start, end, &slave_addr);
        printf("TWI sniff interval: [0x%02x .. 0x%02x]\n", start, end);
        if (sts) {
            printf("slaves:%d fist slave addr:0x%02x\n", sts, slave_addr);
            printf("total number of TWI interrupts: %d\n", twi_ctx.cnt);
        } else {
            printf("No slaves found in the interval");
        }
        start = slave_addr + 1;
    } while (sts > 0);

    while(1) {
    }

    return sts;
}
