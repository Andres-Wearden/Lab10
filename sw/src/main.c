#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/UART.h"
#include "ADC.h"
#include "../inc/ADCT0ATrigger.h"
#include "../inc/CortexM.h"

extern uint32_t adcValue;
extern uint32_t adcValue2;
extern uint32_t adcValue3;

#define AVG_SAMPLES 16  // increase this for more smoothing

void UART1_Init(void){
    SYSCTL_RCGCUART_R |= (1<<1);            // enable UART1 clock
    SYSCTL_RCGCGPIO_R |= (1<<1);            // enable Port B clock
    while((SYSCTL_PRGPIO_R & (1<<1)) == 0){}

    // PB0 = U1Rx, PB1 = U1Tx
    GPIO_PORTB_AFSEL_R |= 0x03;
    GPIO_PORTB_PCTL_R  = (GPIO_PORTB_PCTL_R & ~0xFF)
                       | (1<<0)   // PB0 ? U1RX
                       | (1<<4);  // PB1 ? U1TX
    GPIO_PORTB_DEN_R   |= 0x03;
    GPIO_PORTB_DIR_R   |=  (1<<1);
    GPIO_PORTB_DIR_R   &= ~(1<<0);

    // 115200 baud, 8-N-1 @ 80 MHz
    UART1_CTL_R &= ~UART_CTL_UARTEN;
    UART1_CC_R   = 0;    // system clock
    UART1_IBRD_R = 43;   // int(80e6/(16×115200))
    UART1_FBRD_R = 26;   // frac(.40×64 + .5)
    UART1_LCRH_R = (3<<5);
    UART1_CTL_R |= (UART_CTL_UARTEN | UART_CTL_TXE);
}

void UART1_Send16(uint16_t value){
    // send high byte
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = (value >> 8) & 0xFF;
    // send low byte
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = value & 0xFF;
}

int main(void){
    DisableInterrupts();
    PLL_Init(Bus80MHz);
//    ADC0_InitTimer0ATriggerSeq0(0, 1000, ProcessADCData); // AIN0
//    ADC0_InitTimer0ATriggerSeq0(1, 1000, ProcessADCData); // AIN1
    ADC0_InitTimer0ATriggerSeq0(3, 1000, ProcessADCData); // AIN3
    UART_Init();   // for debug over UART0
    UART1_Init();  // TX averaged data on UART1
    EnableInterrupts();

    uint32_t sum1 = 0, sum2 = 0, sum3 = 0;
    uint32_t count = 0;

    while(1){
        // accumulate latest ADC readings
        sum1 += (adcValue  & 0x0FFF);
        sum2 += (adcValue2 & 0x0FFF);
        sum3 += (adcValue3 & 0x0FFF);
        count++;

        if(count >= AVG_SAMPLES){
            // compute averaged values
            uint16_t avg1 = sum1 / AVG_SAMPLES;
            uint16_t avg2 = sum2 / AVG_SAMPLES;
            uint16_t avg3 = sum3 / AVG_SAMPLES;

            // optional: debug print to console
            UART_OutString("Avg ADCs: ");
            UART_OutUDec(avg1); UART_OutString(", ");
            UART_OutUDec(avg2); UART_OutString(", ");
            UART_OutUDec(avg3); UART_OutString("\r\n");

            // send each 12-bit average over UART1
            UART1_Send16(avg1);
            UART1_Send16(avg2);
            UART1_Send16(avg3);

            // reset for next block
            sum1 = sum2 = sum3 = 0;
            count = 0;
        }
    }
}
