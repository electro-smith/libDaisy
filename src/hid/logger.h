#pragma once
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "hid/usb.h"
#include "usbd_def.h"
#include "sys/system.h"
#include <assert.h>

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
class Logger
{
public:
    Logger()
    {
        // this implementation relies on the fact that UsbHandle class has no member variables and can be shared
        // assert this statement:
        static_assert(1u == sizeof(usb_handle_));
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
    
    static void StartLog(UsbHandle::UsbPeriph device = UsbHandle::FS_INTERNAL, bool wait_for_pc = false)
    {
        usb_handle_.Init(device);
        init_done_ = true;
        PrintLine("Daisy is online");

        if (wait_for_pc)
        {
            while(false == pc_up_)
            {        
                dsy_system_delay(1000);
                usb_handle_.SetReceiveCallback(UsbCallback, device);
                PrintLine("press any key to start Daisy...");
            }
        }
    }

protected:
    static void UsbCallback(uint8_t* buf, uint32_t* len)
    {
        if (false == pc_up_)
        {
            if (len && *len > 0)
            {
                pc_up_ = true;
            }
        }
    }

    static void TransmitBuf()
    {
        if (init_done_)
        {
            if (tx_ptr_ >= sizeof(tx_buff_)) // the buffer is full - treat as overflow
            {
                // indicate truncated string with an unlikely character sequence "$$"
                tx_buff_[sizeof(tx_buff_) - 1] = '$';
                tx_buff_[sizeof(tx_buff_) - 2] = '$';
                tx_ptr_ = sizeof(tx_buff_);
            }

            uint8_t res;            
            do
            {
                res = usb_handle_.TransmitInternal((uint8_t*)tx_buff_, tx_ptr_);
            } while(USBD_BUSY == res);
            tx_ptr_ = 0;
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

    static char rx_buff_[LOGGER_BUFFER]; // not really used currently, may be shrinked
    static char tx_buff_[LOGGER_BUFFER];
    static bool pc_up_; 
    static bool init_done_;
    static size_t tx_ptr_;

    static UsbHandle usb_handle_; 
};

}   // namespace daisy