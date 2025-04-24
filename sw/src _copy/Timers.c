#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Timers.h"
#include "../inc/Timer1A.h"

void Heartbeat(void){
	GPIO_PORTF_DATA_R ^= 0x02;
}

void PortF_Red_Led_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x20; // initialize port F Clock
	while((SYSCTL_PRGPIO_R & 0x20) == 0){} //wait for F clock to stabalize
	GPIO_PORTF_DIR_R |= 0x02; //set PF1 to output
	GPIO_PORTF_DEN_R |= 0x02; // enable digital funciton on PF1
}

void Heartbeat_Init(uint32_t period, uint32_t priority){
	Timer1A_Init(Heartbeat, period, priority);
	PortF_Red_Led_Init();
}
