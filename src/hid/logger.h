#pragma once
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "logger_impl.h"

namespace daisy
{
// Logger configuration
#define LOGGER_NEWLINE "\r\n"   // newline character sequence
#define LOGGER_BUFFER  128       // in bytes (may be reduced for testing or increased for real apps)

// Helper macros for string concatenation and macro expansion
#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B) 

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

// Floating point output formatting
#define FLT_FMT(_n) STRINGIZE(PPCAT(PPCAT(%c%d.%0, _n), d))
#define FLT_VAR(_n, _x) (_x < 0 ? '-': ' '), (int)(abs(_x)), (int)(((abs(_x)) - (int)(abs(_x))) * pow(10, (_n)))

// Shorthand for 10^-3 fractions
#define FLT_FMT3 FLT_FMT(3)
#define FLT_VAR3(_x) FLT_VAR(3, _x)



//   @brief Interface for simple USB logging
//   @author Alexander Petrov-Savchenko (axp@soft-amp.com)
//   @date November 2020
template <LoggerDestination dest = LOGGER_INTERNAL>
class Logger
{
public:

    Logger()
    {
    }
    static void Print(const char* format, ...)
    {    
        va_list va;
        va_start(va, format);
        tx_ptr_ += vsnprintf(tx_buff_ + tx_ptr_, sizeof(tx_buff_) - tx_ptr_, format, va);
        va_end(va);

        TransmitBuf();
    }

    static void PrintLine(const char* format, ...)
    {    
        va_list va;
        va_start(va, format);
        tx_ptr_ += vsnprintf(tx_buff_ + tx_ptr_, sizeof(tx_buff_) - tx_ptr_, format, va);
        va_end(va);

        AppendNewLine();

        TransmitBuf();
    }
    
    static void StartLog(bool wait_for_pc = false)
    {
        impl_.Init();
        // if waiting for PC, use blocking transmission
        pc_sync_ = wait_for_pc ? LOGGER_SYNC_IN : LOGGER_SYNC_OUT;
        PrintLine("Daisy is online");
    }

protected:
    enum LoggerConsts
    {
        LOGGER_SYNC_OUT = 0,
        LOGGER_SYNC_IN  = 2 // successfully transmit this many packets to consider being in sync and switch to blocking transfers
    };
    static void TransmitSync(const void* buffer, size_t bytes)
    {
        while(false == impl_.Transmit(buffer, bytes));
    }

    static void TransmitBuf()
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

    // constexpr function equivalent of strlen(LOGGER_NEWLINE)
    static constexpr size_t NewLineSeqLength()
    {
        size_t len = 0;
        constexpr const char* nl = LOGGER_NEWLINE;
        while (nl[len] != '\0')
        {
            len++;
        }
        return len;    
    }

    // trim control characters and append clean newline sequence, if there's room in the buffer
    static void AppendNewLine()
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

    static char     tx_buff_[LOGGER_BUFFER];
    static size_t   tx_ptr_;
    static size_t   pc_sync_;
    static LoggerImpl<dest> impl_;
};

template<LoggerDestination dest>
char Logger<dest>::tx_buff_[LOGGER_BUFFER];   // this needs to remain in SRAM to support startup-time printouts

template<LoggerDestination dest>
size_t Logger<dest>::pc_sync_ = LOGGER_SYNC_OUT;    // start with non-blocking transfers to support startup-time printouts

template<LoggerDestination dest>
size_t Logger<dest>::tx_ptr_ = 0;

template<LoggerDestination dest>
LoggerImpl<dest> Logger<dest>::impl_;

// Specialization for a muted log
template <>
class Logger<LOGGER_NONE>
{
public:
    Logger() {}
    static void Print(const char* format, ...)      {}
    static void PrintLine(const char* format, ...)  {}
    static void StartLog(bool wait_for_pc = false)  {}
};


}   // namespace daisy