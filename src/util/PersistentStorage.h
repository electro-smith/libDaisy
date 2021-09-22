#pragma once

#include "daisy_core.h"
#include "per/qspi.h"

namespace daisy
{
/** @brief Non Volatile storage class for persistent settings on an external flash device.
 *  @author shensley
 * 
 *  Storage occupied by the struct will be one word larger than 
 *  the SettingStruct used. The extra word is used to store the
 *  state of the data, and whether it's been overwritten or not.
 * 
 *  \todo - Make Save() non-blocking
 *  \todo - Add wear leveling
 * 
 **/
template <typename SettingStruct>
class PersistentStorage
{
  public:
    /** State of the storage. 
     *  When created, prior to initialiation, the state will be Unknown
     *  
     *  During initialization, the state will be changed to either FACTORY,
     *  or USER. 
     * 
     *  If this is the first time these settings are being written to the
     *  target address, the defaults will be written to that location,
     *  and the state will be set to FACTORY.
     * 
     *  Once the first user-trigger save has been made, the state will be 
     *  updated to USER to indicate that the defaults have overwritten.
     */
    enum class State
    {
        UNKNOWN = 0,
        FACTORY = 1,
        USER    = 2,
    };

    /** Constructor for storage class 
     *  \param qspi reference to the hardware qspi peripheral.
     */
    PersistentStorage(QSPIHandle &qspi)
    : qspi_(qspi),
      address_offset_(0),
      default_settings_(),
      settings_(),
      state_(State::UNKNOWN)
    {
    }

    /** Initialize Storage class
     *
     *  The values in this class will be stored as the default
     *  for restoration to 'factory' settings.
     *
     *  \param defaults should be a setting structure containing the default values.
     *      this will be updated to contain the stored data.
     *  \param address_offset offset for location on the QSPI chip (offset to base address of device).
     *      This defaults to the first address on the chip, and will be masked to the nearest multiple of 256
     **/
    void Init(const SettingStruct &defaults, uint32_t address_offset = 0)
    {
        default_settings_ = defaults;
        settings_         = defaults;
        address_offset_   = address_offset & (uint32_t)(~0xff);
        auto storage_data
            = reinterpret_cast<SaveStruct *>(qspi_.GetData(address_offset_));

        // check to see if the state is already in use.
        State cur_state = storage_data->storage_state;
        if(cur_state != State::FACTORY && cur_state != State::USER)
        {
            // Initialize the Data store State::FACTORY, and the DefaultSettings
            state_ = State::FACTORY;
            StoreSettingsIfChanged();
        }
        else
        {
            state_    = cur_state;
            settings_ = storage_data->user_data;
        }
    }

    /** Returns the state of the Persistent Data */
    State GetState() const { return state_; }

    /** Returns a reference to the setting struct */
    SettingStruct &GetSettings() { return settings_; }

    /** Performs the save operation, storing the storage */
    void Save()
    {
        state_ = State::USER;
        StoreSettingsIfChanged();
    }

    /** Restores the settings stored in the QSPI */
    void RestoreDefaults()
    {
        *settings_ = default_settings_;
        state_     = State::FACTORY;
        StoreSettingsIfChanged();
    }

  private:
    struct SaveStruct
    {
        State         storage_state;
        SettingStruct user_data;
    };

    void StoreSettingsIfChanged()
    {
        SaveStruct s;
        s.storage_state = state_;
        s.user_data     = settings_;
        // Only actually save if the new data is different
        // Use the `==operator` in custom SettingStruct to fine tune
        // what may or may not trigger the erase/save.
        auto storage_data
            = reinterpret_cast<SaveStruct *>(qspi_.GetData(address_offset_));
        if(settings_ != storage_data->user_data)
        {
            qspi_.Erase(address_offset_, address_offset_ + sizeof(s));
            qspi_.Write(address_offset_, sizeof(s), (uint8_t *)&s);
        }
    }

    QSPIHandle &  qspi_;
    uint32_t      address_offset_;
    SettingStruct default_settings_;
    SettingStruct settings_;
    State         state_;
};

} // namespace daisy