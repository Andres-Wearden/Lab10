#include "../inc/PLL.h"

#include "../inc/PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

void PWM_PB6_Init(void);

void PWM0_PB6_SetPeriod(uint32_t periodCycles);

void PWM0_PB6_SetDuty(uint16_t pulse);

void PWM_PD0_Init(void);

void PWM0_PD0_SetPeriod(uint32_t periodCycles);

void PWM0_PD0_Duty(uint16_t pulse);