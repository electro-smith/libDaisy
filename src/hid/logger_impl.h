#pragma once
#ifndef __DSY_LOGGER_IMPL_H
#define __DSY_LOGGER_IMPL_H
#include <unistd.h>
#include <cstdint>
#include <cassert>
#include "hid/usb.h"
#include "per/uart.h"
#include "sys/system.h"
#include "daisy_core.h"


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
    LOGGER_UART,     /**< uart */
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

/** @brief Specialization for UART */
template <>
class LoggerImpl<LOGGER_UART>
{
  public:
    struct Config
    {
        UartHandler::Config::Peripheral uart;
        Pin                             tx_pin;
        uint32_t                        baudrate = 115200;
    };

    /** Unique to this variant, this should be called ahead of initialization */
    static void Configure(const Config cfg) { config_ = cfg; }


    /** Initialize the logging destination */
    static void Init()
    {
        if(!config_.tx_pin.IsValid())
            return;
        UartHandler::Config uart_cfg;
        uart_cfg.mode          = UartHandler::Config::Mode::TX;
        uart_cfg.periph        = config_.uart;
        uart_cfg.pin_config.tx = config_.tx_pin;
        uart_cfg.baudrate      = config_.baudrate;
        uart_.Init(uart_cfg);
    }

    /** Transmit a block of data */
    static bool Transmit(const void* buffer, size_t bytes)
    {
        return uart_.BlockingTransmit((uint8_t*)(buffer), bytes, 20)
               == UartHandler::Result::OK;
    }


  protected:
    static Config      config_;
    static UartHandler uart_;
};


} /* namespace daisy */

#endif //__DSY_LOGGER_IMPL_H
