/** Test program for evaluating QSPI erase performance.
 *  This program will perform multiple write-erase cycles
 *  on the first 256kB bytes of the QSPI.
 */
#include "daisy_seed.h"
#include <cmath>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

struct TestProfile
{
    uint32_t erase_dur, write_dur;
    bool     pass;
};

DaisySeed hw;

/** statically allocated workspace for creation/validation of data
 *  256kB so we can look at multiple blocks of 64kB if we want.
*/
static const size_t kWorkspaceSize = 0x40000;
static uint8_t      workspace[kWorkspaceSize];

TestProfile EraseWriteValidate(QSPIHandle& qspi, size_t test_size)
{
    TestProfile res;

    // Erase
    auto start = System::GetNow();
    qspi.Erase(0, test_size);
    auto end      = System::GetNow();
    res.erase_dur = end - start;

    // Write
    start = System::GetNow();
    qspi.Write(0, test_size, workspace);
    end           = System::GetNow();
    res.write_dur = end - start;

    // Validate write
    dsy_dma_invalidate_cache_for_buffer((uint8_t*)(0x90000000), test_size);
    uint8_t* mem = (uint8_t*)(0x90000000);
    res.pass     = true;
    for(size_t i = 0; i < test_size; i++)
    {
        if(mem[i] != 0x00)
            res.pass = false;
    }
    return res;
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Start Log */
    hw.StartLog(true);
    // without these delays the printing seems to just get lost before being output?
    System::Delay(100);
    hw.PrintLine("Beginning Erasure Timing Tests...");

    // Naive write data, but since erase sets all bytes 0xff, we'll just flip'em.
    std::fill(workspace, workspace + kWorkspaceSize, 0x00);

    hw.PrintLine("Test 1: Erase, Write and Validate a single 4kB sector.");
    hw.PrintLine("\tstarting...");
    auto t1_res = EraseWriteValidate(hw.qspi, 0x1000);
    hw.PrintLine("\tErase Time: %dms", t1_res.erase_dur);
    hw.PrintLine("\tWrite Time: %dms", t1_res.write_dur);
    hw.PrintLine("\tValidated Write: %s", t1_res.pass ? "Pass" : "Fail");
    hw.PrintLine("\tfinished.");

    hw.PrintLine("Test 2: Erase, Write, and Validate a single 64kB block");
    hw.PrintLine("\tstarting...");
    auto t2_res = EraseWriteValidate(hw.qspi, 0x10000);
    hw.PrintLine("\tErase Time: %dms", t2_res.erase_dur);
    hw.PrintLine("\tWrite Time: %dms", t2_res.write_dur);
    hw.PrintLine("\tValidated Write: %s", t2_res.pass ? "Pass" : "Fail");
    hw.PrintLine("\tfinished.");

    hw.PrintLine("Test 3: Erase 256kB in one call (internally 4x 64kB erases)");
    size_t test_size = 0x40000;
    auto   start     = System::GetNow();
    hw.qspi.Erase(0, test_size);
    auto end       = System::GetNow();
    auto erase_dur = end - start;
    hw.PrintLine("\tTotal Erase Time: %dms", erase_dur);

    hw.PrintLine("4: Erase 256kB in 4x 64kB blocks/fn calls");
    uint32_t t4_durs[4] = {0};
    test_size           = 0x10000;
    start               = System::GetNow();
    for(size_t i = 0; i < 4; i++)
    {
        auto istart     = System::GetNow();
        auto start_addr = 0 + (test_size * i);
        hw.qspi.Erase(start_addr, start_addr + test_size);
        auto iend  = System::GetNow();
        t4_durs[i] = iend - istart;
    }
    end       = System::GetNow();
    erase_dur = end - start;
    hw.PrintLine("\t0: %dms\t1: %dms\t2: %dms\t3: %dms",
                 t4_durs[0],
                 t4_durs[1],
                 t4_durs[2],
                 t4_durs[3]);
    hw.PrintLine("\tTotal Erase Time: %dms", erase_dur);
    hw.PrintLine("\tfinished.");

    hw.PrintLine("5: Erase 256kB in 64x 4kB blocks/fn calls");
    uint32_t t5_durs[64] = {0};
    test_size            = 0x1000;
    start                = System::GetNow();
    for(size_t i = 0; i < 64; i++)
    {
        auto istart     = System::GetNow();
        auto start_addr = 0 + (test_size * i);
        hw.qspi.Erase(start_addr, start_addr + test_size);
        auto iend  = System::GetNow();
        t5_durs[i] = iend - istart;
    }
    end       = System::GetNow();
    erase_dur = end - start;

    for(size_t i = 0; i < 64; i++)
    {
        hw.Print("\t%d: %dms", i, t5_durs[i]);
        if(i % 4 == 3)
            hw.Print("\n");
    }

    hw.PrintLine("\tTotal Erase Time: %dms", erase_dur);
    hw.PrintLine("\tfinished.");

    hw.PrintLine("Done.");

    /** Infinite Loop to Blink when completed. */
    while(1)
    {
        hw.SetLed((System::GetNow() & 511) > 255);
    }
}