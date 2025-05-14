#pragma once
#ifndef __DSY_LOGGER_IMPL_H
#define __DSY_LOGGER_IMPL_H
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "hid/usb.h"
#include "sys/system.h"
#include "util/FIFO.h"

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

/** Log message structure for queuing */
struct LogMessage
{
    // NOTE: kMaxMessageSize should not be larger than CFG_TUD_CDC_EP_BUFSIZE (see hid/tusb_config.h)
    static const size_t kMaxMessageSize = 64;
    char                message[kMaxMessageSize];
    size_t              length;

    LogMessage() : length(0) { message[0] = '\0'; }

    LogMessage(const char* msg, size_t len)
    : length(std::min(len, kMaxMessageSize - 1))
    {
        memcpy(message, msg, length);
        message[length] = '\0';
    }
};

/** FIFO capacity for log messages */
static const size_t kLogFifoSize = 32;

/** @brief Logging I/O underlying implementation
 *  @author Alexander Petrov-Savchenko (axp@soft-amp.com)
 *  @date November 2020
 */
template <LoggerDestination dest>
class LoggerImpl
{
  public:
    /** Initialize logging destination */
    static void Init() {}

    /** Queue a message for transmission */
    static bool QueueMessage(const void* buffer, size_t bytes)
    {
        // Default implementation does nothing
        return true;
    }

    /** Process queued messages */
    static void ProcessQueue() {}

    /** Transmit a block of data directly (blocking) */
    static bool TransmitDirect(const void* buffer, size_t bytes)
    {
        return true;
    }

    /** Non-blocking transmission - queues message instead */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        return QueueMessage(buffer, bytes);
    }
};


/**  @brief Specialization for internal USB port */
template <>
class LoggerImpl<LOGGER_INTERNAL>
{
  public:
    /** Initialize logging destination */
    static void Init()
    {
        /** this implementation relies on the fact that UsbHandle class has no member variables and can be shared */
        static_assert(1u == sizeof(usb_handle_), "UsbHandle is not static");
        usb_handle_.Init(UsbHandle::FS_INTERNAL);
    }

    /** Queue a message for transmission */
    static bool QueueMessage(const void* buffer, size_t bytes)
    {
        if(bytes == 0 || buffer == nullptr)
            return false;

        LogMessage msg(static_cast<const char*>(buffer), bytes);
        return log_fifo_.PushBack(msg);
    }

    /** Process queued messages without blocking the caller */
    static void ProcessQueue()
    {
        // Only process one message per call to avoid blocking too long
        if(!log_fifo_.IsEmpty())
        {
            const LogMessage& msg = log_fifo_.Front();
            if(TransmitDirect(msg.message, msg.length))
            {
                log_fifo_.PopFront();
                System::DelayUs(100);
            }
        }
    }

    /** Direct transmission - may block */
    static bool TransmitDirect(const void* buffer, size_t bytes)
    {
        return UsbHandle::Result::OK
               == usb_handle_.TransmitInternal((uint8_t*)buffer, bytes);
    }

    /** Non-blocking transmission - queues the message instead */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        return QueueMessage(buffer, bytes);
    }

  protected:
    /** USB Handle for CDC transfers */
    static UsbHandle usb_handle_;

    /** FIFO for log messages */
    static FIFO<LogMessage, kLogFifoSize> log_fifo_;
};

/**  @brief Specialization for external USB port */
template <>
class LoggerImpl<LOGGER_EXTERNAL>
{
  public:
    /** Initialize logging destination */
    static void Init()
    {
        /** this implementation relies on the fact that UsbHandle class has no member variables and can be shared */
        static_assert(1u == sizeof(usb_handle_), "UsbHandle is not static");
        usb_handle_.Init(UsbHandle::FS_EXTERNAL);
    }

    /** Queue a message for transmission */
    static bool QueueMessage(const void* buffer, size_t bytes)
    {
        if(bytes == 0 || buffer == nullptr)
            return false;

        LogMessage msg(static_cast<const char*>(buffer), bytes);
        return log_fifo_.PushBack(msg);
    }

    /** Process queued messages without blocking the caller */
    static void ProcessQueue()
    {
        // Only process one message per call to avoid blocking too long
        if(!log_fifo_.IsEmpty())
        {
            const LogMessage& msg = log_fifo_.Front();
            if(TransmitDirect(msg.message, msg.length))
            {
                log_fifo_.PopFront();
                System::DelayUs(100);
            }
        }
    }

    /** Direct transmission - may block */
    static bool TransmitDirect(const void* buffer, size_t bytes)
    {
        return UsbHandle::Result::OK
               == usb_handle_.TransmitExternal((uint8_t*)buffer, bytes);
    }

    /** Non-blocking transmission - queues the message instead */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        return QueueMessage(buffer, bytes);
    }

  protected:
    /** USB Handle for CDC transfers */
    static UsbHandle usb_handle_;

    /** FIFO for log messages */
    static FIFO<LogMessage, kLogFifoSize> log_fifo_;
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

    /** Queue a message for transmission
     * For semihosting, we'll send immediately since there's no USB contention
     */
    static bool QueueMessage(const void* buffer, size_t bytes)
    {
        return Transmit(buffer, bytes);
    }

    /** Process queued messages - noop for semihosting
     * since messages are sent immediately
     */
    static void ProcessQueue() {}

    /** Direct transmission */
    static bool TransmitDirect(const void* buffer, size_t bytes)
    {
        return Transmit(buffer, bytes);
    }

    /** Transmit a block of data
     */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        if(buffer && bytes > 0)
        {
            write(STDOUT_FILENO, buffer, bytes);
            return true;
        }
        return false;
    }
};

} /* namespace daisy */

#endif //__DSY_LOGGER_IMPL_H
