#include "dev/max11300.h"
#include <gtest/gtest.h>
#include <cmath>
#include <bitset>

using namespace daisy;

#define UNUSED(x) (void)x

// we run these tests for a driver with 2 chips
static constexpr size_t num_devices = 2;

/**
 * This is a mock transport implementation with hooks for verifying the 
 * SPI transactions invoked by the driver at the byte level. All calls from
 * the MAX11300 driver (our system-under-test) to the transport will be
 * examined during the test to verify that the driver sent the correct
 * byte patterns to the device. The examination is done by injecting a
 * callback function into the transport.
 */
class TestTransport
{
  public:
    enum class Mode
    {
        blocking,
        dma
    };

    // These are our callback function defs. During the test, we use these
    // callbacks to examine what the MAX11300 driver is doing.
    typedef std::function<
        bool(size_t device_index, uint8_t* buff, size_t size, Mode mode)>
        TxCallback;
    typedef std::function<bool(size_t   device_index,
                               uint8_t* tx_buff,
                               uint8_t* rx_buff,
                               size_t   size,
                               Mode     mode)>
        TxRxCallback;

    /**
     * TestTransport configuration struct; used to inject the
     * examination callbacks for each of the test cases
     */
    template <size_t num_driver_devices>
    struct Config
    {
        TxCallback   tx_callback;
        TxRxCallback txrx_callback;
        void         Defaults() {}
    };

    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    // Called by the MAX11300 driver during initialization
    template <size_t num_driver_devices>
    Result Init(Config<num_driver_devices> config)
    {
        tx_callback_        = config.tx_callback;
        txrx_callback_      = config.txrx_callback;
        num_driver_devices_ = num_driver_devices;
        return Result::OK;
    }

    bool Ready() { return true; }

    Result TransmitBlocking(size_t device_index, uint8_t* buff, size_t size)
    {
        return tx_callback_(device_index, buff, size, Mode::blocking)
                   ? Result::OK
                   : Result::ERR;
    }

    Result TransmitAndReceiveBlocking(size_t   device_index,
                                      uint8_t* tx_buff,
                                      uint8_t* rx_buff,
                                      size_t   size)
    {
        return txrx_callback_(
                   device_index, tx_buff, rx_buff, size, Mode::blocking)
                   ? Result::OK
                   : Result::ERR;
    }

    Result
    TransmitDma(size_t                                      device_index,
                uint8_t*                                    buff,
                size_t                                      size,
                MAX11300Types::TransportCallbackFunctionPtr complete_callback,
                void*                                       callback_context)
    {
        const auto result = tx_callback_(device_index, buff, size, Mode::dma);

        // execute the provided complete callbacks just like the real
        // driver would do it albeit not asynchronously
        if(complete_callback)
        {
            complete_callback(callback_context,
                              result ? SpiHandle::Result::OK
                                     : SpiHandle::Result::ERR);
        }

        return result ? Result::OK : Result::ERR;
    }

    Result TransmitAndReceiveDma(
        size_t                                      device_index,
        uint8_t*                                    tx_buff,
        uint8_t*                                    rx_buff,
        size_t                                      size,
        MAX11300Types::TransportCallbackFunctionPtr complete_callback,
        void*                                       callback_context)
    {
        const auto result
            = txrx_callback_(device_index, tx_buff, rx_buff, size, Mode::dma);

        // execute the provided complete callbacks just like the real
        // driver would do it albeit not asynchronously
        if(complete_callback)
        {
            complete_callback(callback_context,
                              result ? SpiHandle::Result::OK
                                     : SpiHandle::Result::ERR);
        }

        return result ? Result::OK : Result::ERR;
    }

    size_t GetNumDevices() const { return num_driver_devices_; }

  private:
    TxCallback   tx_callback_;
    TxRxCallback txrx_callback_;
    size_t       num_driver_devices_;
};

using MAX11300Test = daisy::MAX11300Driver<TestTransport, num_devices>;

/**
 * This is a test fixture class that serves as a base for each test case.
 * It manages some common setup/teardown and provides basic testing functions for each
 * test case. This helps keep the test cases small and readable by extracting commonly used
 * "helper" code out of the test cases.
 * 
 * Internally, it holds a buffer of expected SPI transactions. The individual test cases
 * can fill that buffer with expected transactions and eventually call some function on the
 * MAX11300 driver (our SUT). The SUT will execute and call its transport layer to send and
 * receive data via SPI. We use a mocked transport layer that doesn't actually use a hardware
 * SPI - instead it refers to the buffer of expected SPI transactions; comparing the actual
 * transaction to the expected transaction and failing the test if there's a mismatch.
 * 
 * For that reason, all test cases look like this:
 * - init the driver (this is done implicitly at the start of each test when the
 *   googletest framework executes SetUp() here in the fixture base class):
 *      - fill buffer of expected transactions
 *      - call driver.Init() - comparing each transaction from the driver
 *        with the buffer of excpected transactions
 * - configure some pins (done from the test case itself by calling helper functions
 *   that this fixture provides):
 *      - fill buffer of expected transactions
 *      - call driver.<SomeConfigFunction>() - comparing each transaction from the driver
 *        with the buffer of excpected transactions
 * - some more buffer-filling and driver-calling - depending on the test case
 * - fail the test if there are still transactions in the buffer that have not been
 *   executed by the SUT (this is done implicitly at the end of eah test case when the
 *   googletest framework executes TearDown() here in the fixture base class)
 */
class MAX11300TestFixture : public ::testing::Test
{
  public:
    /**
     * This struct holds a TX SPI transaction with which the contents of "buff" will be
     * compared, byte for byte, with the output of the driver. If these do not match, 
     * The test will fail.
     */
    struct TxTransaction
    {
        std::string          description;
        size_t               device_index;
        std::vector<uint8_t> buff;
        size_t               size;
    };

    /**
     * This struct holds a TXRX SPI transaction with which the contents of "tx_buff" will be
     * compared, byte for byte, with the tx_buffer of the SPI transaction.  If these do not match, 
     * The test will fail.
     * 
     * Bytes added to the "rx_buff" will be interpreted by the driver, as if they were received
     * by the MAX11300 device itself, and can be used to further verify driver behavior.
     */
    struct TxRxTransaction
    {
        std::string          description;
        size_t               device_index;
        std::vector<uint8_t> tx_buff;
        std::vector<uint8_t> rx_buff;
        size_t               size;
    };

    /**
     * Pins/ports of the MAX11300 are freely configurable to function as 
     * ANALOG_IN (ADC), ANALOG_OUT (DAC), GPI, or GPO.  
     * This enum describes these modes.
     * We don't use the same enum from the actual driver code, so that we 
     * don't end up testing the code with itself.
     */
    enum class PinMode
    {
        NONE       = 0x0000, // Mode 0 (High impedance)
        GPI        = 0x1000, // Mode 1
        GPO        = 0x3000, // Mode 3
        ANALOG_OUT = 0x5000, // Mode 5
        ANALOG_IN  = 0x7000, // Mode 7
    };

    MAX11300TestFixture() {}
    ~MAX11300TestFixture() { clearAllTransactions(); }

    /**
     * This is called by the googletest framework before each test case starts.
     * It initializes this helper class and also verifies the Init() routine of the driver.
     */
    void SetUp()
    {
        // fill our list of expected transactions with the init transactions for
        // each of the chips
        for(size_t device_idx = 0; device_idx < num_devices; device_idx++)
        {
            const auto device_idx_str = std::to_string(device_idx);

            // This is the intial set of transactions called when "Init()" is invoked
            TxRxTransaction txrx_device_id;
            txrx_device_id.description
                = "Chip " + device_idx_str + ": Initial device ID verification";
            txrx_device_id.device_index = device_idx;
            txrx_device_id.tx_buff
                = {(MAX11300_DEVICE_ID << 1) | 1, 0x00, 0x00};
            txrx_device_id.rx_buff
                = {0x00, (uint8_t)(0x0424 >> 8), (uint8_t)0x0424};
            txrx_device_id.size = 3;
            txrx_transactions_.push_back(txrx_device_id);

            TxTransaction tx_devicectl;
            tx_devicectl.description
                = "Chip " + device_idx_str + ": Initial device configuration";
            tx_devicectl.device_index = device_idx;
            tx_devicectl.buff         = {(MAX11300_DEVCTL << 1), 0x41, 0xF3};
            tx_devicectl.size         = 3;
            tx_transactions_.push_back(tx_devicectl);

            TxRxTransaction txrx_devicectl_verify;
            txrx_devicectl_verify.description
                = "Chip " + device_idx_str
                  + ": Initial device configuration verification";
            txrx_devicectl_verify.device_index = device_idx;
            txrx_devicectl_verify.tx_buff
                = {(MAX11300_DEVCTL << 1) | 1, 0x00, 0x00};
            txrx_devicectl_verify.rx_buff = {0x00, 0x41, 0xF3};
            txrx_devicectl_verify.size    = 3;
            txrx_transactions_.push_back(txrx_devicectl_verify);

            for(size_t i = 0; i <= MAX11300Types::Pin::PIN_19; i++)
            {
                // The initial pin config transactions
                TxTransaction tx_pincfg;
                tx_pincfg.description
                    = std::string("Chip " + device_idx_str + ": Pin config #")
                          .append(std::to_string(i));
                tx_pincfg.device_index = device_idx;
                tx_pincfg.buff
                    = {(uint8_t)((MAX11300_FUNC_BASE + i) << 1), 0x00, 0x00};
                tx_pincfg.size = 3;
                tx_transactions_.push_back(tx_pincfg);

                // ...and their verification transactions
                TxRxTransaction txrx_pincfg_verify;
                txrx_pincfg_verify.description
                    = std::string("Chip " + device_idx_str
                                  + ": Pin config verification #")
                          .append(std::to_string(i));
                txrx_pincfg_verify.device_index = device_idx;
                txrx_pincfg_verify.tx_buff      = {
                    (uint8_t)(((MAX11300_FUNC_BASE + i) << 1) | 1), 0x00, 0x00};
                txrx_pincfg_verify.rx_buff = {0x00, 0x00, 0x00};
                txrx_pincfg_verify.size    = 3;
                txrx_transactions_.push_back(txrx_pincfg_verify);
            }
        }

        MAX11300Test::Config               max11300_config;
        TestTransport::Config<num_devices> transport_config;
        transport_config.tx_callback     = tx_callback;
        transport_config.txrx_callback   = txrx_callback;
        max11300_config.transport_config = transport_config;

        // Invoke the Init method now...
        if(max11300_.Init(max11300_config, &dma_buffer_)
           != MAX11300Types::Result::OK)
        {
            ADD_FAILURE() << "MAX11300 Init() invocation result was ERR";
        }

        ExpectNoRemainingTransactions();
    }

    void TearDown() { ExpectNoRemainingTransactions(); }

    /**
     * Remove all transaction fixtures.
     */
    void clearAllTransactions()
    {
        tx_transactions_.clear();
        txrx_transactions_.clear();
    }

    /**
     * Expects that no transactions remain in the buffers
     */
    void ExpectNoRemainingTransactions()
    {
        if(!tx_transactions_.empty() || !txrx_transactions_.empty())
        {
            std::string tx_transactions_string
                = "Remaining TX transactions: \n";
            for(const auto& t : tx_transactions_)
                tx_transactions_string += t.description + "\n";

            std::string txrx_transactions_string
                = "Remaining TX-RX transactions: \n";
            for(const auto& t : txrx_transactions_)
                txrx_transactions_string += t.description + "\n";

            ADD_FAILURE() << "Not all expected transactions were executed.\n"
                          << tx_transactions_string << txrx_transactions_string;
        }
    }

    bool ConfigurePinAsDigitalReadAndVerify(size_t             device_index,
                                            MAX11300Types::Pin pin,
                                            float threshold_voltage)
    {
        const auto device_idx_str = std::to_string(device_index);

        // We expect..
        // The initial pin config reset transaction
        MAX11300TestFixture::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "Chip " + device_idx_str
                                     + ": High impedance pin reset transaction";
        tx_pincfgreset.device_index = device_index;
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_transactions_.push_back(tx_pincfgreset);

        // In GPI/O mode there is an additional transaction for setting the threshold
        uint16_t gpio_threshold = MAX11300Test::VoltsTo12BitUint(
            threshold_voltage, MAX11300Types::DacVoltageRange::ZERO_TO_10);

        MAX11300TestFixture::TxTransaction tx_set_threshold;
        tx_set_threshold.description
            = "Chip " + device_idx_str + ": GPIO set threshold transaction";
        tx_set_threshold.device_index = device_index;
        tx_set_threshold.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                                 (uint8_t)(gpio_threshold >> 8),
                                 (uint8_t)gpio_threshold};
        tx_set_threshold.size = 3;
        tx_transactions_.push_back(tx_set_threshold);


        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg
            = expected_pin_cfg | static_cast<uint16_t>(PinMode::GPI);


        // The pin config transaction
        MAX11300TestFixture::TxTransaction tx_pincfg;
        tx_pincfg.description
            = "Chip " + device_idx_str + ": Pin configuration transaction";
        tx_pincfg.device_index = device_index;
        tx_pincfg.buff         = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size         = 3;
        tx_transactions_.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        MAX11300TestFixture::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description
            = "Chip " + device_idx_str
              + ": Pin config verification transaction";
        txrx_pincfg_verify.device_index = device_index;
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions_.push_back(txrx_pincfg_verify);

        bool result = max11300_.ConfigurePinAsDigitalRead(
                          device_index, pin, threshold_voltage)
                      == MAX11300Types::Result::OK;

        ExpectNoRemainingTransactions();

        return result;
    }

    bool ConfigurePinAsDigitalWriteAndVerify(size_t             device_index,
                                             MAX11300Types::Pin pin,
                                             float              output_voltage)
    {
        const auto device_idx_str = std::to_string(device_index);

        // We expect..
        // The initial pin config reset transaction
        MAX11300TestFixture::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "Chip " + device_idx_str
                                     + ": High impedance pin reset transaction";
        tx_pincfgreset.device_index = device_index;
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_transactions_.push_back(tx_pincfgreset);

        // In GPI/O mode there is an additional transaction for setting the threshold
        uint16_t gpio_threshold = MAX11300Test::VoltsTo12BitUint(
            output_voltage, MAX11300Types::DacVoltageRange::ZERO_TO_10);

        MAX11300TestFixture::TxTransaction tx_set_threshold;
        tx_set_threshold.description
            = "Chip " + device_idx_str + ": GPIO set threshold transaction";
        tx_set_threshold.device_index = device_index;
        tx_set_threshold.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                                 (uint8_t)(gpio_threshold >> 8),
                                 (uint8_t)gpio_threshold};
        tx_set_threshold.size = 3;
        tx_transactions_.push_back(tx_set_threshold);


        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg
            = expected_pin_cfg | static_cast<uint16_t>(PinMode::GPO);


        // The pin config transaction
        MAX11300TestFixture::TxTransaction tx_pincfg;
        tx_pincfg.description
            = "Chip " + device_idx_str + ": Pin configuration transaction";
        tx_pincfg.device_index = device_index;
        tx_pincfg.buff         = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size         = 3;
        tx_transactions_.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        MAX11300TestFixture::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description
            = "Chip " + device_idx_str
              + ": Pin config verification transaction";
        txrx_pincfg_verify.device_index = device_index;
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions_.push_back(txrx_pincfg_verify);

        bool result = max11300_.ConfigurePinAsDigitalWrite(
                          device_index, pin, output_voltage)
                      == MAX11300Types::Result::OK;

        ExpectNoRemainingTransactions();

        return result;
    }

    bool ConfigurePinAsAnalogReadAndVerify(size_t             device_index,
                                           MAX11300Types::Pin pin,
                                           MAX11300Types::AdcVoltageRange range)
    {
        const auto device_idx_str = std::to_string(device_index);

        // We expect..
        // The initial pin config reset transaction
        MAX11300TestFixture::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "Chip " + device_idx_str
                                     + ": High impedance pin reset transaction";
        tx_pincfgreset.device_index = device_index;
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_transactions_.push_back(tx_pincfgreset);

        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg          = expected_pin_cfg
                           | static_cast<uint16_t>(PinMode::ANALOG_IN)
                           | static_cast<uint16_t>(range);

        // In ADC mode the config includes a sample rate...
        expected_pin_cfg = expected_pin_cfg | 0x00E0;

        // The pin config transaction
        MAX11300TestFixture::TxTransaction tx_pincfg;
        tx_pincfg.description
            = "Chip " + device_idx_str + ": Pin configuration transaction";
        tx_pincfg.device_index = device_index;
        tx_pincfg.buff         = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size         = 3;
        tx_transactions_.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        MAX11300TestFixture::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description
            = "Chip " + device_idx_str
              + ": Pin config verification transaction";
        txrx_pincfg_verify.device_index = device_index;
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions_.push_back(txrx_pincfg_verify);

        bool result
            = max11300_.ConfigurePinAsAnalogRead(device_index, pin, range)
              == MAX11300Types::Result::OK;

        ExpectNoRemainingTransactions();

        return result;
    }


    bool
    ConfigurePinAsAnalogWriteAndVerify(size_t             device_index,
                                       MAX11300Types::Pin pin,
                                       MAX11300Types::DacVoltageRange range)
    {
        const auto device_idx_str = std::to_string(device_index);

        // We expect..
        // The initial pin config reset transaction
        MAX11300TestFixture::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "Chip " + device_idx_str
                                     + ": High impedance pin reset transaction";
        tx_pincfgreset.device_index = device_index;
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_transactions_.push_back(tx_pincfgreset);

        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg          = expected_pin_cfg
                           | static_cast<uint16_t>(PinMode::ANALOG_OUT)
                           | static_cast<uint16_t>(range);

        // The pin config transaction
        MAX11300TestFixture::TxTransaction tx_pincfg;
        tx_pincfg.description
            = "Chip " + device_idx_str + ": Pin configuration transaction";
        tx_pincfg.device_index = device_index;
        tx_pincfg.buff         = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size         = 3;
        tx_transactions_.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        MAX11300TestFixture::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description
            = "Chip " + device_idx_str
              + ": Pin config verification transaction";
        txrx_pincfg_verify.device_index = device_index;
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions_.push_back(txrx_pincfg_verify);

        bool result
            = max11300_.ConfigurePinAsAnalogWrite(device_index, pin, range)
              == MAX11300Types::Result::OK;

        ExpectNoRemainingTransactions();

        return result;
    }

    /**
     * The driver expects a DMA buffer in its initialisation. This is just a dummy for that.
     */
    daisy::MAX11300Types::DmaBuffer dma_buffer_;
    /**
     * The driver instance we're going to test (= system under test -> SUT)
     */
    MAX11300Test max11300_;
    /**
     * A list of TX transaction fixtures to be verified
     */
    std::vector<TxTransaction> tx_transactions_;
    /**
     * A list of TXRX transaction fixtures to be verified
     */
    std::vector<TxRxTransaction> txrx_transactions_;

  protected:
    int update_complete_callback_count_ = 0;
    int stop_auto_updates_after_        = 1;
    /** In a test, we can provide this callback function to the SUT when calling
     * MAX11300Driver.Start(), and use `this` as the callback context.
     * It will use the callback_context as a pointer and increment the
     * update_complete_callback_count_ for later examination.
     */
    static void update_complete_callback(void* callback_context)
    {
        auto& fixture
            = *reinterpret_cast<MAX11300TestFixture*>(callback_context);
        fixture.update_complete_callback_count_++;
        if(fixture.stop_auto_updates_after_ >= 0
           && fixture.update_complete_callback_count_
                  >= fixture.stop_auto_updates_after_)
        {
            fixture.max11300_.Stop();
        }
    }

  private:
    // This method verifies a TX transaction against a TxTransaction fixture
    void verifyTxTransaction(size_t device_index, uint8_t* buff, size_t size)
    {
        // Make sure we have enough fixtures to compare against...
        if(tx_transactions_.empty())
        {
            ADD_FAILURE() << "Missing TxTransaction fixture";
            return;
        }
        // Get the transaction fixture
        const auto& t = tx_transactions_.front();

        // Verify that our fixture is targeting the right device index...
        if(t.device_index != device_index)
        {
            ADD_FAILURE()
                << "In Transaction: " << t.description << ":\n"
                << "TxTransaction fixture transaction device_index != actual "
                   "transaction device_index: "
                << t.device_index << " != " << device_index;
        }

        // Verify that our fixture has the right transaction size...
        if(t.size != size)
        {
            ADD_FAILURE() << "In Transaction: " << t.description << ":\n"
                          << "TxTransaction fixture transaction size != actual "
                             "transaction size: "
                          << t.size << " != " << size;
        }

        bool tx_valid = true;

        // Here we build some useful output while verifying to help when things go wrong.
        std::string expecting = std::string("[  DETAIL  ] Expecting: ");
        std::string actual    = std::string("[  DETAIL  ] Actual   : ");
        for(std::size_t i = 0; i < size; ++i)
        {
            // expected byte output
            std::bitset<8> x(t.buff[i]);
            expecting.append(x.to_string());
            expecting.append(" ");
            // actual byte output
            std::bitset<8> a(buff[i]);
            actual.append(a.to_string());
            actual.append(" ");

            // verify equality...
            if(buff[i] != t.buff[i])
            {
                tx_valid = false;
            }
        }

        if(!tx_valid)
        {
            // The expected and actual bytes did not match. Output them
            // now to help visualize the problem...
            std::string message("[  DETAIL  ] Invalid TX: ");
            message.append(t.description);
            message.append("\n");
            message.append(expecting);
            message.append("\n");
            message.append(actual);
            message.append("\n");
            ADD_FAILURE() << message;
        }

        // remove this transaction from the queue
        tx_transactions_.erase(tx_transactions_.begin());
    }

    // This method verifies a TXRX transaction against a TxRxTransaction fixture
    void verifyTxRxTransaction(size_t   device_index,
                               uint8_t* tx_buff,
                               uint8_t* rx_buff,
                               size_t   size)
    {
        // Make sure we have enough fixtures to compare against...
        if(txrx_transactions_.empty())
        {
            ADD_FAILURE() << "Missing TxRxTransaction fixture";
            return;
        }
        // Get the transaction fixture
        const auto& t = txrx_transactions_.front();

        // Verify that our fixture is targeting the right device index...
        if(t.device_index != device_index)
        {
            ADD_FAILURE()
                << "TxRxTransaction fixture transaction device_index != actual "
                   "transaction device_index: "
                << t.device_index << " != " << device_index;
        }

        // Verify that our fixture has the right transaction size...
        if(t.size != size)
        {
            ADD_FAILURE()
                << "TxRxTransaction fixture transaction size != actual "
                   "transaction size: "
                << t.size << " != " << size;
        }

        bool tx_valid = true;

        // Here we build some useful output while verifying to help when things go wrong.
        std::string expecting = std::string("[  DETAIL  ] Expecting: ");
        std::string actual    = std::string("[  DETAIL  ] Actual   : ");
        for(std::size_t i = 0; i < size; ++i)
        {
            // expected byte output
            std::bitset<8> x(t.tx_buff[i]);
            expecting.append(x.to_string());
            expecting.append(" ");
            // actual byte output
            std::bitset<8> a(tx_buff[i]);
            actual.append(a.to_string());
            actual.append(" ");
            // verify equality...
            if(tx_buff[i] != t.tx_buff[i])
            {
                tx_valid = false;
            }
        }

        if(!tx_valid)
        {
            // The expected and actual bytes did not match. Output them
            // now to help visualize the problem...
            std::string message("[  DETAIL  ] Invalid TX: ");
            message.append(t.description);
            message.append("\n");
            message.append(expecting);
            message.append("\n");
            message.append(actual);
            message.append("\n");
            ADD_FAILURE() << message;
        }

        // Write the fixture rx_buff to the transation buffer now
        for(std::size_t i = 0; i < size; ++i)
        {
            rx_buff[i] = t.rx_buff[i];
        }

        // remove this transaction from the queue
        txrx_transactions_.erase(txrx_transactions_.begin());
    }

    // Callback for tx transactions. This is what the SUT will be
    // calling during the test execution.
    TestTransport::TxCallback tx_callback
        = [this](size_t              device_index,
                 uint8_t*            buff,
                 size_t              size,
                 TestTransport::Mode mode) -> bool
    {
        UNUSED(mode); // irrelevant for the test
        verifyTxTransaction(device_index, buff, size);
        return true;
    };

    // Callback for txrx transactions. This is what the SUT will be
    // calling during the test execution.
    TestTransport::TxRxCallback txrx_callback
        = [this](size_t              device_index,
                 uint8_t*            tx_buff,
                 uint8_t*            rx_buff,
                 size_t              size,
                 TestTransport::Mode mode) -> bool
    {
        UNUSED(mode); // irrelevant for the test
        verifyTxRxTransaction(device_index, tx_buff, rx_buff, size);
        return true;
    };
};


TEST_F(MAX11300TestFixture, verifyDriverInitializationRoutine) {}

TEST_F(MAX11300TestFixture, verifyDacPinConfiguration)
{
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        0,
        MAX11300Types::PIN_6,
        MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5));
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        1, MAX11300Types::PIN_2, MAX11300Types::DacVoltageRange::ZERO_TO_10));
}


TEST_F(MAX11300TestFixture, verifyAdcPinConfiguration)
{
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        0, MAX11300Types::PIN_14, MAX11300Types::AdcVoltageRange::ZERO_TO_10));
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        0, MAX11300Types::PIN_8, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5));
}

TEST_F(MAX11300TestFixture, verifyGpiPinConfiguration)
{
    EXPECT_TRUE(
        ConfigurePinAsDigitalReadAndVerify(0, MAX11300Types::PIN_19, 2.5f));
    EXPECT_TRUE(
        ConfigurePinAsDigitalReadAndVerify(1, MAX11300Types::PIN_14, 2.5f));
}

TEST_F(MAX11300TestFixture, verifyGpoPinConfiguration)
{
    EXPECT_TRUE(
        ConfigurePinAsDigitalWriteAndVerify(0, MAX11300Types::PIN_3, 5.0f));
    EXPECT_TRUE(
        ConfigurePinAsDigitalWriteAndVerify(1, MAX11300Types::PIN_15, 4.0f));
}


TEST_F(MAX11300TestFixture, verifyWriteAnalogPin)
{
    // Configure a single DAC pin on the second chip.
    // Set a DAC value, call Start() and expect the correct
    // byte patterns to be sent via the transport.

    MAX11300Types::Pin pin = MAX11300Types::PIN_3;
    const auto range = MAX11300Types::DacVoltageRange::ZERO_TO_10;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        1, pin, range));

    // Write two different values to a single DAC pin and verify
    // the transactions...
    // Transaction 1
    uint16_t dac_val = 3583;
    max11300_.WriteAnalogPinRaw(1, pin, dac_val);

    TxTransaction tx_write_dac1;
    tx_write_dac1.description  = "Chip 1: DAC write value transaction";
    tx_write_dac1.device_index = 1;
    tx_write_dac1.buff         = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                          (uint8_t)(dac_val >> 8),
                          (uint8_t)dac_val};
    tx_write_dac1.size         = 3;
    tx_transactions_.push_back(tx_write_dac1);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);
    update_complete_callback_count_ = 0; // reset

    // Transaction 2, this time using the "voltage" API
    const auto voltage = 1.2f;
    dac_val = MAX11300Test::VoltsTo12BitUint(voltage, range);
    max11300_.WriteAnalogPinVolts(1, pin, voltage);

    TxTransaction tx_write_dac2;
    tx_write_dac2.description  = "Chip 1: DAC write value transaction";
    tx_write_dac2.device_index = 1;
    tx_write_dac2.buff         = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                          (uint8_t)(dac_val >> 8),
                          (uint8_t)dac_val};
    tx_write_dac2.size         = 3;
    tx_transactions_.push_back(tx_write_dac2);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);
}

TEST_F(MAX11300TestFixture, verifyWriteAnalogPinMultiple)
{
    // Configure a two DAC pins on each chip.
    // Set the DAC values, call Start() and expect the correct
    // byte patterns to be sent via the transport.

    // configure some pins on both chips
    MAX11300Types::Pin pin0_0 = MAX11300Types::PIN_3;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        0, pin0_0, MAX11300Types::DacVoltageRange::ZERO_TO_10));

    MAX11300Types::Pin pin0_1 = MAX11300Types::PIN_12;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        0, pin0_1, MAX11300Types::DacVoltageRange::ZERO_TO_10));

    MAX11300Types::Pin pin1_0 = MAX11300Types::PIN_5;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        1, pin1_0, MAX11300Types::DacVoltageRange::ZERO_TO_10));

    MAX11300Types::Pin pin1_1 = MAX11300Types::PIN_17;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        1, pin1_1, MAX11300Types::DacVoltageRange::ZERO_TO_10));

    // Write two different values to two DAC pins and verify
    // the transaction
    uint16_t dac_val0_0 = 3583;
    max11300_.WriteAnalogPinRaw(0, pin0_0, dac_val0_0);
    uint16_t dac_val0_1 = 1421;
    max11300_.WriteAnalogPinRaw(0, pin0_1, dac_val0_1);
    uint16_t dac_val1_0 = 2345;
    max11300_.WriteAnalogPinRaw(1, pin1_0, dac_val1_0);
    uint16_t dac_val1_1 = 0002;
    max11300_.WriteAnalogPinRaw(1, pin1_1, dac_val1_1);

    // Here we have a 5 byte transaction as per the datasheet when configured in
    // burst mode.
    TxTransaction tx_write_dac0;
    tx_write_dac0.description  = "Chip 0: DAC write multi value transaction";
    tx_write_dac0.device_index = 0;
    tx_write_dac0.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin0_0) << 1),
                          (uint8_t)(dac_val0_0 >> 8),
                          (uint8_t)dac_val0_0,
                          (uint8_t)(dac_val0_1 >> 8),
                          (uint8_t)dac_val0_1};
    tx_write_dac0.size = 5;
    tx_transactions_.push_back(tx_write_dac0);

    TxTransaction tx_write_dac1;
    tx_write_dac1.description  = "Chip 1: DAC write multi value transaction";
    tx_write_dac1.device_index = 1;
    tx_write_dac1.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin1_0) << 1),
                          (uint8_t)(dac_val1_0 >> 8),
                          (uint8_t)dac_val1_0,
                          (uint8_t)(dac_val1_1 >> 8),
                          (uint8_t)dac_val1_1};
    tx_write_dac1.size = 5;
    tx_transactions_.push_back(tx_write_dac1);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);
}


TEST_F(MAX11300TestFixture, verifyReadAnalogPin)
{
    // Configure a single ADC pin on the first chip.
    // Call Start() and expect the correct byte patterns
    // to be sent via the transport to request the ADC value.
    // Expect the results from those transactions to be evaluated
    // correctly.

    MAX11300Types::Pin pin = MAX11300Types::PIN_7;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        0, pin, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5));

    uint16_t adc_val = 3583;

    // The expected adc read transaction...
    TxRxTransaction txrx_read_adc1;
    txrx_read_adc1.description  = "Chip 0: ADC read transaction";
    txrx_read_adc1.device_index = 0;
    txrx_read_adc1.tx_buff
        = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin) << 1) | 1), 0x00, 0x00};
    txrx_read_adc1.rx_buff = {0x00, (uint8_t)(adc_val >> 8), (uint8_t)adc_val};
    txrx_read_adc1.size    = 3;
    txrx_transactions_.push_back(txrx_read_adc1);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);

    EXPECT_EQ(adc_val, max11300_.ReadAnalogPinRaw(0, pin));
}

TEST_F(MAX11300TestFixture, verifyReadAnalogPinMultiple)
{
    // Configure two ADC pins on each chip.
    // Call Start() and expect the correct byte patterns
    // to be sent via the transport to request the ADC values.
    // Expect the results from those transactions to be evaluated
    // correctly.

    MAX11300Types::Pin pin0_0 = MAX11300Types::PIN_12;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        0, pin0_0, MAX11300Types::AdcVoltageRange::ZERO_TO_10));

    MAX11300Types::Pin pin0_1 = MAX11300Types::PIN_18;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        0, pin0_1, MAX11300Types::AdcVoltageRange::ZERO_TO_10));

    MAX11300Types::Pin pin1_0 = MAX11300Types::PIN_5;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        1, pin1_0, MAX11300Types::AdcVoltageRange::ZERO_TO_10));

    MAX11300Types::Pin pin1_1 = MAX11300Types::PIN_10;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        1, pin1_1, MAX11300Types::AdcVoltageRange::ZERO_TO_10));

    // Read two different values from the ADC pins and verify
    // the transaction
    uint16_t adc_val0_0 = 456;
    uint16_t adc_val0_1 = 4081;
    uint16_t adc_val1_0 = 0;
    uint16_t adc_val1_1 = 1235;

    // Here we have a 5 byte transaction as per the datasheet when configured in
    // burst mode; per device
    TxRxTransaction txrx_read_adc;
    txrx_read_adc.description  = "Chip 0: ADC read multi transaction";
    txrx_read_adc.device_index = 0;
    txrx_read_adc.tx_buff
        = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin0_0) << 1) | 1),
           0x00,
           0x00,
           0x00,
           0x00};
    txrx_read_adc.rx_buff = {0x00,
                             (uint8_t)(adc_val0_0 >> 8),
                             (uint8_t)adc_val0_0,
                             (uint8_t)(adc_val0_1 >> 8),
                             (uint8_t)adc_val0_1};
    txrx_read_adc.size    = 5;
    txrx_transactions_.push_back(txrx_read_adc);

    TxRxTransaction txrx_read_adc1;
    txrx_read_adc1.description  = "Chip 1: ADC read multi transaction";
    txrx_read_adc1.device_index = 1;
    txrx_read_adc1.tx_buff
        = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin1_0) << 1) | 1),
           0x00,
           0x00,
           0x00,
           0x00};
    txrx_read_adc1.rx_buff = {0x00,
                              (uint8_t)(adc_val1_0 >> 8),
                              (uint8_t)adc_val1_0,
                              (uint8_t)(adc_val1_1 >> 8),
                              (uint8_t)adc_val1_1};
    txrx_read_adc1.size    = 5;
    txrx_transactions_.push_back(txrx_read_adc1);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);

    EXPECT_EQ(adc_val0_0, max11300_.ReadAnalogPinRaw(0, pin0_0));
    EXPECT_EQ(adc_val0_1, max11300_.ReadAnalogPinRaw(0, pin0_1));
    EXPECT_EQ(adc_val1_0, max11300_.ReadAnalogPinRaw(1, pin1_0));
    EXPECT_EQ(adc_val1_1, max11300_.ReadAnalogPinRaw(1, pin1_1));
}

TEST_F(MAX11300TestFixture, verifyWriteDigitalPin)
{
    // Configure a single GPO pin on the second chip.
    // Call Start() and expect the correct byte patterns
    // to be sent via the transport to update the pin

    MAX11300Types::Pin pin1 = MAX11300Types::PIN_16;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(1, pin1, 5.0f));

    TxTransaction tx_write_gpo1;
    tx_write_gpo1.description  = "Chip 2: GPO write transaction";
    tx_write_gpo1.device_index = 1;
    tx_write_gpo1.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x00, 0x00, 0x01};
    tx_write_gpo1.size = 5;
    tx_transactions_.push_back(tx_write_gpo1);

    max11300_.WriteDigitalPin(1, pin1, true);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);
}


TEST_F(MAX11300TestFixture, verifyWriteDigitalPinMultiple)
{
    // Configure two GPO pins on each chip.
    // Call Start() and expect the correct byte patterns
    // to be sent via the transport to update the pins

    MAX11300Types::Pin pin0_0 = MAX11300Types::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(0, pin0_0, 5.0f));

    MAX11300Types::Pin pin0_1 = MAX11300Types::PIN_16;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(0, pin0_1, 5.0f));

    MAX11300Types::Pin pin1_0 = MAX11300Types::PIN_4;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(1, pin1_0, 5.0f));

    MAX11300Types::Pin pin1_1 = MAX11300Types::PIN_14;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(1, pin1_1, 5.0f));

    TxTransaction tx_write_gpo0_0;
    tx_write_gpo0_0.description  = "Chip 0: GPO write transaction 1";
    tx_write_gpo0_0.device_index = 0;
    tx_write_gpo0_0.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x02, 0x00, 0x01};
    tx_write_gpo0_0.size = 5;
    tx_transactions_.push_back(tx_write_gpo0_0);

    TxTransaction tx_write_gpo1_0;
    tx_write_gpo1_0.description  = "Chip 1: GPO write transaction 1";
    tx_write_gpo1_0.device_index = 1;
    tx_write_gpo1_0.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x40, 0x10, 0x00, 0x00};
    tx_write_gpo1_0.size = 5;
    tx_transactions_.push_back(tx_write_gpo1_0);

    max11300_.WriteDigitalPin(0, pin0_0, true);
    max11300_.WriteDigitalPin(0, pin0_1, true);
    max11300_.WriteDigitalPin(1, pin1_0, true);
    max11300_.WriteDigitalPin(1, pin1_1, true);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);
    update_complete_callback_count_ = 0; // reset

    TxTransaction tx_write_gpo0_1;
    tx_write_gpo0_1.description  = "Chip 0: GPO write transaction 2";
    tx_write_gpo0_1.device_index = 0;
    tx_write_gpo0_1.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x00, 0x00, 0x00};
    tx_write_gpo0_1.size = 5;
    tx_transactions_.push_back(tx_write_gpo0_1);

    TxTransaction tx_write_gpo1_1;
    tx_write_gpo1_1.description  = "Chip 1: GPO write transaction 2";
    tx_write_gpo1_1.device_index = 1;
    tx_write_gpo1_1.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x00, 0x00, 0x00};
    tx_write_gpo1_1.size = 5;
    tx_transactions_.push_back(tx_write_gpo1_1);

    max11300_.WriteDigitalPin(0, pin0_0, false);
    max11300_.WriteDigitalPin(0, pin0_1, false);
    max11300_.WriteDigitalPin(1, pin1_0, false);
    max11300_.WriteDigitalPin(1, pin1_1, false);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);
}

TEST_F(MAX11300TestFixture, verifyReadDigitalPin)
{
    // Configure a single GPI pin on the first chip.
    // Call Start() and expect the correct byte patterns
    // to be sent via the transport to read the pin.
    // Expect the results from those transactions to be evaluated
    // correctly.

    MAX11300Types::Pin pin1 = MAX11300Types::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(0, pin1, 2.5f));

    TxRxTransaction txrx_read_gpi1;
    txrx_read_gpi1.description  = "Chip 0: GPI read transaction";
    txrx_read_gpi1.device_index = 0;
    txrx_read_gpi1.tx_buff
        = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
    txrx_read_gpi1.rx_buff = {0x00, 0x00, 0b00000010, 0x00, 0x00};
    txrx_read_gpi1.size    = 5;
    txrx_transactions_.push_back(txrx_read_gpi1);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);

    EXPECT_TRUE(max11300_.ReadDigitalPin(0, pin1));
}

TEST_F(MAX11300TestFixture, verifyReadDigitalPinMultiple)
{
    // Configure two GPI pins on each chip.
    // Call Start() and expect the correct byte patterns
    // to be sent via the transport to update the pins.
    // Expect the results from those transactions to be evaluated
    // correctly.

    MAX11300Types::Pin pin0_0 = MAX11300Types::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(0, pin0_0, 2.5f));

    MAX11300Types::Pin pin0_1 = MAX11300Types::PIN_18;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(0, pin0_1, 2.7f));

    MAX11300Types::Pin pin1_0 = MAX11300Types::PIN_3;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(1, pin0_0, 2.5f));

    MAX11300Types::Pin pin1_1 = MAX11300Types::PIN_19;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(1, pin0_1, 2.7f));

    TxRxTransaction txrx_read_gpi0;
    txrx_read_gpi0.description  = "Chip 0: GPI read transaction";
    txrx_read_gpi0.device_index = 0;
    txrx_read_gpi0.tx_buff
        = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
    txrx_read_gpi0.rx_buff = {0x00, 0x00, 0b00000010, 0x00, 0b00000100};
    txrx_read_gpi0.size    = 5;
    txrx_transactions_.push_back(txrx_read_gpi0);

    TxRxTransaction txrx_read_gpi1;
    txrx_read_gpi1.description  = "Chip 1: GPI read transaction";
    txrx_read_gpi1.device_index = 1;
    txrx_read_gpi1.tx_buff
        = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
    txrx_read_gpi1.rx_buff = {0x00, 0x00, 0b00001000, 0x00, 0b00001000};
    txrx_read_gpi1.size    = 5;
    txrx_transactions_.push_back(txrx_read_gpi1);

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);

    EXPECT_TRUE(max11300_.ReadDigitalPin(0, pin0_0));
    EXPECT_TRUE(max11300_.ReadDigitalPin(0, pin0_1));
    EXPECT_TRUE(max11300_.ReadDigitalPin(1, pin1_0));
    EXPECT_TRUE(max11300_.ReadDigitalPin(1, pin1_1));
}

TEST_F(MAX11300TestFixture, verifyHeterogeneousPinBehavior)
{
    // Write and read multiple pins on both chips at once.
    // Expect the correct byte patterns and results.
    // Expect a fixed update order

    const uint16_t           adc_val = 3583;
    const MAX11300Types::Pin pin1    = MAX11300Types::PIN_11;
    const MAX11300Types::Pin pin2    = MAX11300Types::PIN_10;
    const MAX11300Types::Pin pin3    = MAX11300Types::PIN_5;
    const MAX11300Types::Pin pin4    = MAX11300Types::PIN_16;

    // configure all pins
    for(size_t device_index = 0; device_index < num_devices; device_index++)
    {
        // Pin 1 is a GPI pin
        EXPECT_TRUE(
            ConfigurePinAsDigitalReadAndVerify(device_index, pin1, 2.5f));

        // Pin 2 is a GPO pin
        EXPECT_TRUE(
            ConfigurePinAsDigitalWriteAndVerify(device_index, pin2, 2.5f));

        // Pin 3 is a DAC pin
        EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
            device_index, pin3, MAX11300Types::DacVoltageRange::ZERO_TO_10));

        // Pin 4 is an ADC pin
        EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
            device_index, pin4, MAX11300Types::AdcVoltageRange::ZERO_TO_10));
    }

    // set expectationed transactions and write data to the driver
    for(size_t device_index = 0; device_index < num_devices; device_index++)
    {
        const auto device_idx_str = std::to_string(device_index);

        uint16_t dac_val = 1234;
        max11300_.WriteAnalogPinRaw(device_index, pin3, dac_val);
        max11300_.WriteDigitalPin(device_index, pin2, true);

        TxTransaction tx_write_dac1;
        tx_write_dac1.description
            = "Chip " + device_idx_str + ": DAC write value transaction";
        tx_write_dac1.device_index = device_index;
        tx_write_dac1.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin3) << 1),
                              (uint8_t)(dac_val >> 8),
                              (uint8_t)dac_val};
        tx_write_dac1.size = 3;
        tx_transactions_.push_back(tx_write_dac1);

        TxRxTransaction txrx_read_adc1;
        txrx_read_adc1.description
            = "Chip " + device_idx_str + ": ADC read transaction";
        txrx_read_adc1.device_index = device_index;
        txrx_read_adc1.tx_buff
            = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin4) << 1) | 1), 0x00, 0x00};
        txrx_read_adc1.rx_buff
            = {0x00, (uint8_t)(adc_val >> 8), (uint8_t)adc_val};
        txrx_read_adc1.size = 3;
        txrx_transactions_.push_back(txrx_read_adc1);

        TxTransaction tx_write_gpo1;
        tx_write_gpo1.description
            = "Chip " + device_idx_str + ": GPO write transaction";
        tx_write_gpo1.device_index = device_index;
        tx_write_gpo1.buff
            = {(uint8_t)(MAX11300_GPODAT << 1), 0b00000100, 0x00, 0x00, 0x00};
        tx_write_gpo1.size = 5;
        tx_transactions_.push_back(tx_write_gpo1);

        TxRxTransaction txrx_read_gpi1;
        txrx_read_gpi1.description
            = "Chip " + device_idx_str + ": GPI read transaction";
        txrx_read_gpi1.device_index = device_index;
        txrx_read_gpi1.tx_buff
            = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
        txrx_read_gpi1.rx_buff = {0x00, 0b00001000, 0x00, 0x00, 0x00};
        txrx_read_gpi1.size    = 5;
        txrx_transactions_.push_back(txrx_read_gpi1);
    }

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    EXPECT_EQ(update_complete_callback_count_, 1);

    EXPECT_TRUE(max11300_.ReadDigitalPin(0, pin1));
    EXPECT_TRUE(max11300_.ReadDigitalPin(1, pin1));
    EXPECT_EQ(adc_val, max11300_.ReadAnalogPinRaw(0, pin4));
    EXPECT_EQ(adc_val, max11300_.ReadAnalogPinRaw(1, pin4));
}

TEST_F(MAX11300TestFixture, verifyAutoUpdate)
{
    // Configure a single GPI pin on the first chip.
    // Call Start() and enable auto updating.
    // Expect the correct number of updates and
    // update_complete callbacks

    MAX11300Types::Pin pin1 = MAX11300Types::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(0, pin1, 2.5f));

    // abort test case after 10 updates
    stop_auto_updates_after_ = 10;

    // queue expected transactions
    for(int i = 0; i < stop_auto_updates_after_; i++)
    {
        TxRxTransaction txrx_read_gpi1;
        txrx_read_gpi1.description
            = "Chip 0: GPI read transaction " + std::to_string(i);
        txrx_read_gpi1.device_index = 0;
        txrx_read_gpi1.tx_buff
            = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
        txrx_read_gpi1.rx_buff = {0x00, 0x00, 0b00000010, 0x00, 0x00};
        txrx_read_gpi1.size    = 5;
        txrx_transactions_.push_back(txrx_read_gpi1);
    }

    // this will call the mocked transport, verifying each transaction
    // against the expectation we set in the lines above
    EXPECT_TRUE(max11300_.Start(&update_complete_callback, this)
                == MAX11300Types::Result::OK);
    // the correct number of callbacks was made
    EXPECT_EQ(update_complete_callback_count_, stop_auto_updates_after_);
}

TEST(dev_MAX11300, a_VoltsTo12BitUint)
{
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -1, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              0);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  0, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              0);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  2.5, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              1023);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  5, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              2047);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  7.5, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              3071);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  10, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              4095);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  12, MAX11300Types::DacVoltageRange::ZERO_TO_10),
              4095);

    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -5.5, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              0);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -5, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              0);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -2.5, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              1023);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  0, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              2047);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  2.5, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              3071);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  5, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              4095);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  7, MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5),
              4095);

    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -12, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              0);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -10, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              0);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -7.5, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              1023);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -5, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              2047);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  -2.5, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              3071);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  0, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              4095);
    EXPECT_EQ(MAX11300Test::VoltsTo12BitUint(
                  2, MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0),
              4095);
}

TEST(dev_MAX11300, b_TwelveBitUintToVolts)
{
    float oneLsbAtTenVolts = 10.0f / 4096.0f;

    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        0, MAX11300Types::AdcVoltageRange::ZERO_TO_10),
                    0);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    1023, MAX11300Types::AdcVoltageRange::ZERO_TO_10),
                2.5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    2047, MAX11300Types::AdcVoltageRange::ZERO_TO_10),
                5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    3071, MAX11300Types::AdcVoltageRange::ZERO_TO_10),
                7.5,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        4095, MAX11300Types::AdcVoltageRange::ZERO_TO_10),
                    10);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        5000, MAX11300Types::AdcVoltageRange::ZERO_TO_10),
                    10);

    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        0, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5),
                    -5);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    1023, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5),
                -2.5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    2047, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5),
                0,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    3071, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5),
                2.5,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        4095, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5),
                    5);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        5000, MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5),
                    5);

    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        0, MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0),
                    -10);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    1023, MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0),
                -7.5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    2047, MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0),
                -5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    3071, MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0),
                -2.5,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        4095, MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0),
                    0);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        5000, MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0),
                    0);

    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        0, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5),
                    0);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    1023, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5),
                0.625,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    2047, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5),
                1.25,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300Test::TwelveBitUintToVolts(
                    3071, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5),
                1.875,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        4095, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5),
                    2.5);
    EXPECT_FLOAT_EQ(MAX11300Test::TwelveBitUintToVolts(
                        5000, MAX11300Types::AdcVoltageRange::ZERO_TO_2P5),
                    2.5);
}
