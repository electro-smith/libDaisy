#include "libdaisy.h"
#include <string.h>
static void Callback(float *in, float *out, size_t size)
{
    memcpy(out, in, size*sizeof(float));
}
int main(void)
{
    uint8_t board = DSY_SYS_BOARD_DAISY_SEED;
    dsy_system_init(board);
    dsy_audio_init(board, DSY_AUDIO_INTERNAL, DSY_AUDIO_DEVICE_WM8731);
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, Callback);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
    while(1) {}
}
