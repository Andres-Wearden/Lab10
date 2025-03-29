// TLV5616.c
// Runs on TM4C123
// Use SSI1 to send a 16-bit code to the TLV5616 and return the reply.
// Daniel Valvano
// EE445L Fall 2015
//    Jonathan W. Valvano 9/22/15

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// SSIClk (SCLK) connected to PD0
// SSIFss (FS)   connected to PD1
// SSITx (DIN)   connected to PD3

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

//----------------   DAC_Init     -------------------------------------------
// Initialize TLV5616 12-bit DAC
// assumes bus clock is 80 MHz
// inputs: initial voltage output (0 to 4095)
// outputs:none
void DAC_Init(void){
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0) == 0) {}
		GPIO_PORTA_AFSEL_R |= 0x2C;
		GPIO_PORTA_DIR_R |= 0x40;
		GPIO_PORTA_DEN_R |= 0x6C;
		GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFF0FF0FF) | 0x00202200;
		SSI0_CR1_R &= ~SSI_CR1_SSE;
    SSI0_CR1_R = 0;
    SSI0_CC_R = SSI_CC_CS_SYSPLL;
    SSI0_CPSR_R = 2;
    SSI0_CR0_R = (15 << SSI_CR0_SCR_S) | SSI_CR0_FRF_MOTO | SSI_CR0_DSS_16;
    SSI0_CR1_R |= SSI_CR1_SSE;
}

// --------------     DAC_Out   --------------------------------------------
// Send data to TLV5616 12-bit DAC
// inputs:  voltage output (0 to 4095)
// 
void DAC_Out(uint16_t code){
    while((SSI0_SR_R & SSI_SR_TNF) == 0) {
    }
    SSI0_DR_R = code;
}

// --------------     DAC_OutNonBlocking   ------------------------------------
// Send data to TLV5616 12-bit DAC without checking for room in the FIFO
// inputs:  voltage output (0 to 4095)
// 
void DAC_Out_NB(uint16_t code){
    // Consider writing this (If it is what your heart desires)
    // Consider the following registers:
	  // SSI1_SR_R, SSI1_DR_R
}
