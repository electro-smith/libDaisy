/** Example showing serial reception
 *  This program initializes the daisy, and then waits for connection to a serial monitor.
 *
 *  Once the connection has been established, messages received over serial will be echoed back.
 *  This is done by writing the input to a FIFO of messages as they are received, and
 *  then from within the main while() loop, messages are popped from the FIFO, and printed back over serial.
 */
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;

/** Create a FIFO for receiving messages to echo out. */
FIFO<FixedCapStr<128>, 16> msg_fifo;

/** Callback that fires whenever new data is sent from the serial port */
void UsbCallback(uint8_t *buff, uint32_t *length)
{
    if(buff && length) /**< Check that our inputs are not null */
    {
        // Create a new string, and push it to the FIFO
        FixedCapStr<128> rx((const char *)buff, *length);
        msg_fifo.PushBack(rx);

        /** Something like this below _can_ work. (where outbuff is a global char array)
         *  However, it is ideal to leave this callback as quickly as possible.
         *  So it is recommended to handle any responses to the input elsewhere.
         */
        // strncpy(outbuff, (const char*)buff, *length);
        // hw.PrintLine(outbuff);
    }
}

int main(void)
{
    // Initialize the Daisy Seed
    hw.Init();

    // Start the log, and wait for connection
    hw.StartLog(true);

    // Set USB callback
    hw.usb_handle.SetReceiveCallback(UsbCallback,
                                     UsbHandle::UsbPeriph::FS_INTERNAL);

    /** Print an initial message once the connection occurs */
    hw.PrintLine("Send messages in your Serial Monitor!");

    while(1)
    {
        /** Whenever the FIFO has contents,
         * go through and print each message that has been received */
        while(!msg_fifo.IsEmpty())
        {
            auto msg = msg_fifo.PopFront();
            hw.Print("Received Message: ");
            hw.PrintLine(msg.Cstr());
        }

        /** Blink LED to show program is running (this indicates to
         * the user that the Serial Monitor Connection has been established) */
        hw.SetLed((System::GetNow() & 1023) > 511);
    }
}
