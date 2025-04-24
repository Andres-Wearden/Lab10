#include "../inc/PLL.h"

#include "../inc/PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define PWM_ENABLE_PWM0 0x00000001  // enable PWM output 0
#define PWM_ENABLE_PWM6 0x00000040  // enable PWM output 6

void PWM_PB6_Init(void) {
    // 1) Enable PWM0 + Port B, wait
    SYSCTL_RCGCPWM_R |= 1<<0;
    while (!(SYSCTL_PRPWM_R & (1<<0))) {}
    SYSCTL_RCGCGPIO_R |= 1<<1;
    while (!(SYSCTL_PRGPIO_R & (1<<1))) {}
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M)
                   | SYSCTL_RCC_USEPWMDIV
                   | SYSCTL_RCC_PWMDIV_64;
    GPIO_PORTB_AFSEL_R  |= 1<<6;
    GPIO_PORTB_PCTL_R   = (GPIO_PORTB_PCTL_R & ~0x0F000000)
                         | 0x04000000;
    GPIO_PORTB_AMSEL_R  &= ~(1<<6);
    GPIO_PORTB_DEN_R    |= 1<<6;
    PWM0_0_CTL_R &= ~PWM_0_CTL_ENABLE;
    PWM0_0_GENA_R  = PWM_0_GENA_ACTLOAD_ONE 
                   | PWM_0_GENA_ACTCMPAD_ZERO;
    PWM0_0_LOAD_R = 500 - 1;
    PWM0_0_CMPA_R = (500 - 1) - (500/2);  // 50% duty
    PWM0_0_CTL_R   |= PWM_0_CTL_ENABLE;
    PWM0_ENABLE_R |= PWM_ENABLE_PWM0;
}

void PWM0_PB6_SetPeriod(uint32_t periodCycles) {
    PWM0_0_CTL_R &= ~PWM_0_CTL_ENABLE;
    PWM0_0_LOAD_R = periodCycles - 1;
    PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;
}

void PWM0_PB6_SetDuty(uint16_t pulse) {
    uint32_t load = PWM0_0_LOAD_R + 1;
    PWM0_0_CMPA_R = load - pulse;
}

void PWM_PD0_Init(void) {
    SYSCTL_RCGCPWM_R |= 1<<0;
    while (!(SYSCTL_PRPWM_R & (1<<0))) {}
    SYSCTL_RCGCGPIO_R |= 1<<3;
    while (!(SYSCTL_PRGPIO_R & (1<<3))) {}
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M)
                   | SYSCTL_RCC_USEPWMDIV
                   | SYSCTL_RCC_PWMDIV_64;
    GPIO_PORTD_AFSEL_R  |= 1<<0;
    GPIO_PORTD_PCTL_R   = (GPIO_PORTD_PCTL_R & ~0x0000000F)
                         | 0x00000004;
    GPIO_PORTD_AMSEL_R  &= ~(1<<0);
    GPIO_PORTD_DEN_R    |= 1<<0;
    PWM0_3_CTL_R &= ~PWM_3_CTL_ENABLE;
    PWM0_3_GENA_R  = PWM_3_GENA_ACTLOAD_ONE 
                   | PWM_3_GENA_ACTCMPAD_ZERO;
    PWM0_3_LOAD_R = 5000 - 1;
    PWM0_3_CMPA_R = (5000 - 1) - (5000/10);  // 10% duty
    PWM0_3_CTL_R   |= PWM_3_CTL_ENABLE;
    PWM0_ENABLE_R |= PWM_ENABLE_PWM6;
}

void PWM0_PD0_SetPeriod(uint32_t periodCycles) {
    PWM0_3_CTL_R &= ~PWM_3_CTL_ENABLE;
    PWM0_3_LOAD_R = periodCycles - 1;
    PWM0_3_CTL_R |= PWM_3_CTL_ENABLE;
}

void PWM0_PD0_Duty(uint16_t pulse) {
    uint32_t load = PWM0_3_LOAD_R + 1;
    PWM0_3_CMPA_R = load - pulse;
}