#ifndef AURA_MMIO_H
#define AURA_MMIO_H

#include <stdint.h>

static inline uint8_t mmio_read8(volatile void *addr) {
    return *(volatile uint8_t *)addr;
}

static inline void mmio_write8(volatile void *addr, uint8_t value) {
    *(volatile uint8_t *)addr = value;
}

static inline uint32_t mmio_read32(volatile void *addr) {
    return *(volatile uint32_t *)addr;
}

static inline void mmio_write32(volatile void *addr, uint32_t value) {
    *(volatile uint32_t *)addr = value;
}

static inline uint64_t mmio_read64(volatile void *addr) {
    return *(volatile uint64_t *)addr;
}

static inline void mmio_write64(volatile void *addr, uint64_t value) {
    *(volatile uint64_t *)addr = value;
}

#endif
