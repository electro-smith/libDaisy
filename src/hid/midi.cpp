#include "midi.h"

namespace daisy
{
static constexpr size_t kDefaultMidiRxBufferSize = 256;

static uint8_t DMA_BUFFER_MEM_SECTION
    default_midi_rx_buffer[kDefaultMidiRxBufferSize];

MidiUartTransport::Config::Config()
: mode(UartHandler::Config::Mode::TX_RX),
  periph(UartHandler::Config::Peripheral::USART_1),
  rx({DSY_GPIOB, 7}),
  tx({DSY_GPIOB, 6}),
  rx_buffer(default_midi_rx_buffer),
  rx_buffer_size(kDefaultMidiRxBufferSize),
  rx_dma_stream(UartHandler::Config::DmaStream::DMA_1_STREAM_5),
  tx_dma_stream(UartHandler::Config::DmaStream::DMA_2_STREAM_4)
{
}

} // namespace daisy
