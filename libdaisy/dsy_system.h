#ifndef DSY_SYSTEM_H
#define DSY_SYSTEM_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

// Sets clock speeds, etc.
void dsy_system_init(uint8_t board);
void dsy_system_jumpto(uint32_t addr);
void dsy_system_jumptoqspi();
#ifdef __cplusplus
}
#endif
#endif