#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"

int main(void)
{
    uint8_t board = DSY_SYS_BOARD_DAISY_SEED;
    dsy_system_init(board);
    while(1) {}
}
