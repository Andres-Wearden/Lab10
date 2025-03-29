#ifndef __ADC__
#define __ADC__
#include <stdint.h>
void ADC_Init(void);
void ProcessADCData(uint32_t data);
extern uint32_t adcValue;
extern uint32_t previousADC;
extern uint32_t noiseSum;
extern uint32_t noiseCount;

#define BUFSIZE 1000
extern uint32_t TimeBuf[BUFSIZE]; // in bus cycles
extern uint32_t DataBuf[BUFSIZE]; // 0 to 4095 assuming constant analog input
extern volatile uint32_t Num;     // index from 0 to BUFSIZE-1

// time jitter variables
extern uint32_t MinT;    // minimum(TimeBuf[i-1] – TimeBuf[i]) for i equals 1 to BUFSIZE-1
extern uint32_t MaxT;    // maximum(TimeBuf[i-1] – TimeBuf[i]) for i equals 1 to BUFSIZE-1
extern uint32_t Jitter;  // MaxT – MinT (in bus cycles)
extern uint16_t Periods[256];  // histogram of times between ADC triggers, optional

// SNR variables
extern uint32_t Averaging; // 1,2,4,8,16,32, or 64 to student CLT
extern uint32_t Vmin, Vmax, PMFmax;
extern int32_t Signal,Noise,SNR,Distance;
extern uint16_t PMF[100];  // histogram of ADC samples
extern uint32_t adcValue;

#endif