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
    enum class State
    {
        UNKNOWN = 0,
        FACTORY = 1,
        USER    = 2,
    };

    PersistentStorage() : settings_(nullptr) {}

    /** Initialize Storage class
     *
     *  The values in this class will be stored as the default
     *  for restoration to 'factory' settings.
     *
     *  \param qspi handle to the hardware qspi peripheral.
     *  \param settings should be a setting structure containing the default values.
     *      this will be updated to contain the stored data.
     *  \param start_address address for location on the QSPI chip (offset to base address of device).
     *      This defaults to the first address on the chip, and should be a multiple of 256
     **/
    void Init(QSPIHandle *   qspi,
              SettingStruct &settings,
              uint32_t       start_address = 0x90000000)
    {
        qspi_             = qspi;
        state_            = State::UNKNOWN;
        default_settings_ = settings; // copy data to fixed struct.
        settings_ = &settings; // copy reference to pointer for modification.
        start_address_    = start_address;
        in_place_storage_ = (SaveStruct *)(start_address_);

        // check to see if the state is already in use.
        State cur_state = in_place_storage_->storage_state;
        if(cur_state != State::FACTORY && cur_state != State::USER)
        {
            // Initialize the Data store State::FACTORY, and the DefaultSettings
            state_ = State::FACTORY;
            StoreSettings();
        }
        else
        {
            state_     = cur_state;
            *settings_ = in_place_storage_->user_data;
        }
    }

    /** Returns the state of the Persistent Data */
    State GetState() const { return state_; }

    /** Performs the save operation, storing the storage */
    void Save()
    {
        state_ = State::USER;
        StoreSettings();
    }

    /** Restores the settings stored in the QSPI */
    void RestoreDefaults()
    {
        *settings_ = default_settings_;
        state_     = State::FACTORY;
        StoreSettings();
    }

  private:
    static constexpr uint32_t kMemBaseAddr = 0x90000000;

    struct SaveStruct
    {
        State         storage_state;
        SettingStruct user_data;
    };

    void StoreSettings()
    {
        SaveStruct s;
        s.storage_state = state_;
        s.user_data     = *settings_;
        // Only actually save if the new data is different
        // Use the `==operator` in custom SettingStruct to fine tune
        // what may or may not trigger the erase/save.
        if(*settings_ != in_place_storage_->user_data)
        {
            qspi_->Erase(start_address_, start_address_ + sizeof(s));
            qspi_->Write(start_address_, sizeof(s), (uint8_t *)&s);
        }
    }

    QSPIHandle *   qspi_;
    uint32_t       start_address_;
    SettingStruct  default_settings_;
    SettingStruct *settings_;
    State          state_;
    SaveStruct *   in_place_storage_;
};

} // namespace daisy