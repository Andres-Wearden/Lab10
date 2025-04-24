#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ADC.h"
#include "../inc/ADCT0ATrigger.h"
#include <string.h>
#include "../inc/UART2.h"
#include <stdio.h>


uint32_t adcValue;
//char b2w_buf[64];


uint32_t adcValue;
uint32_t adcValue2;
uint32_t adcValue3;


void ProcessADCData(uint32_t data1, uint32_t data2, uint32_t data3) {
    adcValue = data1;
    adcValue2 = data2;
    adcValue3 = data3;
}
