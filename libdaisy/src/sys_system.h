#ifndef DSY_SYSTEM_H
#define DSY_SYSTEM_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

// Sets clock speeds, etc.
void dsy_system_init();
void dsy_system_jumpto(uint32_t addr);
void dsy_system_jumptoqspi();
uint32_t dsy_system_getnow(); // returns HAL_GetTick()
void dsy_system_delay(uint32_t delay_ms);
#ifdef __cplusplus
};
#endif
#endif
