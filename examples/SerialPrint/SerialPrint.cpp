#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;

int main(void)
{
    // Initialize the Daisy Seed
    hw.Init();
    // Start the log, and wait for connection
    hw.StartLog(true);
    // Print "Hello World" to the Serial Monitor
    hw.PrintLine("Hello World!");
    while(1) {}
}
