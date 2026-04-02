#pragma once

#include <cstdint>
#include <vector>

#include "ff.h"

namespace test
{
struct FatFsMockState
{
    std::vector<uint8_t> file_data;
    FSIZE_t              cursor      = 0;
    int                  open_count  = 0;
    int                  close_count = 0;
};

FatFsMockState& GetFatFsMockState();
void            ResetFatFsMockState();
} // namespace test

