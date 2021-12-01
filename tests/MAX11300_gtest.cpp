#include "dev/max11300.h"
#include <gtest/gtest.h>
#include <cmath>
#include <bitset>

using namespace daisy;

#define UNUSED(x) (void)x

/**
 * This is a mock transport implementation with hooks for verifying the 
 * SPI transactions invoked by the driver at the byte level.
 */
class TestTransport
{
  public:
    // These are our callback function defs.
    typedef std::function<bool(uint8_t* buff, size_t size, uint32_t wait_us)>
        TxCallback;
    typedef std::function<bool(uint8_t* tx_buff, uint8_t* rx_buff, size_t size)>
        TxRxCallback;

    /**
     * TestTransport configuration struct
     */
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

    void Init(Config config)
    {
        tx_callback   = config.tx_callback;
        txrx_callback = config.txrx_callback;
    }

    bool Ready() { return true; }

    TestTransport::Result Transmit(uint8_t* buff, size_t size, uint32_t wait_us)
    {
        if(!tx_callback(buff, size, wait_us))
        {
            return Result::ERR;
        }
        return Result::OK;
    }

    TestTransport::Result
    TransmitAndReceive(uint8_t* tx_buff, uint8_t* rx_buff, size_t size)
    {
        if(!txrx_callback(tx_buff, rx_buff, size))
        {
            return Result::ERR;
        }
        return Result::OK;
    }

  private:
    TxCallback   tx_callback;
    TxRxCallback txrx_callback;
};

using MAX11300Test = daisy::MAX11300Driver<TestTransport>;

/**
 * This is a helper class for persisting a structured set of SPI transactions, 
 * which can then be verified when driver methods are invoked.
 */
class Max11300TestHelper : public ::testing::Test
{
  public:
    /**
     * This struct holds a TX SPI transaction with which the contents of "buff" will be
     * compared, byte for byte, with the output of the driver.  If these do not match, 
     * The test will fail.
     */
    struct TxTransaction
    {
        std::string          description;
        std::vector<uint8_t> buff;
        size_t               size;
        uint32_t             wait;
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
        std::vector<uint8_t> tx_buff;
        std::vector<uint8_t> rx_buff;
        size_t               size;
    };

    /**
     * Pins/ports of the MAX11300 are freely configurable to function as 
     * ANALOG_IN (ADC), ANALOG_OUT (DAC), GPI, or GPO.  
     * This enum describes these modes.
     */
    enum class PinMode
    {
        NONE       = 0x0000, // Mode 0 (High impedance)
        GPI        = 0x1000, // Mode 1
        GPO        = 0x3000, // Mode 3
        ANALOG_OUT = 0x5000, // Mode 5
        ANALOG_IN  = 0x7000, // Mode 7
    };

    /**
     * The PinConfig struct holds the necessary information needed to configure a
     * pin/port of the MAX11300, as well as a pointer to the stateful pin data.  
     */
    struct PinConfig
    {
        PinMode                    mode;  /**< & */
        MAX11300Test::VoltageRange range; /**< & */
        /**
         * This is a voltage value used as follows:
         * 
         *  GPI - Defines what input voltage constituates a logical 1
         *  GPO - The output voltage of the pin at logical 1
         */
        float threshold;
        /**
         * In the case of ANALOG_IN or ANALOG_OUT modes, this points to the 
         * current 12 bit value of the pin.
         */
        void Defaults()
        {
            mode      = PinMode::NONE;
            range     = MAX11300Test::VoltageRange::ZERO_TO_10;
            threshold = 0.0f;
        }
    };

    Max11300TestHelper() {}
    ~Max11300TestHelper() { clearAllTransactions(); }

    /**
     * Initializing this helper class also verifies the Init() routine of the driver.
     */
    void SetUp()
    {
        // This is the intial set of transactions called when "Init()" is invoked
        TxRxTransaction txrx_device_id;
        txrx_device_id.description = "Initial device ID verification";
        txrx_device_id.tx_buff = {(MAX11300_DEVICE_ID << 1) | 1, 0x00, 0x00};
        txrx_device_id.rx_buff
            = {0x00, (uint8_t)(0x0424 >> 8), (uint8_t)0x0424};
        txrx_device_id.size = 3;
        txrx_transactions.push_back(txrx_device_id);

        TxTransaction tx_devicectl;
        tx_devicectl.description = "Initial device configuration";
        tx_devicectl.buff        = {(MAX11300_DEVCTL << 1), 0x41, 0xF7};
        tx_devicectl.size        = 3;
        tx_devicectl.wait        = 0;
        tx_transactions.push_back(tx_devicectl);

        TxRxTransaction txrx_devicectl_verify;
        txrx_devicectl_verify.description
            = "Initial device configuration verification";
        txrx_devicectl_verify.tx_buff
            = {(MAX11300_DEVCTL << 1) | 1, 0x00, 0x00};
        txrx_devicectl_verify.rx_buff = {0x00, 0x41, 0xF7};
        txrx_devicectl_verify.size    = 3;
        txrx_transactions.push_back(txrx_devicectl_verify);

        for(size_t i = 0; i <= MAX11300::Pin::PIN_19; i++)
        {
            // The initial pin config transactions
            TxTransaction tx_pincfg;
            tx_pincfg.description
                = std::string("Pin config #").append(std::to_string(i));
            tx_pincfg.buff
                = {(uint8_t)((MAX11300_FUNC_BASE + i) << 1), 0x01, 0x00};
            tx_pincfg.size = 3;
            tx_pincfg.wait = 0;
            tx_transactions.push_back(tx_pincfg);

            // ...and their verification transactions
            TxRxTransaction txrx_pincfg_verify;
            txrx_pincfg_verify.description
                = std::string("Pin config verification #")
                      .append(std::to_string(i));
            txrx_pincfg_verify.tx_buff
                = {(uint8_t)(((MAX11300_FUNC_BASE + i) << 1) | 1), 0x00, 0x00};
            txrx_pincfg_verify.rx_buff = {0x00, 0x00, 0x00};
            txrx_pincfg_verify.size    = 3;
            txrx_transactions.push_back(txrx_pincfg_verify);
        }


        MAX11300Driver<TestTransport>::Config max11300_config;
        TestTransport::Config                 transport_config;
        transport_config.tx_callback     = tx_callback;
        transport_config.txrx_callback   = txrx_callback;
        max11300_config.transport_config = transport_config;

        // Invoke the Init method now...
        if(max11300.Init(max11300_config) != MAX11300Test::Result::OK)
        {
            ADD_FAILURE() << "MAX11300 Init() invocation result was ERR";
        }
        // ...and clean up...
        clearAllTransactions();
    }

    void TearDown() {}

    /**
     * Remove all transaction fixtures.
     */
    void clearAllTransactions()
    {
        for(size_t i = 0; i < txrx_transactions.size(); i++)
        {
            txrx_transactions[i].rx_buff.clear();
            txrx_transactions[i].tx_buff.clear();
            txrx_transactions[i].description.clear();
        }
        for(size_t i = 0; i < tx_transactions.size(); i++)
        {
            tx_transactions[i].buff.clear();
            tx_transactions[i].description.clear();
        }
        tx_transactions.clear();
        txrx_transactions.clear();
        tx_transaction_count   = 0;
        txrx_transaction_count = 0;
    }


    bool ConfigurePinAsDigitalReadAndVerify(MAX11300Test::Pin pin,
                                            float             threshold_voltage)
    {
        // We expect..
        // The initial pin config reset transaction
        Max11300TestHelper::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "High impedance pin reset transaction";
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_pincfgreset.wait = 0;
        tx_transactions.push_back(tx_pincfgreset);

        // In GPI/O mode there is an additional transaction for setting the threshold
        uint16_t gpio_threshold = MAX11300Test::VoltsTo12BitUint(
            threshold_voltage, MAX11300Test::VoltageRange::ZERO_TO_10);

        Max11300TestHelper::TxTransaction tx_set_threshold;
        tx_set_threshold.description = "GPIO set threshold transaction";
        tx_set_threshold.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                                 (uint8_t)(gpio_threshold >> 8),
                                 (uint8_t)gpio_threshold};
        tx_set_threshold.size = 3;
        tx_set_threshold.wait = 0;
        tx_transactions.push_back(tx_set_threshold);


        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg
            = expected_pin_cfg | static_cast<uint16_t>(PinMode::GPI)
              | static_cast<uint16_t>(MAX11300Test::VoltageRange::ZERO_TO_10);


        // The pin config transaction
        Max11300TestHelper::TxTransaction tx_pincfg;
        tx_pincfg.description = "Pin configuration transaction";
        tx_pincfg.buff        = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size        = 3;
        tx_pincfg.wait        = 0;
        tx_transactions.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        Max11300TestHelper::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description = "Pin config verification transaction";
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions.push_back(txrx_pincfg_verify);

        bool result = max11300.ConfigurePinAsDigitalRead(pin, threshold_voltage)
                      == MAX11300Test::Result::OK;

        clearAllTransactions();

        return result;
    }

    bool ConfigurePinAsDigitalWriteAndVerify(MAX11300Test::Pin pin,
                                             float             output_voltage)
    {
        // We expect..
        // The initial pin config reset transaction
        Max11300TestHelper::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "High impedance pin reset transaction";
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_pincfgreset.wait = 0;
        tx_transactions.push_back(tx_pincfgreset);

        // In GPI/O mode there is an additional transaction for setting the threshold
        uint16_t gpio_threshold = MAX11300Test::VoltsTo12BitUint(
            output_voltage, MAX11300Test::VoltageRange::ZERO_TO_10);

        Max11300TestHelper::TxTransaction tx_set_threshold;
        tx_set_threshold.description = "GPIO set threshold transaction";
        tx_set_threshold.buff = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                                 (uint8_t)(gpio_threshold >> 8),
                                 (uint8_t)gpio_threshold};
        tx_set_threshold.size = 3;
        tx_set_threshold.wait = 0;
        tx_transactions.push_back(tx_set_threshold);


        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg
            = expected_pin_cfg | static_cast<uint16_t>(PinMode::GPO)
              | static_cast<uint16_t>(MAX11300Test::VoltageRange::ZERO_TO_10);


        // The pin config transaction
        Max11300TestHelper::TxTransaction tx_pincfg;
        tx_pincfg.description = "Pin configuration transaction";
        tx_pincfg.buff        = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size        = 3;
        tx_pincfg.wait        = 0;
        tx_transactions.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        Max11300TestHelper::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description = "Pin config verification transaction";
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions.push_back(txrx_pincfg_verify);

        bool result = max11300.ConfigurePinAsDigitalWrite(pin, output_voltage)
                      == MAX11300Test::Result::OK;

        clearAllTransactions();

        return result;
    }

    bool ConfigurePinAsAnalogReadAndVerify(MAX11300Test::Pin          pin,
                                           MAX11300Test::VoltageRange range)
    {
        // We expect..
        // The initial pin config reset transaction
        Max11300TestHelper::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "High impedance pin reset transaction";
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_pincfgreset.wait = 0;
        tx_transactions.push_back(tx_pincfgreset);

        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg          = expected_pin_cfg
                           | static_cast<uint16_t>(PinMode::ANALOG_IN)
                           | static_cast<uint16_t>(range);

        // In ADC mode the config includes a sample rate...
        expected_pin_cfg = expected_pin_cfg | 0x00E0;

        // The pin config transaction
        Max11300TestHelper::TxTransaction tx_pincfg;
        tx_pincfg.description = "Pin configuration transaction";
        tx_pincfg.buff        = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size        = 3;
        tx_pincfg.wait        = 0;
        tx_transactions.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        Max11300TestHelper::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description = "Pin config verification transaction";
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions.push_back(txrx_pincfg_verify);

        bool result = max11300.ConfigurePinAsAnalogRead(pin, range)
                      == MAX11300Test::Result::OK;

        clearAllTransactions();

        return result;
    }


    bool ConfigurePinAsAnalogWriteAndVerify(MAX11300Test::Pin          pin,
                                            MAX11300Test::VoltageRange range)
    {
        // We expect..
        // The initial pin config reset transaction
        Max11300TestHelper::TxTransaction tx_pincfgreset;
        tx_pincfgreset.description = "High impedance pin reset transaction";
        tx_pincfgreset.buff
            = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1), 0x00, 0x00};
        tx_pincfgreset.size = 3;
        tx_pincfgreset.wait = 0;
        tx_transactions.push_back(tx_pincfgreset);

        // Now we handle the expected pin configuration
        uint16_t expected_pin_cfg = 0x0000;
        expected_pin_cfg          = expected_pin_cfg
                           | static_cast<uint16_t>(PinMode::ANALOG_OUT)
                           | static_cast<uint16_t>(range);

        // The pin config transaction
        Max11300TestHelper::TxTransaction tx_pincfg;
        tx_pincfg.description = "Pin configuration transaction";
        tx_pincfg.buff        = {(uint8_t)((MAX11300_FUNC_BASE + pin) << 1),
                          (uint8_t)(expected_pin_cfg >> 8),
                          (uint8_t)expected_pin_cfg};
        tx_pincfg.size        = 3;
        tx_pincfg.wait        = 0;
        tx_transactions.push_back(tx_pincfg);

        // ...and the pin config verification transaction
        Max11300TestHelper::TxRxTransaction txrx_pincfg_verify;
        txrx_pincfg_verify.description = "Pin config verification transaction";
        txrx_pincfg_verify.tx_buff
            = {(uint8_t)(((MAX11300_FUNC_BASE + pin) << 1) | 1), 0x00, 0x00};
        txrx_pincfg_verify.rx_buff = {
            0x00, (uint8_t)(expected_pin_cfg >> 8), (uint8_t)expected_pin_cfg};
        txrx_pincfg_verify.size = 3;
        txrx_transactions.push_back(txrx_pincfg_verify);

        bool result = max11300.ConfigurePinAsAnalogWrite(pin, range)
                      == MAX11300Test::Result::OK;

        clearAllTransactions();

        return result;
    }

    /**
     * The driver instance
     */
    MAX11300Test max11300;
    /**
     * A list of TX transaction fixtures to be verified
     */
    std::vector<TxTransaction> tx_transactions;
    /**
     * A list of TXRX transaction fixtures to be verified
     */
    std::vector<TxRxTransaction> txrx_transactions;

    // Counters to keep track of how many transactions the driver has invoked
    size_t tx_transaction_count   = 0;
    size_t txrx_transaction_count = 0;


  private:
    // This method verifies a TX transaction against a TxTransaction fixture
    void verifyTxTransaction(uint8_t* buff, size_t size, uint32_t wait_us)
    {
        // TODO, how to best verify wait_us?
        UNUSED(wait_us);

        // Increment the tx count and make sure we have enough fixtures...
        tx_transaction_count++;
        if(tx_transactions.size() < tx_transaction_count)
        {
            ADD_FAILURE() << "Missing TxTransaction fixture";
        }
        // Get the transaction fixture
        TxTransaction t = tx_transactions.at(tx_transaction_count - 1);

        // Verify that our fixture has the right transaction size...
        if(t.size != size)
        {
            ADD_FAILURE() << "TxTransaction fixture transaction size != actual "
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
    }

    // This method verifies a TXRX transaction against a TxRxTransaction fixture
    void verifyTxRxTransaction(uint8_t* tx_buff, uint8_t* rx_buff, size_t size)
    {
        // Increment the txrx count and make sure we have enough fixtures...
        txrx_transaction_count++;
        if(txrx_transactions.size() < txrx_transaction_count)
        {
            ADD_FAILURE() << "Missing TxRxTransaction fixture";
        }
        // Get the transaction fixture
        TxRxTransaction t = txrx_transactions.at(txrx_transaction_count - 1);

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
    }

    // Callback for tx transactions
    TestTransport::TxCallback tx_callback
        = [this](uint8_t* buff, size_t size, uint32_t wait_us) -> bool {
        verifyTxTransaction(buff, size, wait_us);
        return true;
    };

    // Callback for txrx transactions
    TestTransport::TxRxCallback txrx_callback
        = [this](uint8_t* tx_buff, uint8_t* rx_buff, size_t size) -> bool {
        verifyTxRxTransaction(tx_buff, rx_buff, size);
        return true;
    };
};


TEST_F(Max11300TestHelper, verifyDriverInitializationRoutine) {}

TEST_F(Max11300TestHelper, verifyDacPinConfiguration)
{
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        MAX11300Test::PIN_6, MAX11300Test::VoltageRange::NEGATIVE_5_TO_5));
}


TEST_F(Max11300TestHelper, verifyAdcPinConfiguration)
{
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        MAX11300Test::PIN_14, MAX11300Test::VoltageRange::ZERO_TO_10));
}

TEST_F(Max11300TestHelper, verifyGpiPinConfiguration)
{
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(MAX11300Test::PIN_19, 2.5f));
}

TEST_F(Max11300TestHelper, verifyGpoPinConfiguration)
{
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(MAX11300Test::PIN_3, 5.0f));
}


TEST_F(Max11300TestHelper, verifyWriteAnalogPin)
{
    MAX11300Test::Pin pin = MAX11300Test::PIN_3;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        pin, MAX11300Test::VoltageRange::ZERO_TO_10));

    // Write two different values to a single DAC pin and verify
    // the transactions...
    // Transaction 1
    uint16_t dac_val = 3583;
    max11300.WriteAnalogPinRaw(pin, dac_val);

    TxTransaction tx_write_dac1;
    tx_write_dac1.description = "DAC write value transaction";
    tx_write_dac1.buff        = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                          (uint8_t)(dac_val >> 8),
                          (uint8_t)dac_val};
    tx_write_dac1.size        = 3;
    tx_write_dac1.wait        = 0;
    tx_transactions.push_back(tx_write_dac1);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    // Transaction 2
    dac_val = 1421;
    max11300.WriteAnalogPinRaw(pin, dac_val);

    TxTransaction tx_write_dac2;
    tx_write_dac2.description = "DAC write value transaction";
    tx_write_dac2.buff        = {(uint8_t)((MAX11300_DACDAT_BASE + pin) << 1),
                          (uint8_t)(dac_val >> 8),
                          (uint8_t)dac_val};
    tx_write_dac2.size        = 3;
    tx_write_dac2.wait        = 0;
    tx_transactions.push_back(tx_write_dac2);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);
}

TEST_F(Max11300TestHelper, verifyWriteAnalogPinMultiple)
{
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_3;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        pin1, MAX11300Test::VoltageRange::ZERO_TO_10));

    MAX11300Test::Pin pin2 = MAX11300Test::PIN_12;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        pin2, MAX11300Test::VoltageRange::ZERO_TO_10));

    // Write two different values to two DAC pins and verify
    // the transaction
    uint16_t dac_val1 = 3583;
    max11300.WriteAnalogPinRaw(pin1, dac_val1);

    uint16_t dac_val2 = 1421;
    max11300.WriteAnalogPinRaw(pin2, dac_val2);

    // Here we have a 5 byte transaction as per the datasheet when configured in
    // burst mode.
    TxTransaction tx_write_dac1;
    tx_write_dac1.description = "DAC write multi value transaction";
    tx_write_dac1.buff        = {(uint8_t)((MAX11300_DACDAT_BASE + pin1) << 1),
                          (uint8_t)(dac_val1 >> 8),
                          (uint8_t)dac_val1,
                          (uint8_t)(dac_val2 >> 8),
                          (uint8_t)dac_val2};
    tx_write_dac1.size        = 5;
    tx_write_dac1.wait        = 0;
    tx_transactions.push_back(tx_write_dac1);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);
}


TEST_F(Max11300TestHelper, verifyReadAnalogPin)
{
    MAX11300Test::Pin pin = MAX11300Test::PIN_7;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        pin, MAX11300Test::VoltageRange::NEGATIVE_5_TO_5));

    uint16_t adc_val = 3583;

    // The expected adc read transaction...
    TxRxTransaction txrx_read_adc1;
    txrx_read_adc1.description = "ADC read transaction";
    txrx_read_adc1.tx_buff
        = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin) << 1) | 1), 0x00, 0x00};
    txrx_read_adc1.rx_buff = {0x00, (uint8_t)(adc_val >> 8), (uint8_t)adc_val};
    txrx_read_adc1.size    = 3;
    txrx_transactions.push_back(txrx_read_adc1);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    EXPECT_EQ(adc_val, max11300.ReadAnalogPinRaw(pin));
}

TEST_F(Max11300TestHelper, verifyReadAnalogPinMultiple)
{
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_12;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        pin1, MAX11300Test::VoltageRange::ZERO_TO_10));

    MAX11300Test::Pin pin2 = MAX11300Test::PIN_18;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        pin2, MAX11300Test::VoltageRange::ZERO_TO_10));

    // Read two different values from the ADC pins and verify
    // the transaction
    uint16_t adc_val1 = 456;
    uint16_t adc_val2 = 4081;

    // Here we have a 5 byte transaction as per the datasheet when configured in
    // burst mode.
    TxRxTransaction txrx_read_adc;
    txrx_read_adc.description = "ADC read multi transaction";
    txrx_read_adc.tx_buff
        = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin1) << 1) | 1),
           0x00,
           0x00,
           0x00,
           0x00};
    txrx_read_adc.rx_buff = {0x00,
                             (uint8_t)(adc_val1 >> 8),
                             (uint8_t)adc_val1,
                             (uint8_t)(adc_val2 >> 8),
                             (uint8_t)adc_val2};
    txrx_read_adc.size    = 5;
    txrx_transactions.push_back(txrx_read_adc);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    EXPECT_EQ(adc_val1, max11300.ReadAnalogPinRaw(pin1));
    EXPECT_EQ(adc_val2, max11300.ReadAnalogPinRaw(pin2));
}

TEST_F(Max11300TestHelper, verifyWriteDigitalPin)
{
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_16;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(pin1, 5.0f));

    TxTransaction tx_write_gpo1;
    tx_write_gpo1.description = "GPO write transaction";
    tx_write_gpo1.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x00, 0x00, 0x01};
    tx_write_gpo1.size = 5;
    tx_transactions.push_back(tx_write_gpo1);

    max11300.WriteDigitalPin(pin1, true);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);
}


TEST_F(Max11300TestHelper, verifyWriteDigitalPinMultiple)
{
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(pin1, 5.0f));

    MAX11300Test::Pin pin2 = MAX11300Test::PIN_16;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(pin2, 5.0f));

    TxTransaction tx_write_gpo1;
    tx_write_gpo1.description = "GPO write transaction 1";
    tx_write_gpo1.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x02, 0x00, 0x01};
    tx_write_gpo1.size = 5;
    tx_transactions.push_back(tx_write_gpo1);

    max11300.WriteDigitalPin(pin1, true);
    max11300.WriteDigitalPin(pin2, true);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    TxTransaction tx_write_gpo2;
    tx_write_gpo2.description = "GPO write transaction 2";
    tx_write_gpo2.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0x00, 0x00, 0x00, 0x00};
    tx_write_gpo2.size = 5;
    tx_transactions.push_back(tx_write_gpo2);

    max11300.WriteDigitalPin(pin1, false);
    max11300.WriteDigitalPin(pin2, false);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);
}


TEST_F(Max11300TestHelper, verifyReadDigitalPin)
{
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(pin1, 2.5f));

    TxRxTransaction txrx_read_gpi1;
    txrx_read_gpi1.description = "GPI read transaction";
    txrx_read_gpi1.tx_buff
        = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
    txrx_read_gpi1.rx_buff = {0x00, 0x00, 0b00000010, 0x00, 0x00};
    txrx_read_gpi1.size    = 5;
    txrx_transactions.push_back(txrx_read_gpi1);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    EXPECT_TRUE(max11300.ReadDigitalPin(pin1));
}


TEST_F(Max11300TestHelper, verifyReadDigitalPinMultiple)
{
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_1;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(pin1, 2.5f));


    MAX11300Test::Pin pin2 = MAX11300Test::PIN_18;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(pin2, 2.7f));

    TxRxTransaction txrx_read_gpi1;
    txrx_read_gpi1.description = "GPI read transaction";
    txrx_read_gpi1.tx_buff
        = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
    txrx_read_gpi1.rx_buff = {0x00, 0x00, 0b00000010, 0x00, 0b00000100};
    txrx_read_gpi1.size    = 5;
    txrx_transactions.push_back(txrx_read_gpi1);

    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    EXPECT_TRUE(max11300.ReadDigitalPin(pin1));
    EXPECT_TRUE(max11300.ReadDigitalPin(pin2));
}


TEST_F(Max11300TestHelper, verifyHeterogeneousPinBehavior)
{
    // Pin 1 is a GPI pin
    MAX11300Test::Pin pin1 = MAX11300Test::PIN_11;
    EXPECT_TRUE(ConfigurePinAsDigitalReadAndVerify(pin1, 2.5f));

    // Pin 2 is a GPO pin
    MAX11300Test::Pin pin2 = MAX11300Test::PIN_10;
    EXPECT_TRUE(ConfigurePinAsDigitalWriteAndVerify(pin2, 2.5f));

    // Pin 3 is a DAC pin
    MAX11300Test::Pin pin3 = MAX11300Test::PIN_5;
    EXPECT_TRUE(ConfigurePinAsAnalogWriteAndVerify(
        pin3, MAX11300Test::VoltageRange::ZERO_TO_10));

    // Pin 4 is an ADC pin
    MAX11300Test::Pin pin4 = MAX11300Test::PIN_16;
    EXPECT_TRUE(ConfigurePinAsAnalogReadAndVerify(
        pin4, MAX11300Test::VoltageRange::ZERO_TO_10));

    uint16_t dac_val = 1234;
    max11300.WriteAnalogPinRaw(pin3, dac_val);
    max11300.WriteDigitalPin(pin2, true);


    TxTransaction tx_write_dac1;
    tx_write_dac1.description = "DAC write value transaction";
    tx_write_dac1.buff        = {(uint8_t)((MAX11300_DACDAT_BASE + pin3) << 1),
                          (uint8_t)(dac_val >> 8),
                          (uint8_t)dac_val};
    tx_write_dac1.size        = 3;
    tx_write_dac1.wait        = 0;
    tx_transactions.push_back(tx_write_dac1);

    uint16_t        adc_val = 3583;
    TxRxTransaction txrx_read_adc1;
    txrx_read_adc1.description = "ADC read transaction";
    txrx_read_adc1.tx_buff
        = {(uint8_t)(((MAX11300_ADCDAT_BASE + pin4) << 1) | 1), 0x00, 0x00};
    txrx_read_adc1.rx_buff = {0x00, (uint8_t)(adc_val >> 8), (uint8_t)adc_val};
    txrx_read_adc1.size    = 3;
    txrx_transactions.push_back(txrx_read_adc1);

    TxTransaction tx_write_gpo1;
    tx_write_gpo1.description = "GPO write transaction";
    tx_write_gpo1.buff
        = {(uint8_t)(MAX11300_GPODAT << 1), 0b00000100, 0x00, 0x00, 0x00};
    tx_write_gpo1.size = 5;
    tx_transactions.push_back(tx_write_gpo1);


    TxRxTransaction txrx_read_gpi1;
    txrx_read_gpi1.description = "GPI read transaction";
    txrx_read_gpi1.tx_buff
        = {((MAX11300_GPIDAT << 1) | 1), 0x00, 0x00, 0x00, 0x00};
    txrx_read_gpi1.rx_buff = {0x00, 0b00001000, 0x00, 0x00, 0x00};
    txrx_read_gpi1.size    = 5;
    txrx_transactions.push_back(txrx_read_gpi1);


    EXPECT_TRUE(max11300.Update() == MAX11300Test::Result::OK);

    EXPECT_TRUE(max11300.ReadDigitalPin(pin1));
    EXPECT_EQ(adc_val, max11300.ReadAnalogPinRaw(pin4));
}


TEST(dev_MAX11300, a_VoltsTo12BitUint)
{
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(-1, MAX11300::VoltageRange::ZERO_TO_10), 0);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(0, MAX11300::VoltageRange::ZERO_TO_10),
              0);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(2.5, MAX11300::VoltageRange::ZERO_TO_10),
        1023);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(5, MAX11300::VoltageRange::ZERO_TO_10),
              2047);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(7.5, MAX11300::VoltageRange::ZERO_TO_10),
        3071);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(10, MAX11300::VoltageRange::ZERO_TO_10),
        4095);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(12, MAX11300::VoltageRange::ZERO_TO_10),
        4095);

    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -5.5, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
              0);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(-5, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
        0);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -2.5, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
              1023);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(0, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
        2047);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  2.5, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
              3071);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(5, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
        4095);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(7, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
        4095);

    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -12, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
              0);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -10, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
              0);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -7.5, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
              1023);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -5, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
              2047);
    EXPECT_EQ(MAX11300::VoltsTo12BitUint(
                  -2.5, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
              3071);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(0, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
        4095);
    EXPECT_EQ(
        MAX11300::VoltsTo12BitUint(2, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
        4095);
}

TEST(dev_MAX11300, b_TwelveBitUintToVolts)
{
    float oneLsbAtTenVolts = 10.0f / 4096.0f;

    EXPECT_FLOAT_EQ(
        MAX11300::TwelveBitUintToVolts(0, MAX11300::VoltageRange::ZERO_TO_10),
        0);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    1023, MAX11300::VoltageRange::ZERO_TO_10),
                2.5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    2047, MAX11300::VoltageRange::ZERO_TO_10),
                5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    3071, MAX11300::VoltageRange::ZERO_TO_10),
                7.5,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        4095, MAX11300::VoltageRange::ZERO_TO_10),
                    10);
    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        5000, MAX11300::VoltageRange::ZERO_TO_10),
                    10);

    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        0, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
                    -5);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    1023, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
                -2.5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    2047, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
                0,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    3071, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
                2.5,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        4095, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
                    5);
    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        5000, MAX11300::VoltageRange::NEGATIVE_5_TO_5),
                    5);

    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        0, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
                    -10);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    1023, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
                -7.5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    2047, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
                -5,
                oneLsbAtTenVolts);
    EXPECT_NEAR(MAX11300::TwelveBitUintToVolts(
                    3071, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
                -2.5,
                oneLsbAtTenVolts);
    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        4095, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
                    0);
    EXPECT_FLOAT_EQ(MAX11300::TwelveBitUintToVolts(
                        5000, MAX11300::VoltageRange::NEGATIVE_10_TO_0),
                    0);
}
