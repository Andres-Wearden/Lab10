#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/UART0int.h"
#include "../inc/CortexM.h"
#include "PWMIND.h"

#define AVG_SAMPLES 16  // increased number of readings to average

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

    GPIO_PORTD_AFSEL_R |= (1<<6)|(1<<7);
    GPIO_PORTD_PCTL_R  = (GPIO_PORTD_PCTL_R & ~0xFF000000)
                       | (1<<24)
                       | (1<<28);
    GPIO_PORTD_DEN_R   |= (1<<6)|(1<<7);
    GPIO_PORTD_DIR_R   |=  (1<<7);
    GPIO_PORTD_DIR_R   &= ~(1<<6);

    UART2_CTL_R &= ~UART_CTL_UARTEN;
    UART2_CC_R   = 0;
    UART2_IBRD_R = 43;
    UART2_FBRD_R = 26;
    UART2_LCRH_R = (3<<5);
    UART2_CTL_R |= (UART_CTL_UARTEN|UART_CTL_RXE|UART_CTL_TXE);
}

void UART2_SendChar(char c){
    while(UART2_FR_R & UART_FR_TXFF){}
    UART2_DR_R = c;
}

char UART2_RecvChar(void){
    while(UART2_FR_R & UART_FR_RXFE){}
    return (char)(UART2_DR_R & 0xFF);
}

int main(void){
    uint32_t sum = 0, count = 0;
    uint8_t lastSw = 0, sw;

    DisableInterrupts();
    PLL_Init(Bus80MHz);
    PWM_PB6_Init();
    PWM_PD0_Init();
    PWM0_PD0_SetPeriod(25000);
    PWM0_PB6_SetPeriod(1600);
    PWM0_PB6_SetDuty(200);
    PortF_Init();
    UART2_Init();
    UART_Init();
    EnableInterrupts();

    UART_OutString("UART2 on PD6/PD7; averaging ");
    UART_OutUDec(AVG_SAMPLES);
    UART_OutString(" samples...\r\n");

    while(1){
        // Read two bytes from UART2, reconstruct 12-bit sample
        uint8_t hi = (uint8_t)UART2_RecvChar();
        uint8_t lo = (uint8_t)UART2_RecvChar();
        uint16_t raw12 = ((uint16_t)hi << 8) | lo;
        raw12 &= 0x0FFF;

        // Accumulate
        sum += raw12;
        if(++count >= AVG_SAMPLES){
            // Compute & print the average
            uint32_t avg = sum / AVG_SAMPLES;
            UART_OutString("Avg(16) = ");
            UART_OutUDec(avg);
            UART_OutString("\r\n");

            // Update PWM based on average
            uint32_t duty = 100 + (avg * 1900) / 4095;
            UART_OutString("PWM = ");
            UART_OutUDec(duty);
            UART_OutString("\r\n");
            PWM0_PD0_Duty(duty);

            // Reset for next block
            sum = 0;
            count = 0;
        }
    }
}
