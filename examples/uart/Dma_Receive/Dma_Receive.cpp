#include "daisy_patch.h"

using namespace daisy;

DaisyPatch  hw;
UartHandler uart;

// buffer to read into
uint8_t DMA_BUFFER_MEM_SECTION buff[4];
uint8_t dma_buffer[1024];

FIFO<uint8_t, 1024> my_fifo;


void RestartUart(void* state, UartHandler::Result res);

// dma end callback, will start a new DMA transfer
void RestartUart(void* state, UartHandler::Result res)
{
    uart.DmaReceive(buff, 3, NULL, RestartUart, NULL);
}

void myMidiCallback(uint8_t*            data,
                    size_t              size,
                    void*               context,
                    UartHandler::Result res)
{
    for(size_t i = 0; i < size; i++)
        my_fifo.PushBack(data[i]);
}

// void Audio(AudioHandle::InputBuffer  in,
//            AudioHandle::OutputBuffer out,
//            size_t                    size)
// {
//     for(size_t i = 0; i < size; i++)
//     {
//         out[0][i] = in[0][i];
//         out[1][i] = in[1][i];
//         out[2][i] = in[2][i];
//         out[3][i] = in[3][i];
//     }
// }

int main(void)
{
    // start the Daisy Patch
    hw.Init();

    // hw.StartAudio(Audio);

    // reset our receive buffer
    for(int i = 0; i < 4; i++)
    {
        buff[i] = 0;
    }

    // set up our UART peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::TX_RX;
    uart_conf.baudrate      = 31250;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // initialize the UART peripheral, and start reading
    uart.Init(uart_conf);
    //uart.DmaReceive(buff, 3, NULL, RestartUart, NULL);
    uart.DmaListenStart(dma_buffer, 1024, myMidiCallback, nullptr);

    while(1)
    {
        // while(uart.ReadableFifo())
        // {
        //     auto foo = uart.PopFifo();
        // }
        while(!my_fifo.IsEmpty())
        {
            auto foo = my_fifo.PopFront();
            uart.BlockingTransmit(&foo, 1, 1);
        }
        // // clear the display
        // hw.display.Fill(false);

        // // draw the title text
        // char cstr[22];
        // sprintf(cstr, "Uart DMA Rx Test");
        // hw.display.SetCursor(0, 0);
        // hw.display.WriteString(cstr, Font_7x10, true);

        // // draw the receive buffer contents
        // sprintf(cstr, "%d %d %d", buff[0], buff[1], buff[2]);
        // hw.display.SetCursor(0, 12);
        // hw.display.WriteString(cstr, Font_7x10, true);

        // // update the display
        // hw.display.Update();

        // // wait 100 ms
        // System::Delay(100);
    }
}
