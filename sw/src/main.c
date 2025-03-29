#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "Timers.h"
#include "../inc/CortexM.h"
#include "ADC.h"
#include "../inc/ADCT0ATrigger.h"
#include "../inc/UART.h"
#include "../inc/Unified_Port_Init.h"
uint32_t gearShift = 0;

#define BUFSIZE 1000
uint32_t TimeBuf[BUFSIZE]; // in bus cycles
uint32_t DataBuf[BUFSIZE]; // 0 to 4095 assuming constant analog input
volatile uint32_t Num;     // index from 0 to BUFSIZE-1

// time jitter variables
uint32_t MinT;    // minimum(TimeBuf[i-1] – TimeBuf[i]) for i equals 1 to BUFSIZE-1
uint32_t MaxT;    // maximum(TimeBuf[i-1] – TimeBuf[i]) for i equals 1 to BUFSIZE-1
uint32_t Jitter;  // MaxT – MinT (in bus cycles)
uint16_t Periods[256];  // histogram of times between ADC triggers, optional

// SNR variables
uint32_t Averaging; // 1,2,4,8,16,32, or 64 to student CLT
uint32_t Vmin, Vmax, PMFmax;
int32_t Signal,Noise,SNR,Distance;
uint16_t PMF[100];  // histogram of ADC samples
uint32_t ADCvalue;

uint32_t sqrt2(uint32_t s){ int n; // loop counter
uint32_t t;            // t*t will become s
  t = s/16+1;          // initial guess
  for(n = 16; n; --n){ // will finish
    t = ((t*t+s)/t)/2;
  }
  return t;
}

void CalculateSNR(void){
  Signal = 0;
  Noise = 0;
  SNR = 0;
  for(int i = 0; i < BUFSIZE; i++){
    Signal += DataBuf[i];
  }
  Signal = Signal / BUFSIZE;
  
  uint32_t sumSqDiff = 0;
  for(int i = 0; i < BUFSIZE; i++){
    int32_t diff = DataBuf[i] - Signal;
    sumSqDiff += diff * diff;
  }
  // Using sample variance (divide by BUFSIZE - 1)
  Noise = sumSqDiff / (BUFSIZE - 1);
  Noise = sqrt2(Noise);
	UART_OutString("\r\nNoise = ");
	UART_OutUDec(Noise);
  SNR = Signal / (Noise ? Noise : 1);  // avoid division by zero
}

int main(void){
    DisableInterrupts();
    PLL_Init(Bus80MHz);
    Heartbeat_Init(5000000, 7);
    ADC0_InitTimer0ATriggerSeq0(3, 1000, ProcessADCData);
    UART_Init();
    Port_C_Init();
		ADC0_SAC_R = 0x06;
    EnableInterrupts();
    uint32_t noiseLevel = 0;

    while(1){
        // Print ADC value
				UART_OutString("ADCValue");
        UART_OutUDec(adcValue);
        UART_OutString("\r\n");

        // Print current gear shift
        UART_OutString("Current Gear Shift: ");
        UART_OutUDec(gearShift);
        UART_OutString("\r\n");

//        while(Num < BUFSIZE){ 
//				}
				
				Num = 0;
				
				CalculateSNR();
				
				UART_OutString("\r\nSNR = ");
				UART_OutUDec(SNR);
				
				

        // Process gear shift inputs (same as your code)
        if((GPIO_PORTC_DATA_R & 0x10) == 0x00){
            while((GPIO_PORTC_DATA_R & 0x10) == 0x00){}
            if(gearShift == 5){
                gearShift = 5;
            }else{
                gearShift++;
            }
        } else if ((GPIO_PORTC_DATA_R & 0x20) == 0x00){
            while((GPIO_PORTC_DATA_R & 0x20) == 0x00){}
            if(gearShift == 0){
                gearShift = 0;
            }else{
                gearShift--;
            }
        }
    }
}

