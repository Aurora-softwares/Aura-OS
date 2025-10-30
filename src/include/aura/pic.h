#ifndef AURA_PIC_H
#define AURA_PIC_H

#include <stdint.h>

void pic_init(void);
void pic_send_eoi(uint8_t irq);

#endif
