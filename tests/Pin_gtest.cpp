#include "daisy_core.h"
#include <gtest/gtest.h>

using namespace daisy;

TEST(PinTests, a_comparison)
{
    EXPECT_EQ(Pin(PORTA, 0), Pin(PORTA, 0)); /**< Same Port/Pin */
    EXPECT_NE(Pin(PORTA, 0), Pin(PORTA, 1)); /**< Different Pin */
    EXPECT_NE(Pin(PORTA, 0), Pin(PORTB, 0)); /**< Different Port */
}

TEST(PinTests, b_validity)
{
    Pin invalid_pin;
    EXPECT_FALSE(invalid_pin.IsValid());

    Pin valid_pin = Pin(PORTA, 0);
    EXPECT_TRUE(valid_pin.IsValid());
}

TEST(PinTests, c_copy)
{
    Pin src  = Pin(PORTA, 0);
    Pin dest = src;
    EXPECT_TRUE(src == dest);
}
