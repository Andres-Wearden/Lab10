#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/UART.h"
#include "ADC.h"
#include "../inc/ADCT0ATrigger.h"
#include "../inc/CortexM.h"
#include "../inc/UART1.h"

extern uint32_t adcValue;
extern uint32_t adcValue2;
extern uint32_t adcValue3;

#define AVG_SAMPLES 16  // increase this for more smoothing

void UART1_Send16(uint16_t value){
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = (value >> 8) & 0xFF;
    while(UART1_FR_R & UART_FR_TXFF){}
    UART1_DR_R = value & 0xFF;
}

int main(void){
	DisableInterrupts();
	PLL_Init(Bus80MHz);
	//    ADC0_InitTimer0ATriggerSeq0(0, 1000, ProcessADCData); // AIN0
	//    ADC0_InitTimer0ATriggerSeq0(1, 1000, ProcessADCData); // AIN1
	ADC0_InitTimer0ATriggerSeq0(3, 1000, ProcessADCData); // AIN3
	UART_Init();
	UART1_Init(115200);
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
			uint16_t avg1 = sum1 / AVG_SAMPLES;
			uint16_t avg2 = sum2 / AVG_SAMPLES;
			uint16_t avg3 = sum3 / AVG_SAMPLES;
			
			UART_OutString("Avg ADCs: ");
			UART_OutUDec(avg1); UART_OutString(", ");
			UART_OutUDec(avg2); UART_OutString(", ");
			UART_OutUDec(avg3); UART_OutString("\r\n");

			UART1_Send16(avg1);
			UART1_Send16(avg2);
			UART1_Send16(avg3);

			sum1 = sum2 = sum3 = 0;
			count = 0;
		}
	}
}
