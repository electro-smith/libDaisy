
#include "logger.h"
#include "dev/sdram.h"
namespace daisy
{

char DSY_SDRAM_BSS Logger::rx_buff_[LOGGER_BUFFER];
char DSY_SDRAM_BSS Logger::tx_buff_[LOGGER_BUFFER];
bool Logger::pc_up_ = false;
bool Logger::init_done_ = false;
size_t Logger::tx_ptr_ = 0;
UsbHandle Logger::usb_handle_;

}   // namespace daisy