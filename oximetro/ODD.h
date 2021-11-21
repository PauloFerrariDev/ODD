/*
 * ODD.h
 *
 *  Created on: 15 de nov de 2021
 *      Author: gzane
 */


#ifndef ODD_H_
#define ODD_H_

#include <stdint.h>

#define KEEPALIVE P1OUT |= BIT0
#define KEEPNOTALIVE P1OUT &= ~BIT0

#define MAX_ON  P2OUT |= BIT0;
#define MAX_OFF P2OUT &= ~BIT0;

#define MAX5V_ON   P1OUT |= BIT7
#define MAX5V_OFF  P1OUT &= ~BIT7


// esta estrutura precisa ser global para interação com interrupções
typedef volatile struct {
    volatile unsigned short t20ms     : 1;  // Modificado em interrupção - marca contador de 20ms
    volatile unsigned short t500ms    : 1 ;  //
    volatile unsigned short tlimite   : 1 ;  // tempo limite ligado
    volatile unsigned short MaxBuffer :1 ;  //

    volatile unsigned short BPMfinalizado :1 ;  //
    volatile unsigned short OxFinalizado  :1 ;  //
    volatile unsigned short  :1 ;  //
    volatile unsigned short DesligaTudo :1 ;  //

    volatile uint8_t MaxValores  ;  //
//        volatile unsigned short  ;  //
}ProgramFlowControlBit  ;


typedef volatile union {
    volatile uint16_t Val;
    ProgramFlowControlBit Bit  ;
} ProgramFlowControlFlags ; //PFLOW ; // PFLOW


#endif /* ODD_H_ */
