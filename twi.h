#ifndef TWI_TOOLS_H_
#define TWI_TOOLS_H_

#include <stdint.h>

typedef enum TWI_clock_source_t {
    TWI_PS_1 = 0 ,
    TWI_PS_4,
    TWI_PS_16,
    TWI_PS_64
} TWI_clock_source;

/**
 * TWI interrupt callback type
 * @param[inout] ctx user data for interrupt callback
 * When ISR occurs TWI_isr_cb will be called with ctx as parameter
 */
typedef void (*TWI_isr_cb)(void* ctx);

/**
 * Register callback and context for TWI interrupt
 * @param[in] cb   callback for isr; must not be NULL
 * @param[in] ctx  user defined callback context; must not be NULL
 * 
 */
void regiter_TWI_isr_cb(TWI_isr_cb cb, void* ctx);

/**
 * Initialize SPI as master device
 * set SCL to F_CPU / (16 + 2 * TWBR * prescaler)
 * @param [in] clk_src   select clock prescaler for SCL frequency, see TWI_clock_source
 * @param [in] bit_rate  select bit rate for SCL frequency
 * @param [in] en_isr    enable TWI data ready interrupt
 *
 * @return 0 for success
 *         other in case of fail
 */
uint8_t TWI_init (TWI_clock_source clk_src, uint8_t bit_rate, uint8_t en_isr);

/**
 * Send start bit condition through TWI interface:
 * SCL is high while SDA falls
 * @return TWI status register
 */
uint8_t TWI_start_wait();

/**
 * send stop bit condition through TWI interface: 
 * SCL is high while SDA rises
 */
void TWI_stop();

/**
 * write 8 bit data trough TWI interface into slave device
 * wait until transfer is finished
 * @param [in] data 1B data to be sent
 *
 * @return TWI status register
 */
uint8_t TWI_write_wait(uint8_t data);

/**
 * Read out a 8 bit data from slave through TWI interface
 * wait until the tranfer is completed.
 * Data will appear in TWDR register.
 * After the 8 bit data is received , ACK is sent to slave device.
 * @param [in] data the value from TWDR will be saved in data
 *
 * @return TWI status register
 */
uint8_t TWI_read_ACK(uint8_t* data);

/**
 * Read out a 8 bit data from slave through TWI interface
 * wait until the tranfer is completed.
 * Data will appear in TWDR register.
 * After the 8 bit data is received , ACK is NOT sent to slave device.
 * @param [in] data the value from TWDR will be saved in data
 *
 * @return TWI status register
 */
uint8_t TWI_read_NACK_wait(uint8_t* data);

/**
 * Scan TWI addresses start_addr - end_addr to find slaves
 * @param [in] start_addr  start address for sniffing
 * @param [in] end_addr  end address for sniffing
 * @param [out] first_slave_addr  first slave address found while searching
 *
 * @return total number of slaves found
 *         0xFF indicates error
 */
uint8_t TWI_sniff(uint8_t start_addr, uint8_t end_addr, uint8_t* first_slave_addr);

uint8_t TWI_read_reg_burst (
    uint8_t *buffer,
    uint8_t slave_addr,
    uint8_t internal_addr,
    uint8_t size);

uint8_t TWI_write_reg (
    uint8_t slave_addr,
    uint8_t internal_addr,
    const uint8_t data
);
#endif /* TWI_TOOLS_H_ */
