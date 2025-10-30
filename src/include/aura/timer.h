#ifndef AURA_TIMER_H
#define AURA_TIMER_H

#include <stdint.h>

void timer_init(uint32_t frequency);
uint64_t timer_ticks(void);
void timer_sleep(uint64_t ticks);

#endif
