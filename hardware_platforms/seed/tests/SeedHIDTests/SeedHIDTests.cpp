#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;

static daisy_handle hw;

int main(void)
{
    // Initialize Hardware
    daisy_seed_init(&hw);
    dsy_tim_start();
    while(1) {}
}
