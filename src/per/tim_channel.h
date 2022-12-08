#include "daisy_core.h"
#include "tim.h"

/** Some notes as I set stuff up:
 *  
 *  The HAL_TIM_Base_Init() seems to cover everything we need, 
 *  everything for the "channel" I/O happens via the MspInit HAL callbacks
 *  Since we'll be handling these post-init, on channel-by-channel option
 *  we can probably avoid doing the {function}Init stuff, and populating any MspInit
 *  
 *  Also, I'm including all of the modes here for now, but I'm really focused on 
 *  implementing PWM (via DMA). So I will do all of the implementation for that, 
 *  and then see what doing IC, OC, and ONEPULSE would look like.
 * 
 *  also we'll _probably_ have to do a pimpl, but we'll see what we can do without one for the moment.
 * 
 */

namespace daisy
{
class TimChannel
{
  public:
    struct Config
    {
        /** Specifies the Channel to use */
        enum class Channel
        {
            ONE,
            TWO,
            THREE,
            FOUR,
        };

        enum class Mode
        {
            INPUT_CAPTURE,
            OUTPUT_COMPARE,
            PWM,
            ONE_PULSE,
        };

        TimerHandle* tim;
        Channel      chn;
        Mode         mode;
        Pin          pin;
        Config() : tim(nullptr), chn(Channel::ONE), mode(Mode::PWM) {}
    };

    TimChannel() {}
    ~TimChannel() {}

    /** Initializes the GPIO Pin and sets up PWM for the given channel */
    void Init(const Config& cfg);

    /** Starts the PWM output on the given channel's pin */
    void Start();

    /** Stops the PWM output on the given channel's pin */
    void Stop();

    void SetPwm(uint32_t val);

    /** Starts the DMA for the given buffer, calling the callback 
     *  when the transmission is complete.
     */
    void StartDma(void* data, size_t size, void* callback);

    const Config& GetConfig() const;

  private:
    Config cfg_;
};
} // namespace daisy