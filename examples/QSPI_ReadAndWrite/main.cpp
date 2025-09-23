/** Generation of a simple Audio signal */
#include "daisy_seed.h"
#include <cmath>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

DaisySeed hw;

/** 4kB for fast test */
static constexpr size_t kTestSize = 0x1000;

/** statically allocated workspace for creation/validation of data */
static uint8_t workspace[kTestSize];

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Start Log */
    hw.StartLog(true);

    /** Erase a section of QSPI Memory
     *  4kB chunk at beginning of flash
     */
    hw.PrintLine("Erasing 4kB from QSPI memory...");
    uint32_t start_addr = 0;
    uint32_t end_addr   = start_addr + kTestSize;
    if(hw.qspi.Erase(0, end_addr) == QSPIHandle::Result::OK)
        hw.PrintLine("Success");
    else
        hw.PrintLine("Failed to erase...");

    /** Validate by reading directly from that memory location */
    hw.PrintLine("Validating erasure...");
    uint32_t fail_cnt = 0;
    dsy_dma_invalidate_cache_for_buffer((uint8_t*)(0x90000000) + start_addr,
                                        kTestSize);
    for(size_t i = 0; i < kTestSize; i++)
    {
        uint32_t addr      = 0x90000000 + (start_addr + i);
        uint8_t* qspi_byte = (uint8_t*)addr;
        if(*qspi_byte != 0xff)
            fail_cnt++;
    }
    hw.PrintLine("Done: %d unerased values", fail_cnt);

    /** Now write all zeroes using preallocated static workspace */
    hw.PrintLine("Writing all zeroes to 4kB section of QSPI...");
    std::fill(workspace, workspace + kTestSize, 0x00);
    if(hw.qspi.Write(start_addr, end_addr, workspace) == QSPIHandle::Result::OK)
        hw.PrintLine("Success");
    else
        hw.PrintLine("Failed to write...");

    /** Validate by reading directly from that memory location */
    hw.PrintLine("Validating write success...");
    fail_cnt = 0;
    dsy_dma_invalidate_cache_for_buffer((uint8_t*)(0x90000000) + start_addr,
                                        kTestSize);
    for(size_t i = 0; i < kTestSize; i++)
    {
        uint32_t addr      = 0x90000000 + (start_addr + i);
        uint8_t* qspi_byte = (uint8_t*)addr;
        if(*qspi_byte != 0x00)
            fail_cnt++;
    }
    hw.PrintLine("Done: %d unwritten values", fail_cnt);

    /** Infinite Loop */
    while(1)
    {
        hw.SetLed((System::GetNow() & 511) > 255);
    }
}