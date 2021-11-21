// definições de timer para tempos
//
//
//
//

#include <msp430.h>
#include <stdint.h>
#include "ODD.h"

extern ProgramFlowControlFlags PFLOW ;
extern volatile uint16_t meiossegundos;
extern volatile uint16_t n10ms;
extern volatile uint16_t RETime; // para o Encoder rotativo. Atualizado em interrupção



void InitTimerA1(void)
{
    // timer A1 - > base de tempo para o sistema... interrompe a cada 20ms
    TA1CCR0 = 0; // DESLIGA TIMER
    TA1CTL =0;
    TA1CCTL1 = OUTMOD_3 ;  //011b = Set/reset
    TA1CCTL0 = CCIE; // ccr0 gera interrupção

//    TA1CTL |= ID_0 ;      // sem divisão de clock
    TA1CTL |= ID_2 ;      // com divisão de clock
    TA1CTL |= TASSEL_2 ;  //SMCLK (2MHz)
    TA1CTL |= MC_1;       //Up mode: Timer counts up to TAxCCR0
    TA1R=0;
    TA1CCR0 = 20000; // período de 10ms = 20,000/2,000,000
//    TA1CCR0 = 40000; // período de 20ms = 40,000/2,000,000
//    TA1CCR1 = 32000; // período

} // void InitTimers(void)




/*
 *  ======== Timer1_A0 Interrupt Service Routine ========
 *  interrompe a 20ms segundo para rotinas.
 *  sinaliza meio-segundo
 *  dispara leitura de ADC interno -
 *  necessita de meiossegundos e PFLOW definidos globalmente
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR_HOOK(void)
{
    /* USER CODE START (section: TIMER1_A0_ISR_HOOK) */
    static unsigned char n20ms=1;

    if( RETime< 2500 ) RETime++; // a ser zerado na interrupção da chave

    n10ms++; // definido globalmente
    if (n10ms & 0x02) n20ms++; // se for par... acrescenta
    PFLOW.Bit.t20ms=1;
 //   ADC12CTL0 |= ADC12SC;                     // Start conversion

    if(n20ms==25){ // a cada 25*20ms = 500ms...
        n20ms=1;
        meiossegundos++;
        PFLOW.Bit.t500ms=1;
    }
    /* USER CODE END (section: TIMER1_A0_ISR_HOOK) */
}


