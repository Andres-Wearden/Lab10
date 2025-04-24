#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ADC.h"
#include "../inc/ADCT0ATrigger.h"

uint32_t adcValue;


void ProcessADCData(uint32_t data) {
	adcValue = data;
	if(Num < BUFSIZE){
		TimeBuf[Num] = TIMER1_TAR_R;
		DataBuf[Num] = adcValue;
		Num++;
	}
}
