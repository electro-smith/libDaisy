#pragma once
#ifndef __DSY_LOGGER_H__
#define __DSY_LOGGER_H__

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
    {}
    static void Print(const char* format, ...);         // Print formatted string
    static void PrintLine(const char* format, ...);     // Print formatted string appending line termination sequence
    static void StartLog(bool wait_for_pc = false);     // Start the logging session. Block until remote terminal is ready if wait_for_pc is true

protected:
    enum LoggerConsts
    {
        LOGGER_SYNC_OUT = 0,
        LOGGER_SYNC_IN  = 2 // successfully transmit this many packets to consider being in sync and switch to blocking transfers
    };

    // blocking wrapper for Transmit
    static void TransmitSync(const void* buffer, size_t bytes)
    {
        while(false == impl_.Transmit(buffer, bytes));
    }
    
    // transfer accumulated data
    static void TransmitBuf();

    // trim control characters and append clean newline sequence, if there's room in the buffer
    static void AppendNewLine();

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

    // member variables
    static char     tx_buff_[LOGGER_BUFFER];
    static size_t   tx_ptr_;
    static size_t   pc_sync_;
    static LoggerImpl<dest> impl_;
};

// member variable definition (could switch to inline statics in C++17)

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

#endif // __DSY_LOGGER_H__
