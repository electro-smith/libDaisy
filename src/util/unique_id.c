#include "util/unique_id.h"

// Unique device ID register base address
#define H7_UID_BASE (0x1FF1E800UL)

void dsy_get_unique_id(uint32_t *w0, uint32_t *w1, uint32_t *w2)
{
    *w0 = *((uint32_t *)(H7_UID_BASE));
    *w1 = *((uint32_t *)(H7_UID_BASE + 4));
    *w2 = *((uint32_t *)(H7_UID_BASE + 8));
}
