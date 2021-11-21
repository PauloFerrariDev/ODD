//******************************************************************************
// baseado no
// MSP430F5529 SSD1306 OLED Display
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the MASTER CODE. It cntinuously
//  transmits an array of data and demonstrates how to implement an I2C
//  master transmitter sending multiple bytes using the USCI_B0 TX interrupt.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
//
//
//                                /|\  /|\
//                MSP430F5529     10k  10k      SSD1306 OLED
//                   master        |    |         Display
//             -----------------   |    |   -----------------
//           -|XIN  P4.1/UCB0SDA|<-|----+->|SDA              |-
//            |                 |  |       |                 |
//           -|XOUT             |  |       |                 |-
//            |     P4.2/UCB0SCL|<-+------>|SCL              |
//            |                 |          |                 |
//
//
//
// P2.0 -> output - alimentação MAX30102
// P1.7 -> output - controla (liga/desliga) 5V para leds do MAx30102
// P1.2 -> output - desliga a alimentação
// P6.0 -> output - controla medição de bateria
// P6.1 -> analog input -> medição ndo nível de bateria
//
//
//******************************************************************************

#include <msp430.h>
#include "ssd1306.h"
#include "i2c.h"
#include "clock.h"
#include "ODD.h"
#include "max30102.h"
//#include "algorithm.h"
#include "OxTimer.h"
#include "bitmaps.h"


extern unsigned char *PTxData;                  // Pointer to TX data, defined in i2c.h
extern unsigned char TXByteCtr;                 // number of bytes to transmit, defined in i2c.h


ProgramFlowControlFlags PFLOW ; // controle de fluxo do programa
volatile uint16_t meiossegundos; // contagem de tempos
volatile uint16_t n10ms;         // contagem de tempos
volatile uint16_t RETime; // para o Encoder rotativo. Atualizado em interrupção

// cálculo da oximetria
uint32_t aun_ir_buffer[100]; //infrared LED sensor data
uint32_t aun_red_buffer[100];  //red LED sensor data
int32_t n_ir_buffer_length; //data length
int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;




int main(void)
{
// ***** Start Initialization *****
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT

    P1DIR = BIT2 + BIT7;
//  P1OUT = BIT2;                             // mantém a alimentação
    KEEPALIVE;
    P2OUT = BIT0;
    P2DIR = BIT0;
    P6DIR = BIT0;


    clock_init();
    i2c_init();                                 // initialize UCB1 I2C, port 4 pins 1, 2
    ssd1306_init();                             // initialize SSD1306 OLED
    ssd1306_clearDisplay();                     // clear garbage data
    InitTimerA1();
    __bis_SR_register(GIE);


    PFLOW.Val=0;
    meiossegundos=0;
    n10ms=0;
    RETime = 0;
// ***** End Initialization  *****

// ************************** DISPLAY (START) ***********************

     uint16_t i=0;

     ssd1306_drawFullScreen(bitmap_UTFPR);

     RETime=0;
     while (RETime < 200 ); // aguarda 2000ms

     ssd1306_clearDisplay();

     ssd1306_drawFullScreen(bitmap_layout);

    i = 0;
    while (1) {
        // APAGAR A PARITR DA COLUNA 75 SEMPRE
//        ssd1306_clearFrame(0, 7, 75, 127);

        ssd1306_setFrame(0, 2, 74, 91);
        ssd1306_drawNumber(i);
        ssd1306_setFrame(0, 2, 92, 109);
        ssd1306_drawNumber(i);
        ssd1306_setFrame(0, 2, 110, 127);
        ssd1306_drawNumber(i);

        ssd1306_setFrame(5, 7, 74, 91);
        ssd1306_drawNumber(i);
        ssd1306_setFrame(5, 7, 92, 109);
        ssd1306_drawNumber(i);
        ssd1306_setFrame(5, 7, 110, 127);
        ssd1306_drawNumber(i);

        RETime = 0;
        while (RETime < 400); // aguarda 4000ms
        i++;
        if(i==10) i = 0;
    }

// ************************** DISPLAY (END) ***********************

     MAX_ON;
     RETime=0;
     while (RETime < 4 ); //aguarda 40ms
     initI2C_max();
     maxim_max30102_reset ();
     MAX5V_ON;
     RETime=0;
     while (RETime < 4 ); //aguarda 40ms
     maxim_max30102_init ();

     n_ir_buffer_length = 100;

     for(i = 0; i < n_ir_buffer_length;i++)
     {
         RETime=0;
         while (RETime < 2 ); //aguarda 10ms
         maxim_max30102_read_fifo((aun_red_buffer + i),(aun_ir_buffer + i));
     }

     //calc. from algorithm
      maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

      i2c_init();
      ssd1306_printUI32(0,2,n_spo2, HCENTERUL_ON);
      ssd1306_printUI32(0,4,n_heart_rate, HCENTERUL_ON);


     while(!PFLOW.Bit.DesligaTudo){

         // a cada 20ms
         if(PFLOW.Bit.t20ms){
             PFLOW.Bit.t20ms=0;
             //delete oldest 25 sample (0 - 24), and shift the samples to the left
              i = 25;
              for(i = 25; i < 100;i++)
              {
                  aun_ir_buffer[i - 25] = aun_ir_buffer[i];
                  aun_red_buffer[i - 25] = aun_red_buffer [i];
              }

              //add the latest 25 samples
              i = 75;
              for(i = 75; i < 100; i++)
              {
                  RETime=0;
                  while (RETime < 4 ); //aguarda 10ms

                  initI2C_max();
                  maxim_max30102_read_fifo((aun_red_buffer + i),(aun_ir_buffer + i));
              }

              //calc. hr and spo2
              maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);


              i2c_init();
              ssd1306_printUI32(0,2,n_spo2, HCENTERUL_ON);
              ssd1306_printUI32(0,4,n_heart_rate, HCENTERUL_ON);

         }



     }



     KEEPNOTALIVE;

    //__bis_SR_register(LPM0_bits + GIE);         // Enter LPM0, enable interrupts
    __no_operation();
}

