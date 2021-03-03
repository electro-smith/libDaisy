#pragma once
#ifndef __DSY_LOGGER_IMPL_H
#define __DSY_LOGGER_IMPL_H
#include <unistd.h>
#include <cassert>
#include "hid/usb.h"
#include "sys/system.h"


namespace daisy
{
/** Enumeration of destination ports for debug logging
 */
enum LoggerDestination
{
    LOGGER_NONE,     /**< mute logging */
    LOGGER_INTERNAL, /**< internal USB port */
    LOGGER_EXTERNAL, /**< external USB port */
    LOGGER_SEMIHOST, /**< stdout */
};

/** @brief Logging I/O underlying implementation
 *  @author Alexander Petrov-Savchenko (axp@soft-amp.com)
 *  @date November 2020
 */
template <LoggerDestination dest>
class LoggerImpl
{
  public:
    /** Initialize logging destination
     */
    static void Init() {}

    /** Transmit a block of data
     */
    static bool Transmit(const void* buffer, size_t bytes) { return true; }
};


/**  @brief Specialization for internal USB port
 */
template <>
class LoggerImpl<LOGGER_INTERNAL>
{
  public:
    /** Initialize logging destination
    */
    static void Init()
    {
        /** this implementation relies on the fact that UsbHandle class has no member variables and can be shared
         * assert this statement:
         */
        static_assert(1u == sizeof(usb_handle_), "UsbHandle is not static");
        usb_handle_.Init(UsbHandle::FS_INTERNAL);
    }

    /** Transmit a block of data
     */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        return UsbHandle::Result::OK
               == usb_handle_.TransmitInternal((uint8_t*)buffer, bytes);
    }

  protected:
    /** USB Handle for CDC transfers 
     */
    static UsbHandle usb_handle_;
};


/**  @brief Specialization for external USB port
 */
template <>
class LoggerImpl<LOGGER_EXTERNAL>
{
  public:
    /** Initialize logging destination
     */
    static void Init()
    {
        /** this implementation relies on the fact that UsbHandle class has no member variables and can be shared.
         * assert this statement:
         */
        static_assert(1u == sizeof(usb_handle_), "UsbHandle is not static");
        usb_handle_.Init(UsbHandle::FS_EXTERNAL);
    }

    /** Transmit a block of data
     */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        return UsbHandle::Result::OK
               == usb_handle_.TransmitExternal((uint8_t*)buffer, bytes);
    }

  protected:
    /** USB Handle for CDC transfers 
     */
    static UsbHandle usb_handle_;
};


/**  @brief Specialization for semihosting (stdout)
 */
template <>
class LoggerImpl<LOGGER_SEMIHOST>
{
  public:
    /** Initialize logging destination
     */
    static void Init() {}

    /** Transmit a block of data
     */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        write(STDOUT_FILENO, buffer, bytes);
        return true;
    }
};


} /* namespace daisy */

#endif //__DSY_LOGGER_IMPL_H
