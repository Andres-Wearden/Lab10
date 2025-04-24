#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "Timers.h"
#include "../inc/CortexM.h"
#include "ADC.h"
#include "../inc/ADCT0ATrigger.h"
#include "../inc/UART.h"

extern uint32_t adcValue;
extern uint32_t adcValue2;
extern uint32_t adcValue3;

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

    // 115200, 8-N-1 @ 80 MHz
    UART1_CTL_R &= ~UART_CTL_UARTEN;
    UART1_CC_R   = 0;
    UART1_IBRD_R = 43;
    UART1_FBRD_R = 26;
    UART1_LCRH_R = (3<<5);
    UART1_CTL_R |= (UART_CTL_UARTEN|UART_CTL_TXE);
}

// send a single character
void UART1_SendChar(char c){
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = c;
}

// send a 16-bit word over UART1 (MSB first)
void UART1_Send16(uint16_t value){
    // high byte
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = (value >> 8) & 0xFF;
    // low byte
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = value & 0xFF;
}

int main(void){
    DisableInterrupts();
    PLL_Init(Bus80MHz);
    ADC0_InitTimer0ATriggerSeq0(0, 1000, ProcessADCData);  // configure ADC0 if not already
    UART_Init();
		UART1_Init();
    EnableInterrupts();

    while(1){
				UART_OutString("ADCValue:          ");
				UART_OutUDec(adcValue);
				UART_OutString("\r\n");
			
				UART_OutString("ADCValue2:          ");
				UART_OutUDec(adcValue2);
				UART_OutString("\r\n");
			
				UART_OutString("ADCValue3:          ");
				UART_OutUDec(adcValue3);
				UART_OutString("\r\n");
        // Mask each ADC value to 12 bits and send them sequentially
        UART1_Send16((uint16_t)(adcValue  & 0x0FFF));
        UART1_Send16((uint16_t)(adcValue2 & 0x0FFF));
        UART1_Send16((uint16_t)(adcValue3 & 0x0FFF));
        
        // small pause between bursts (adjust as needed)
        for(volatile int i = 0; i < 500000; i++);
    }
}
