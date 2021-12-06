#include "daisy_core.h"
#include <gtest/gtest.h>

using namespace daisy;

TEST(PinTests, a_comparison)
{
    EXPECT_EQ(Pin(PA, 0), Pin(PA, 0)); /**< Same Port/Pin */
    EXPECT_NE(Pin(PA, 0), Pin(PA, 1)); /**< Different Pin */
    EXPECT_NE(Pin(PA, 0), Pin(PB, 0)); /**< Different Port */
}

TEST(PinTests, b_validity)
{
    Pin invalid_pin;
    EXPECT_FALSE(invalid_pin.IsValid());

    Pin valid_pin = Pin(PA, 0);
    EXPECT_TRUE(valid_pin.IsValid());
}

TEST(PinTests, c_copy)
{
    Pin src  = Pin(PA, 0);
    Pin dest = src;
    EXPECT_TRUE(src == dest);
}

TEST(PinTests, d_conversion)
{
	dsy_gpio_pin old_pin = {DSY_GPIOA, 5};

	Pin new_pin = Pin(PA, 5);

	/** Using operator dsy_gpio_pin for implicit conversion */
	dsy_gpio_pin converted_pin = new_pin;

	EXPECT_EQ(old_pin.port, converted_pin.port);
	EXPECT_EQ(old_pin.pin, converted_pin.pin);
}
