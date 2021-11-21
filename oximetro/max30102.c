/*
 * max30102.c
 *
 *  Created on: 15 de nov de 2021
 *      Author: gzane
 */


#include <msp430.h>
#include <stdint.h>
#include "max30102.h"
#include "i2c.h"
#include "max30102.h"

void maxim_max30102_reset (void)
{
    //S SLA W ACK 0x09 ACK 0x40 ACK P
    //shutdown IC for LPM

    i2c_start_max (MAX_SLAVE_ADDR,WRITE);

    i2c_write_max(REG_MODE_CONFIG);

    i2c_write_max(0x40);//shut down IC for LPM usage

    i2c_stop_max ();
}

void maxim_max30102_init (void)
{
    uint8_t byte_to_store [11] = {0x00,0x00,0x00,0x00,0x00,0x4F,0x03,0x27,0x24,0x24,0x7F};
    /*
    reg_intr_enable_1 0x02 disable isr
    reg_intr_enable_2 0x03 disable isr
    fifo_wr_ptr       0x04 clear reg
    reg_ovf_counter   0x05 clear reg
    fifo_rd_ptr       0x06 clear reg

    fifo_data         0x07

    fifo_config       0x08  4 sample averaged / sample
    mode_config       0x09  spo2 mode ir led + red + led
    spo2_config       0xA0  spo2 adc range 4096nA, sample rate 100 Hz, LED PW 411us
    led1_pa           0xA1  iLED1 = 7mA
    led2_pa           0xA2  iLED2 = 7mA
    pilot_pa          0x7F  i pilot led = 25mA
    */

    i2c_start_max(MAX_SLAVE_ADDR,WRITE);
    i2c_write_max(REG_INTR_ENABLE_1);//write this register

    int i = 0;
    for(i = 0; i < 5; i++)
        i2c_write_max(byte_to_store[i]);

    i2c_stop_max ();
    //when i = 5 it is fifo data

    i2c_start_max(MAX_SLAVE_ADDR,WRITE);
    i2c_write_max(REG_FIFO_CONFIG);

    i = 5;
    for(i = 5; i < 11;i++)
        i2c_write_max(byte_to_store[i]);

    i2c_stop_max ();
}

void maxim_max30102_read_fifo (uint32_t* ptr_red_led, uint32_t * ptr_ir_led)
{
    uint32_t un_temp;

    *ptr_ir_led = 0;
    *ptr_red_led = 0;

    i2c_start_max(MAX_SLAVE_ADDR,WRITE);
    i2c_write_max(REG_FIFO_DATA);

    //i2c_start(SLAVE_ADDR,READ);
    i2c_repeated_start_max(MAX_SLAVE_ADDR,READ);

    uint8_t led[6] = {0,0,0,0,0,0};
    unsigned int RxByteCtr = 6;

    i2c_read_max(led,RxByteCtr);

    un_temp = led[0];
    un_temp <<= 16;
    *ptr_red_led += un_temp;

    un_temp = led[1];
    un_temp <<= 8;
    *ptr_red_led += un_temp;

    un_temp = led[2];
    *ptr_red_led += un_temp;

    un_temp = 0;

    un_temp = led[3];
    un_temp <<= 16;
    *ptr_ir_led += un_temp;

    un_temp = led[4];
    un_temp <<= 8;
    *ptr_ir_led += un_temp;

    un_temp = led[5];
    *ptr_ir_led += un_temp;
}
