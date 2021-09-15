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

    // Resets the emulated memory pool
    qspi.ResetAndClear();

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

    // Resets the emulated memory pool
    qspi.ResetAndClear();

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

    // Resets the emulated memory pool
    qspi.ResetAndClear();

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