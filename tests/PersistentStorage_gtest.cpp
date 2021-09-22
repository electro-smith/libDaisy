#include "util/PersistentStorage.h"
#include <gtest/gtest.h>

using namespace daisy;

struct StorageTestData
{
    StorageTestData() : a(0xdeadbeef) {}

    uint32_t a;

    bool operator==(const StorageTestData &rhs) { return a == rhs.a; }
    bool operator!=(const StorageTestData &rhs) { return !operator==(rhs); }
};

using StorageTestClass = PersistentStorage<StorageTestData>;

TEST(util_PersistentStorage, a_stateAfterInitClean)
{
    QSPIHandle       qspi;
    StorageTestClass storage(qspi);
    StorageTestData  defaults;

    // Set default data and initialize
    // defaults from obj. constructor
    storage.Init(defaults);

    auto state = storage.GetState();
    auto val   = storage.GetSettings().a;
    EXPECT_EQ(state, StorageTestClass::State::FACTORY);
    EXPECT_EQ(val, static_cast<uint32_t>(0xdeadbeef));
}

TEST(util_PersistentStorage, a_stateAfterWrite)
{
    QSPIHandle       qspi;
    StorageTestClass storage(qspi);
    StorageTestData  defaults;

    // Set default data and initialize
    // defaults from obj. constructor
    storage.Init(defaults);

    //Write a new value and save
    auto &data = storage.GetSettings();
    data.a     = 0;
    storage.Save();

    auto state = storage.GetState();
    auto val   = storage.GetSettings().a;
    EXPECT_EQ(state, StorageTestClass::State::USER);
    EXPECT_EQ(val, (uint32_t)0);
}

TEST(util_PersistentStorage, c_recallData)
{
    QSPIHandle       qspi;
    StorageTestClass storage(qspi);
    StorageTestData  defaults;

    // Initialize defaults (sets to FACTORY)
    storage.Init(defaults);

    //Write a new value and save (sets to USER)
    auto &data = storage.GetSettings();
    data.a     = 0;
    storage.Save();

    // Using a new instance to prevent any volatile bits from sticking through.
    // it should still grab the expected memory pool
    StorageTestClass newStorage(qspi);
    newStorage.Init(defaults);

    auto     savedState = newStorage.GetState();
    uint32_t val        = newStorage.GetSettings().a;
    EXPECT_EQ(savedState, StorageTestClass::State::USER);
    EXPECT_EQ(val, (uint32_t)0);
}

TEST(util_PersistentStorage, d_stateBeforeInitialize)
{
    QSPIHandle qspi;
    StorageTestClass storage(qspi);
    auto state = storage.GetState();
    EXPECT_EQ(state, StorageTestClass::State::UNKNOWN);
}

// A few short tests for the QSPIHandle mock wrapper as well.
// These can move to their own file

TEST(per_QSPIHandle_mock, a_stateAfterInit)
{
    QSPIHandle qspi;
    // Pointer needs to be valid for safe
    // casting to other types, even if
    // the memory has not been written to.
    uint8_t *data = reinterpret_cast<uint8_t*>(qspi.GetData());
    EXPECT_NE(data , nullptr);
}

TEST(per_QSPIHandle_mock, b_testErasure)
{
    QSPIHandle qspi;
    uint32_t testsize = 1024;
    uint32_t testoffset = 256;
    // Like on the hardware, the mock flash
    // erases to 0xff. This helps reduce "gotchas"
    // when moving to hardware.
    // Erase offset to test for unerased section
    qspi.Erase(testoffset, testoffset + testsize);
    uint32_t datasize = qspi.GetCurrentSize();
    EXPECT_EQ(datasize, testsize + testoffset);
    // Get the data from the first address
    uint8_t *data = reinterpret_cast<uint8_t*>(qspi.GetData());
    // Check beginning is not erased yet (likely 0, but probably undefined)
    EXPECT_NE(data[0], 0xff);
    // Check the first byte after the beginning of the erase
    EXPECT_EQ(data[testoffset + 1], 0xff);
    // Check that everything got resized accounting for the offset
    EXPECT_EQ(data[testsize+testoffset - 1], 0xff);
}

TEST(per_QSPIHandle_mock, c_testWrite)
{
    uint32_t testsize = 1024;
    uint32_t testoffset = 256;
    uint8_t testdata[testsize];
    QSPIHandle qspi;
    // Fill testdata with sequential chars
    for (uint32_t i = 0; i < testsize; i++)
        testdata[i] = i;

    // Erase entire planned memory
    qspi.Erase(0, testoffset + testsize);
    qspi.Write(testoffset, testsize, testdata);

    // Get Data from beginning
    uint8_t *data = reinterpret_cast<uint8_t*>(qspi.GetData());
    // Beginning should be erased
    EXPECT_EQ(data[0], 0xff);
    // And then the copied data should match
    uint32_t val = 0;
    uint8_t test = data[testoffset+val];
    EXPECT_EQ(test, val & 0xff);
    val = 9;
    test = data[testoffset+val];
    EXPECT_EQ(test, val & 0xff);
    val = 127;
    test = data[testoffset+val];
    EXPECT_EQ(test, val & 0xff);
    val = testsize / 2;
    test = data[testoffset+val];
    EXPECT_EQ(test, val & 0xff);
}