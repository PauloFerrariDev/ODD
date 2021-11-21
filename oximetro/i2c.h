/*
 * i2c.h
 *
 *  Created on: May 23, 2019
 *      Author: samper
 */

#ifndef I2C_H_
#define I2C_H_

#include <msp430.h>
#include <stdint.h>


unsigned char *PTxData;                     // Pointer to TX data
unsigned char TXByteCtr;

void i2c_init(void); // Setup UCB1 for I2C
void i2c_write(unsigned char *, unsigned char); // write date to i2c bus

#define READ        0
#define WRITE       1

#define STOP_I2C        (UCB0CTLW0 |= UCTXSTP)

void initI2C_max (void);

void i2c_start_max (uint8_t,unsigned int);

void i2c_stop_max (void);

void i2c_repeated_start_max(uint8_t,unsigned int);

void i2c_write_max (uint8_t);//write data or reg_address

void i2c_read_max (uint8_t*,unsigned int);


#endif /* I2C_H_ */
