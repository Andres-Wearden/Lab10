#ifndef __TIMERS__
#define __TIMERS__

#include <stdint.h>
#include "../inc/Timer0A.h"

void Heartbeat_Init(uint32_t period, uint32_t priority);
void Heartbeat(void);
void PortF_Red_Led_Init(void);

#endif
