#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/UART0int.h"
#include "../inc/CortexM.h"
#include "PWMIND.h"

#define AVG_SAMPLES 1  // number of readings to average

uint16_t motor_duty = 10;

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
void UART2_Init(void){
    SYSCTL_RCGCUART_R |= (1<<2);
    SYSCTL_RCGCGPIO_R |= (1<<3);
    while((SYSCTL_PRGPIO_R & (1<<3)) == 0){}

    // PD6 = U2Rx, PD7 = U2Tx
    GPIO_PORTD_AFSEL_R |= (1<<6)|(1<<7);
    GPIO_PORTD_PCTL_R  = (GPIO_PORTD_PCTL_R & ~0xFF000000)
                       | (1<<24)  // PD6 ? U2RX
                       | (1<<28); // PD7 ? U2TX
    GPIO_PORTD_DEN_R   |= (1<<6)|(1<<7);
    GPIO_PORTD_DIR_R   |=  (1<<7);  // PD7 output
    GPIO_PORTD_DIR_R   &= ~(1<<6);  // PD6 input

    UART2_CTL_R &= ~UART_CTL_UARTEN;
    UART2_CC_R   = 0;
    UART2_IBRD_R = 43;
    UART2_FBRD_R = 26;
    UART2_LCRH_R = (3<<5);
    UART2_CTL_R |= (UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE);
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
//    PWM_PD0_Init();
//    PWM0_PD0_SetPeriod(25000);
//    PWM0_PB6_SetPeriod(1600);
//    PWM0_PB6_SetDuty(200);
    PortF_Init();
    UART2_Init();
    UART_Init();
    EnableInterrupts();

    UART_OutString("UART2 on PD6/PD7; receiving 3×12-bit values, averaging...\r\n");

    while(1){
        // Only read when at least 6 bytes are available
            char hi1 = UART2_RecvChar();
            char lo1 = UART2_RecvChar();
            char hi2 = UART2_RecvChar();
            char lo2 = UART2_RecvChar();
            char hi3 = UART2_RecvChar();
            char lo3 = UART2_RecvChar();

            uint16_t v1 = ((((uint16_t)hi1)<<8)| (uint8_t)lo1) & 0x0FFF;
            uint16_t v2 = ((((uint16_t)hi2)<<8)| (uint8_t)lo2) & 0x0FFF;
            uint16_t v3 = ((((uint16_t)hi3)<<8)| (uint8_t)lo3) & 0x0FFF;

            sum1 += v1;
            sum2 += v2;
            sum3 += v3;
            count++;

            if(count >= AVG_SAMPLES){
                // compute averages
                uint16_t avg1 = sum1 / AVG_SAMPLES;
                uint16_t avg2 = sum2 / AVG_SAMPLES;
                uint16_t avg3 = sum3 / AVG_SAMPLES;

                // print averaged values
                UART_OutString("Avg1="); UART_OutUDec(avg1);
                UART_OutString("  Avg2="); UART_OutUDec(avg2);
                UART_OutString("  Avg3="); UART_OutUDec(avg3);
                UART_OutString("\r\n");

                // update PWM outputs
                uint32_t duty0 = 100 + (avg1 * 1900) / 4095;
//								PWM0_PD0_Duty(duty0);
                // 2350 -> 4095 (accelator adc, duty1), 2690 -> 4092 (brake adc, duty2)
								uint32_t duty1 = avg2; 
								uint32_t duty2 = avg3;
                
								if ((2350 < duty1 < 2550) && (duty2 < 2890)) { // accelator not pressed
									if (motor_duty >= 10) {
										motor_duty--;
									}
								} else if ((2550 < duty1 < 3500) && (duty2 < 2890)) { // accelarator not completely pressed
									if (motor_duty <= 90) {
										motor_duty += 2;
									}
								} else if ((duty1 >= 3500) && (duty2 < 2890)) { // floored that shit
									if (motor_duty <= 90) {
										motor_duty += 4;
									}
								}
								
								if (2890 < duty2 < 3500) {
									if (motor_duty >= 10) {
										motor_duty -= 2;
									}
								} else if (3500 <= duty2) {
									if (motor_duty >= 10) {
										motor_duty -= 4;
									}
								}
								
//                PWM0_PB6_SetDuty(motor_duty);

                // reset accumulators
                sum1 = sum2 = sum3 = 0;
                count = 0;
            }
    }
}
