#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "logger.h"

namespace daisy
{


template <LoggerDestination dest>
void Logger<dest>::Print(const char* format, ...)
{    
    va_list va;
    va_start(va, format);
    tx_ptr_ += vsnprintf(tx_buff_ + tx_ptr_, sizeof(tx_buff_) - tx_ptr_, format, va);
    va_end(va);

    TransmitBuf();
}

template <LoggerDestination dest>
void Logger<dest>::PrintLine(const char* format, ...)
{    
    va_list va;
    va_start(va, format);
    tx_ptr_ += vsnprintf(tx_buff_ + tx_ptr_, sizeof(tx_buff_) - tx_ptr_, format, va);
    va_end(va);

    AppendNewLine();

    TransmitBuf();
}
    
template <LoggerDestination dest>
void Logger<dest>::StartLog(bool wait_for_pc)
{
    impl_.Init();
    // if waiting for PC, use blocking transmission
    pc_sync_ = wait_for_pc ? LOGGER_SYNC_IN : LOGGER_SYNC_OUT;
    PrintLine("Daisy is online");
}

template <LoggerDestination dest>
void Logger<dest>::TransmitBuf()
{
    if (tx_ptr_ >= sizeof(tx_buff_)) // the buffer is full - treat as overflow
    {
        // indicate truncated string with an unlikely character sequence "$$"
        tx_buff_[sizeof(tx_buff_) - 1] = '$';
        tx_buff_[sizeof(tx_buff_) - 2] = '$';
        tx_ptr_ = sizeof(tx_buff_);
    }

    if (pc_sync_ >= LOGGER_SYNC_IN)
    {
        TransmitSync(tx_buff_, tx_ptr_);
        tx_ptr_ = 0;
    }
    else
    {
        if (true == impl_.Transmit(tx_buff_, tx_ptr_))
        {
            pc_sync_++;
            tx_ptr_ = 0;
        }
        // do not reset tx_ptr_ otherwise to accumulate while buffer permits
    }
}

template <LoggerDestination dest>
void Logger<dest>::AppendNewLine()
{
    // trim existing control characters
    while (tx_ptr_ > 0 && (tx_buff_[tx_ptr_-1] == '\n' || tx_buff_[tx_ptr_-1] == '\r'))
    {
        tx_ptr_--;
    }
    
    // check if there's enough room for newline sequence
    constexpr size_t eol = NewLineSeqLength();
    if (tx_ptr_ + eol < sizeof(tx_buff_))
    {
        constexpr const char* nl = LOGGER_NEWLINE;
        for (size_t i = 0; i < eol; i++)    // this loop will be optimized away by the compiler
        {
            tx_buff_[tx_ptr_++] = nl[i];
        }
    }
    else    //trigger overflow indication
    {
        tx_ptr_ = sizeof(tx_buff_); 
    }
}

// explicit forward specializations
template class Logger<LOGGER_INTERNAL>;
template class Logger<LOGGER_EXTERNAL>;
template class Logger<LOGGER_SEMIHOST>;

// LoggerImpl static member variables
UsbHandle LoggerImpl<LOGGER_INTERNAL>::usb_handle_;
UsbHandle LoggerImpl<LOGGER_EXTERNAL>::usb_handle_;
}   // namespace daisy
