#pragma once
#ifndef __DSY_LOGGER_IMPL_H
#define __DSY_LOGGER_IMPL_H
#include <unistd.h>
#include "hid/usb.h"
#include "usbd_def.h"
#include "sys/system.h"
#include <assert.h>

//  @brief Logging I/O underlying implementation
//  @author Alexander Petrov-Savchenko (axp@soft-amp.com)
//  @date November 2020


namespace daisy
{

enum LoggerDestination
{
    LOGGER_NONE,     /**< mute logging */
    LOGGER_INTERNAL, /**< internal USB port */
    LOGGER_EXTERNAL, /**< external USB port */
    LOGGER_SEMIHOST, /**< stdout */
};

//  @brief Default implementation (mute)
template <LoggerDestination dest>
class LoggerImpl
{
public:
    static void Init()  {}
    static bool Transmit(const void* buffer, size_t bytes)    {return true;   }    
};


//  @brief Specialization for internal USB port
template <>
class LoggerImpl<LOGGER_INTERNAL>
{
public:
    static void Init()
    {
        // this implementation relies on the fact that UsbHandle class has no member variables and can be shared
        // assert this statement:
        static_assert(1u == sizeof(usb_handle_));
        usb_handle_.Init(UsbHandle::FS_INTERNAL);
    }

    static bool Transmit(const void* buffer, size_t bytes)
    {
        return USBD_OK == usb_handle_.TransmitInternal((uint8_t*)buffer, bytes);
    }    

protected:
    static UsbHandle usb_handle_; 
};




//  @brief Specialization for external USB port
template <>
class LoggerImpl<LOGGER_EXTERNAL>
{
public:
    static void Init()
    {
        // this implementation relies on the fact that UsbHandle class has no member variables and can be shared.
        // assert this statement:
        static_assert(1u == sizeof(usb_handle_));
        usb_handle_.Init(UsbHandle::FS_EXTERNAL);
    }

    static bool Transmit(const void* buffer, size_t bytes)
    {
        return USBD_OK == usb_handle_.TransmitExternal((uint8_t*)buffer, bytes);
    }    

protected:
    static UsbHandle usb_handle_; 
};




//  @brief Specialization for semihosting (stdout)
template <>
class LoggerImpl<LOGGER_SEMIHOST>
{
public:
    static void Init()  {}
    static bool Transmit(const void* buffer, size_t bytes)
    {
        write(STDOUT_FILENO, buffer, bytes);
        return true;
    }
};


}   // namespace daisy

#endif //__DSY_LOGGER_IMPL_H