#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/UART0int.h"
#include "../inc/CortexM.h"
#include "PWMIND.h"

#define AVG_SAMPLES 1  // number of readings to average

uint16_t motor_duty = 160;

//——— Port F (PF4) switch init —————————————————————————
void PortF_Init(void){
    SYSCTL_RCGCGPIO_R |= (1<<5);               
    while((SYSCTL_PRGPIO_R & (1<<5)) == 0){}   

    GPIO_PORTF_LOCK_R = 0x4C4F434B;            
    GPIO_PORTF_CR_R   = 0x10;                  
    GPIO_PORTF_AMSEL_R &= ~0x10;               
    GPIO_PORTF_PCTL_R  &= ~0x000F0000;         
    GPIO_PORTF_DIR_R   &= ~0x10;               
    GPIO_PORTF_PUR_R   |=  0x10;               
    GPIO_PORTF_DEN_R   |=  0x10;               
}

//——— UART2 helpers on PD6/PD7 ——————————————————————————
void UART2_Init(uint32_t baud) {
  uint32_t brd, remainder;
  
  // Enable UART2 clock (bit 2) and wait until it is ready.
  SYSCTL_RCGCUART_R |= 0x04;
  while ((SYSCTL_PRUART_R & 0x04) == 0) {};
  
  // Enable clock for Port D (bit 3) and wait until ready.
  SYSCTL_RCGCGPIO_R |= 0x08;
  while ((SYSCTL_PRGPIO_R & 0x08) == 0) {};
  
  // Unlock PD7 for reconfiguration (PD7 is a locked NMI pin) and allow changes on PD7 and PD6.
  GPIO_PORTD_LOCK_R = 0x4C4F434B;      // Unlock Port D
  GPIO_PORTD_CR_R |= 0xC0;             // Commit PD6 and PD7
  
  // Disable analog functionality on PD6 and PD7.
  GPIO_PORTD_AMSEL_R &= ~0xC0;
  
  // Enable alternate function on PD6 (U2Rx) and PD7 (U2Tx).
  GPIO_PORTD_AFSEL_R |= 0xC0;
  
  // Set port control to assign PD6 and PD7 to UART2.
  // Clear previous PCTL settings for these pins and set to 0x1 for UART.
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R & 0x00FFFFFF) | 0x11000000;
  
  // Enable digital functionality on PD6 and PD7.
  GPIO_PORTD_DEN_R |= 0xC0;
  
  // Disable UART2 while configuring.
  UART2_CTL_R &= ~UART_CTL_UARTEN;
  
  // Calculate baud rate divisor:
  //   BRD = System Clock / (16 * baud)
  brd = 80000000 / (16 * baud);
  remainder = 80000000 % (16 * baud);
  UART2_IBRD_R = brd;
  UART2_FBRD_R = ((remainder * 64) + (baud / 2)) / baud;
  
  // Set line control for 8-bit, no parity, 1 stop bit, and enable FIFOs.
  UART2_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);
  
  // Clear FIFO interrupt level fields (optional if not using interrupts).
  UART2_IFLS_R &= ~0x3F;
  
  // Enable UART2.
  UART2_CTL_R |= UART_CTL_UARTEN;
}

char UART2_RecvChar(void){
    while(UART2_FR_R & UART_FR_RXFE){}
    return (char)(UART2_DR_R & 0xFF);
}

int main(void){
    uint32_t sum1 = 0, sum2 = 0, sum3 = 0;
    uint8_t count = 0;
    uint8_t lastSw = 0, sw;

    DisableInterrupts();
    PLL_Init(Bus80MHz);
//    PWM_PB6_Init();
    PWM_PD0_Init();
    PWM0_PD0_SetPeriod(25000);
    PWM0_PB6_SetPeriod(1600);
    PWM0_PB6_SetDuty(160);
    PortF_Init();
    UART2_Init(115200);
    UART_Init();
    EnableInterrupts();

    UART_OutString("UART2 on PD6/PD7; receiving 3×12-bit values, averaging...\r\n");

    while(1){
			PWM0_PB6_SetDuty(160);
//        // Only read when at least 6 bytes are available
//				// simpler: just try to read 6 times, but safe assume data comes in order
//				// Read 3×(hi,lo)
//			uint8_t lo1 = (uint8_t) UART2_RecvChar();	
//			uint8_t hi1 = (uint8_t) UART2_RecvChar();
////            char hi2 = UART2_RecvChar();
////            char lo2 = UART2_RecvChar();
////            char hi3 = UART2_RecvChar();
////            char lo3 = UART2_RecvChar();

//				uint16_t v1 = ((((uint16_t)hi1)<<8)| lo1) & 0x0FFF;
////            uint16_t v2 = ((((uint16_t)hi2)<<8)| (uint8_t)lo2) & 0x0FFF;
////            uint16_t v3 = ((((uint16_t)hi3)<<8)| (uint8_t)lo3) & 0x0FFF;

//				sum1 += v1;
////            sum2 += v2;
////            sum3 += v3;
//				count++;

//				if(count >= AVG_SAMPLES){
//						// compute averages
//						uint16_t avg1 = sum1 / AVG_SAMPLES;
////                uint16_t avg2 = sum2 / AVG_SAMPLES;
////                uint16_t avg3 = sum3 / AVG_SAMPLES;

//						// print averaged values
//						UART_OutString("Avg1="); UART_OutUDec(avg1);
////                UART_OutString("  Avg2="); UART_OutUDec(avg2);
////                UART_OutString("  Avg3="); UART_OutUDec(avg3);
//						UART_OutString("\r\n");

//						// update PWM outputs
//						uint32_t duty0 = 100 + (avg1 * 1900) / 4095;
//						//UART_OutUDec(duty0);
//						PWM0_PD0_Duty(duty0);
//						// 2350 -> 4095 (accelator adc, duty1), 2690 -> 4092 (brake adc, duty2)
////						uint32_t duty1 = avg1; 
////						if (2800 < duty1 < 3200) { // accelator not pressed
////							if (motor_duty > 160) {
////								motor_duty--;
////							}
////						} else if (3200 <= duty1) {
////							if (motor_duty < 1280) {
////								motor_duty++;
////							}
////						}
//						
////								uint32_t duty2 = avg3;
////                
////								if ((2350 < duty1 < 2550) && (duty2 < 2890)) { // accelator not pressed
////									if (motor_duty >= 160) {
////										motor_duty--;
////									}
////								} else if ((2550 < duty1 < 3500) && (duty2 < 2890)) { // accelarator not completely pressed
////									if (motor_duty <= 1280) {
////										motor_duty += 2;
////									}
////								} else if ((duty1 >= 3500) && (duty2 < 2890)) { // floored that shit
////									if (motor_duty <= 1280) {
////										motor_duty += 4;
////									}
////								}
////								
////								if (2890 < duty2 < 3500) {
////									if (motor_duty >= 160) {
////										motor_duty -= 2;
////									}
////								} else if (3500 <= duty2) {
////									if (motor_duty >= 160) {
////										motor_duty -= 4;
////									}
////								}
//						
////						PWM0_PB6_SetDuty(motor_duty);
//						// reset accumulators
//						sum1 = sum2 = sum3 = 0;
//						count = 0;
//				}
    }
}
