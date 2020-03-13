#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "per_sdmmc.h"

#define TEST_FILE_NAME "test.txt"

static daisy_handle hw;

daisy::SdmmcHandler sd;


int main(void)
{
    daisy_seed_init(&hw);
    char   outbuff[512];
    char   inbuff[512];
    size_t len, failcnt;
    sprintf(outbuff, "Daisy...Testing...\n1...\n2...\3...\n");
    memset(inbuff, 0, 512);
    len     = strlen(outbuff);
    failcnt = 0;
    // Initialize the SD Card, and mount it.
    sd.Init();
    sd.Mount();
    // Write the test file to the SD Card.
    // Read back the test file from the SD Card.
    // Check for sameness.
    for(size_t i = 0; i < len; i++)
    {
        if(inbuff[i] != outbuff[i])
        {
            failcnt++;
        }
    }
    if(failcnt)
    {
        asm("bkpt 255");
    }
    for(;;) {}
}
