#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include "logger.h"
#include "sys/system.h"

namespace daisy
{
template <LoggerDestination dest>
void Logger<dest>::Print(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    PrintV(format, va);
    va_end(va);
}
template <LoggerDestination dest>
void Logger<dest>::PrintV(const char* format, va_list va)
{
    tx_ptr_ += vsnprintf(
        tx_buff_ + tx_ptr_, sizeof(tx_buff_) - tx_ptr_, format, va);

    TransmitBuf();
}

template <LoggerDestination dest>
void Logger<dest>::PrintLine(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    PrintLineV(format, va);
    va_end(va);
}

template <LoggerDestination dest>
void Logger<dest>::PrintLineV(const char* format, va_list va)
{
    tx_ptr_ += vsnprintf(
        tx_buff_ + tx_ptr_, sizeof(tx_buff_) - tx_ptr_, format, va);

    AppendNewLine();

    TransmitBuf();
}

template <LoggerDestination dest>
void Logger<dest>::StartLog(bool wait_for_pc)
{
    impl_.Init();
    /* if waiting for PC, use blocking transmission */
    pc_sync_ = wait_for_pc ? LOGGER_SYNC_IN : LOGGER_SYNC_OUT;
    /** transmit something to stall the UART until a terminal is connected
     * at least two separate calls are required
     */
    PrintLine("Daisy is online");
    PrintLine("===============");
    System::Delay(10);
}

template <LoggerDestination dest>
void Logger<dest>::TransmitBuf()
{
    /** if the buffer is full - treat as overflow */
    if(tx_ptr_ >= sizeof(tx_buff_))
    {
        /** indicate truncation with an unlikely character sequence "$$" */
        tx_buff_[sizeof(tx_buff_) - 1] = '$';
        tx_buff_[sizeof(tx_buff_) - 2] = '$';

        tx_ptr_ = sizeof(tx_buff_);
    }

    if(pc_sync_ >= LOGGER_SYNC_IN)
    {
        TransmitSync(tx_buff_, tx_ptr_);
        tx_ptr_ = 0;
    }
    else
    {
        if(true == impl_.Transmit(tx_buff_, tx_ptr_))
        {
            pc_sync_++;
            tx_ptr_ = 0;
        }
        /** otherwise do not reset tx_ptr_
         *  accumulate data while buffer size permits 
         */
    }
}

template <LoggerDestination dest>
void Logger<dest>::AppendNewLine()
{
    /*  trim existing control characters */
    while(tx_ptr_ > 0
          && (tx_buff_[tx_ptr_ - 1] == '\n' || tx_buff_[tx_ptr_ - 1] == '\r'))
    {
        tx_ptr_--;
    }

    /* check if there's enough room for newline sequence */
    constexpr size_t eol = NewLineSeqLength();
    if(tx_ptr_ + eol < sizeof(tx_buff_))
    {
        /* this loop will be optimized away by the compiler */
        constexpr const char* nl = LOGGER_NEWLINE;
        for(size_t i = 0; i < eol; i++)
        {
            tx_buff_[tx_ptr_++] = nl[i];
        }
    }
    else /**< trigger overflow indication */
    {
        tx_ptr_ = sizeof(tx_buff_);
    }
}

/** explicit forward specializations */
template class Logger<LOGGER_INTERNAL>;
template class Logger<LOGGER_EXTERNAL>;
template class Logger<LOGGER_SEMIHOST>;

/** LoggerImpl static member variables */
UsbHandle LoggerImpl<LOGGER_INTERNAL>::usb_handle_;
UsbHandle LoggerImpl<LOGGER_EXTERNAL>::usb_handle_;
} // namespace daisy
