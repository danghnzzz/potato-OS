#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define PIT_CONTROL_REGISTER 0x43
#define PIT_CHANNEL_0_DATA_REGISTER 0x40
#define PIT_BASE_FREQUENCY 1193180
#define TIMER_HZ 100

void init_timer(void);

#endif
