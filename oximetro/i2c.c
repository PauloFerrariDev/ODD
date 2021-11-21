/*
 * i2c.c
 *
 *  Created on: May 23, 2019
 *      Author: samper
 */


#include <msp430.h>
#include <stdint.h>
#include "i2c.h"


#define SDA BIT1                                // i2c SDA pin on port 4
#define SCL BIT2                                // i2c SCL pin on port 4




void initI2C_max()
{
    P4SEL |= SDA | SCL;                         // Assign I2C pins to USCI_B1
    UCB1CTLW0 = UCSWRST;                      // Enable SW reset
    UCB1CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK | UCSYNC; // I2C master mode, SMCLK
    UCB1BRW = 160;                            // fSCL = SMCLK/10 = ~100kHz
//    UCB1I2CSA = MAX_SLAVE_ADDR;                   // Slave Address
    UCB1CTLW0 &= ~UCSWRST;                    // Clear SW reset, resume operation
    UCB1IE |= UCNACKIE;//enable NACK ISR (TX and RX?)
}

void i2c_start_max (uint8_t dev_addr,unsigned int RW)
{
    while(UCB1STAT & UCBBUSY);//check if SDA and SCL are idle

    UCB1I2CSA = dev_addr;

    if(RW == READ){UCB1CTLW0 &= ~UCTR;}
    else{UCB1CTLW0 |= UCTR;}

    UCB1CTLW0 |= UCTXSTT;

    while (UCB1CTLW0 & UCTXSTT);//wait till the whole address has been sent and ACK'ed
}

void i2c_stop_max (void)
{
    UCB1CTLW0 |= UCTXSTP;//stop
    while(UCB1CTLW0 & UCTXSTP);//wait for a stop to happen
}

void i2c_repeated_start_max(uint8_t dev_addr, unsigned int RW)
{
    UCB1I2CSA = dev_addr;

    if(RW == READ){UCB1CTLW0 &= ~UCTR;}
    else{UCB1CTLW0 |= UCTR;}

    UCB1CTLW0 |= UCTXSTT;

    while (UCB1CTLW0 & UCTXSTT);//wait till the whole address has been sent and ACK'ed
}

void i2c_write_max (uint8_t data)
{
    UCB1TXBUF = data;

    while(UCB1CTLW0 & UCTXIFG);//1 means data is sent completely
}

void i2c_read_max (uint8_t * led,unsigned int RxByteCtr)
{
    int i = 0;
    for(i = 0; i < RxByteCtr;i++)
    {
        while(!(UCB1IFG & UCRXIFG));//make sure rx buffer got data

        if(i == RxByteCtr - 1)
            STOP_I2C;

        led[i] = UCB1RXBUF;
    }

    while(UCB1CTLW0 & UCTXSTP);//wait for a stop to happen
}




void i2c_init(void) {
    P4SEL |= SDA | SCL;                         // Assign I2C pins to USCI_B1
    UCB1CTL1 |= UCSWRST;                        // Enable SW reset
    UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;       // I2C Master, synchronous mode
    UCB1CTL1 = UCSSEL_2 + UCSWRST;              // Use SMCLK=24MHz, keep SW reset
    UCB1BR0 = 160;                               // fSCL = SMCLK/96 = ~300kHz 64 - 400kHz
    UCB1BR1 = 0;                                // UCBRx = (UCxxBR0 + UCxxBR1 * 256) -> fSCL = SMCLK/USBRx
    UCB1I2CSA = 0x3C;                           // Slave Address is 0x3C
    UCB1CTL1 &= ~UCSWRST;                       // Clear SW reset, resume operation
    UCB1IE |= UCTXIE;                           // Enable TX interrupt
} // end i2c_init

void i2c_write(unsigned char *DataBuffer, unsigned char ByteCtr) {
    //__delay_cycles(10);                         // small wait
    UCB1I2CSA = 0x3C;                           // Slave Address is 0x3C
    PTxData = DataBuffer;                       // TX array start address
                                                // Place breakpoint here to see each
                                                // transmit operation.
    TXByteCtr = ByteCtr;                        // Load TX byte counter

    UCB1CTL1 |= UCTR + UCTXSTT;                 // I2C TX, start condition

    __bis_SR_register(LPM0_bits + GIE);         // Enter LPM0, enable interrupts
    __no_operation();                           // Remain in LPM0 until all data
                                                // is TX'd
    while (UCB1CTL1 & UCTXSTP);                 // Ensure stop condition got sent
} // end i2c_write




//------------------------------------------------------------------------------
// The USCIAB1TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB1IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: break;                           // Vector  4: NACKIFG
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 10: break;                           // Vector 10: RXIFG
  case 12:                                  // Vector 12: TXIFG
    if (TXByteCtr)                          // Check TX byte counter
    {
      UCB1TXBUF = *PTxData++;               // Load TX buffer
      TXByteCtr--;                          // Decrement TX byte counter
    }
    else
    {
      UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
      UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B1 TX int flag
      __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
    }
  default: break;
  }
}
