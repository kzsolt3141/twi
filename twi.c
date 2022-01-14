#include "twi.h"

#include <stddef.h>

#include <avr/io.h>
#include <avr/common.h>
#include <util/twi.h>
#include <avr/interrupt.h>

static TWI_isr_cb  TWI_cb_     = NULL;
static void       *TWI_cb_ctx_ = NULL;

void regiter_TWI_isr_cb(TWI_isr_cb cb, void* ctx) {
    TWI_cb_ = cb;
    TWI_cb_ctx_ = ctx;
}

/**
 * After each thransfer the TWI controller signalizes the
 * status of the transfer writing a status code into TWSR register
 * the function returnes the status register value
 * obs: lower 3 bits are not foo status => masking them is required
 */
static inline uint8_t TWI_status(void) {
    return TWSR & 0xF8; // mask lower 3 bits (prescaler value)
}

void TWI_init(TWI_clock_source clk_src, uint8_t bit_rate) {
    cli();

    TWSR  = clk_src;      // set prescaler
    TWBR  = bit_rate;     // set bitrate

    if (TWI_cb_) {
        TWCR = (1 << TWIE);  // TWI interrupt enable
    }

    TWCR |= (1 << TWEN);  // enable TWI

    sei();
}

uint8_t TWI_start_wait(void) {
    TWCR |= (1 << TWINT) |  // clearing this flag (by writing 1) will start a transfer
            (1 << TWSTA);   // set controller to send start bit trough TWI
    while ((TWCR & (1 << TWINT)) == 0);  // wait until TWINT is cleared

    return TWI_status();
}

void TWI_stop(void) {
    TWCR |= (1 << TWINT)|  // clearing this flag will start a transfer
            (1 << TWSTO);  // enable TWI interface
}

uint8_t TWI_write_wait(uint8_t data) {
    TWDR = data;           // insert 8bit data in data register

    TWCR |= (1 << TWINT);    // start a transfer by clearing TWINT flag

    TWCR &= ~(1 << TWSTA);   // start bit condition set to 0 !!!!! NOT HERE!

    while ((TWCR & (1 << TWINT)) == 0); // wait until TWINT is cleared

    return TWI_status();
}

uint8_t TWI_read_ACK(uint8_t* data) {
    TWCR |= (1 << TWINT)|               // start a transfer by clearing TWINT flag
            (1 << TWEA);                // send/receive acknowledge bit

    while ((TWCR & (1 << TWINT)) == 0); // wait until TWINT is cleared

    *data = TWDR;                       // return read out data

    return TWI_status();
}

uint8_t TWI_read_NACK_wait(uint8_t* data) {
    TWCR &= ~(1 << TWEA);  // disable ACK

    TWCR |= (1 << TWINT);  // start a transfer by clearing TWINT flag

    while ((TWCR & (1 << TWINT)) == 0);  // wait until TWINT is cleared

    *data = TWDR;  // return read out data

    return TWI_status();
}

uint8_t TWI_read_reg_burst (
    uint8_t *buffer,
    uint8_t slave_addr,
    uint8_t internal_addr, 
    uint8_t size
) {
    uint8_t status = 0;

    slave_addr <<= 1;

    if ((status = TWI_start_wait()) != TW_START) return status;  // check if S was sent

    // write to the slave address with W (write) condition (U8 {ADDR,W})
    if ((status = TWI_write_wait(slave_addr)) != TW_MT_SLA_ACK) return status;  // check slave respond

    // send to slave the internal register address which will be read
    if ((status = TWI_write_wait(internal_addr)) != TW_MT_DATA_ACK) return status;  // // check slave respond to data write

    if ((status = TWI_start_wait() != TW_REP_START)) return status;  // verify if the slave responds to the repeated start condition

    // write to the slave address with R (read) condition
    if ((status = TWI_write_wait(slave_addr | 1)) != TW_MR_SLA_ACK) return status; // verify if slave responds to his address

    for(uint8_t i = 0; i < (size -1); i++) {
        // read out data from with ACK (if only one data will be read out, no need for ACK)
        if ((status = TWI_read_ACK(buffer + i)) != TW_MR_DATA_ACK) return status;
    }

    if ((status = TWI_read_NACK_wait(buffer + size - 1)) != TW_MR_DATA_NACK) return status;  // last read without ACK

    TWI_stop();  // send P (stop) bit

    return 0;
}

uint8_t TWI_write_reg (
    uint8_t slave_addr,
    uint8_t internal_addr,
    const uint8_t data
) {
    uint8_t status = 0;

    slave_addr <<= 1;

    if ((status = TWI_start_wait()) != TW_START) return status;  // check if S was sent

    // write to the slave address with W (write) condition (U8 {ADDR,W})
    if ((status = TWI_write_wait(slave_addr)) != TW_MT_SLA_ACK) return status;  // check slave respond

    if ((status = TWI_write_wait(internal_addr)) != TW_MT_DATA_ACK) return status;  // // check slave respond to data write

    if ((status = TWI_write_wait(data)) != TW_MT_DATA_ACK) return status;  // // check slave respond to data write

    TWI_stop();

    return 0;
}

uint8_t TWI_sniff (uint8_t start_addr, uint8_t end_addr, uint8_t* first_slave_addr) {
    if (start_addr > end_addr ||
        !first_slave_addr
    ) {
        return 0xFF;
    }

    uint8_t slave_cnt = 0;
    *first_slave_addr = 0;

    for (uint8_t i = start_addr; i < end_addr; i ++) {
        if (TWI_start_wait() != TW_START) return 0xFF;  //send Start bit, check for S bit error

        if (TWI_write_wait(i << 1) == TW_MT_SLA_ACK) {  // write to the slave address with W (write) condition, check response
            slave_cnt++;
            if (! *first_slave_addr) *first_slave_addr = i;
        }

        TWI_stop();
    }
    return slave_cnt;
}

ISR(TWI_vect) {
    TWI_cb_(TWI_cb_ctx_);
}
