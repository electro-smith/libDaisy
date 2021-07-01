#include "hid/disp/graphics_common.h"
#include <gtest/gtest.h>

using namespace daisy;

// pretty printing for googletest
namespace daisy
{
std::ostream& operator<<(std::ostream& os, const Rectangle& r)
{
    return os << "(" << r.GetX() << ", " << r.GetY() << ", " << r.GetWidth()
              << ", " << r.GetHeight() << ")";
}
} // namespace daisy

TEST(hid_disp_Rectangle, a_ctor)
{
    // default ctor
    {
        const auto rect = Rectangle();
        EXPECT_EQ(rect.GetX(), 0);
        EXPECT_EQ(rect.GetY(), 0);
        EXPECT_EQ(rect.GetWidth(), 0);
        EXPECT_EQ(rect.GetHeight(), 0);
    }
    // width/height only ctor
    {
        const auto rect = Rectangle(1, 2);
        EXPECT_EQ(rect.GetX(), 0);
        EXPECT_EQ(rect.GetY(), 0);
        EXPECT_EQ(rect.GetWidth(), 1);
        EXPECT_EQ(rect.GetHeight(), 2);
    }
    // full ctor
    {
        const auto rect = Rectangle(1, 2, 3, 4);
        EXPECT_EQ(rect.GetX(), 1);
        EXPECT_EQ(rect.GetY(), 2);
        EXPECT_EQ(rect.GetWidth(), 3);
        EXPECT_EQ(rect.GetHeight(), 4);
    }
    // clamp width and height
    {
        const auto rect = Rectangle(1, 2, -3, -4);
        EXPECT_EQ(rect.GetX(), 1);
        EXPECT_EQ(rect.GetY(), 2);
        EXPECT_EQ(rect.GetWidth(), 0);
        EXPECT_EQ(rect.GetHeight(), 0);
    }
    // copy ctor
    {
        const auto rect2 = Rectangle(1, 2, 3, 4);
        const auto rect  = Rectangle(rect2);
        EXPECT_EQ(rect.GetX(), 1);
        EXPECT_EQ(rect.GetY(), 2);
        EXPECT_EQ(rect.GetWidth(), 3);
        EXPECT_EQ(rect.GetHeight(), 4);
    }
}

TEST(hid_disp_Rectangle, b_equals)
{
    EXPECT_EQ(Rectangle(1, 2, 3, 4), Rectangle(1, 2, 3, 4));

    EXPECT_NE(Rectangle(1, 2, 3, 4), Rectangle(0, 2, 3, 4));
    EXPECT_NE(Rectangle(1, 2, 3, 4), Rectangle(1, 0, 3, 4));
    EXPECT_NE(Rectangle(1, 2, 3, 4), Rectangle(1, 2, 0, 4));
    EXPECT_NE(Rectangle(1, 2, 3, 4), Rectangle(1, 2, 3, 0));
}

TEST(hid_disp_Rectangle, c_empty)
{
    EXPECT_FALSE(Rectangle(1, 2, 3, 4).IsEmpty());
    EXPECT_TRUE(Rectangle(1, 2, 0, 4).IsEmpty());
    EXPECT_TRUE(Rectangle(1, 2, 3, 0).IsEmpty());
}

TEST(hid_disp_Rectangle, d_getters)
{
    EXPECT_EQ(Rectangle(1, 2, 3, 4).GetRight(), 1 + 3);
    EXPECT_EQ(Rectangle(1, 2, 3, 4).GetBottom(), 2 + 4);

    EXPECT_EQ(Rectangle(1, 2, 3, 4).GetCenterX(), 2); // round down
    EXPECT_EQ(Rectangle(1, 2, 4, 4).GetCenterX(), 3);

    EXPECT_EQ(Rectangle(1, 2, 3, 3).GetCenterY(), 3); // round down
    EXPECT_EQ(Rectangle(1, 2, 4, 4).GetCenterY(), 4);
}

TEST(hid_disp_Rectangle, e_setters)
{
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithX(10), Rectangle(10, 2, 3, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithY(10), Rectangle(1, 10, 3, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithWidth(10), Rectangle(1, 2, 10, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithHeight(10), Rectangle(1, 2, 3, 10));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithSize(10, 20), Rectangle(1, 2, 10, 20));
    EXPECT_EQ(Rectangle(0, 0, 5, 5).WithWidthKeepingCenter(3),
              Rectangle(1, 0, 3, 5));
    EXPECT_EQ(Rectangle(0, 0, 5, 5).WithHeightKeepingCenter(3),
              Rectangle(0, 1, 5, 3));
    EXPECT_EQ(Rectangle(0, 0, 5, 5).WithSizeKeepingCenter(3, 3),
              Rectangle(1, 1, 3, 3));
}

TEST(hid_disp_Rectangle, f_reduced)
{
    EXPECT_EQ(Rectangle(5, 6, 7, 8).Reduced(1), Rectangle(6, 7, 5, 6));
    EXPECT_EQ(Rectangle(5, 6, 7, 8).Reduced(1, 2), Rectangle(6, 8, 5, 4));
    // clamp with and height to zero
    EXPECT_EQ(Rectangle(1, 2, 3, 5).Reduced(2), Rectangle(3, 4, 0, 1));
    EXPECT_EQ(Rectangle(1, 2, 5, 3).Reduced(2), Rectangle(3, 4, 1, 0));
    EXPECT_EQ(Rectangle(1, 2, 2, 3).Reduced(2, 0), Rectangle(3, 2, 0, 3));
    EXPECT_EQ(Rectangle(1, 2, 3, 2).Reduced(0, 2), Rectangle(1, 4, 3, 0));
}

TEST(hid_disp_Rectangle, g_translated)
{
    EXPECT_EQ(Rectangle(1, 2, 3, 4).Translated(10, 20),
              Rectangle(11, 22, 3, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).Translated(-10, -20),
              Rectangle(-9, -18, 3, 4));
}

TEST(hid_disp_Rectangle, h_stetched)
{
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithLeft(-1), Rectangle(-1, 2, 5, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithRight(10), Rectangle(1, 2, 9, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithTop(-2), Rectangle(1, -2, 3, 8));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithBottom(20), Rectangle(1, 2, 3, 18));
}

TEST(hid_disp_Rectangle, i_trimmed)
{
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithTrimmedLeft(2), Rectangle(3, 2, 1, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithTrimmedRight(2), Rectangle(1, 2, 1, 4));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithTrimmedTop(2), Rectangle(1, 4, 3, 2));
    EXPECT_EQ(Rectangle(1, 2, 3, 4).WithTrimmedBottom(2),
              Rectangle(1, 2, 3, 2));
}

TEST(hid_disp_Rectangle, j_withCenter)
{
    EXPECT_EQ(Rectangle(0, 0, 6, 6).WithCenterX(2), Rectangle(-1, 0, 6, 6));
    EXPECT_EQ(Rectangle(0, 0, 6, 6).WithCenterY(2), Rectangle(0, -1, 6, 6));
    EXPECT_EQ(Rectangle(0, 0, 6, 6).WithCenter(2, 4), Rectangle(-1, 1, 6, 6));
}

TEST(hid_disp_Rectangle, k_removeFrom)
{
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromLeft(4), Rectangle(0, 0, 4, 10));
        EXPECT_EQ(rect, Rectangle(4, 0, 6, 10));
    }
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromRight(4), Rectangle(6, 0, 4, 10));
        EXPECT_EQ(rect, Rectangle(0, 0, 6, 10));
    }
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromTop(4), Rectangle(0, 0, 10, 4));
        EXPECT_EQ(rect, Rectangle(0, 4, 10, 6));
    }
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromBottom(4), Rectangle(0, 6, 10, 4));
        EXPECT_EQ(rect, Rectangle(0, 0, 10, 6));
    }
    // now remove too much...
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromLeft(11), Rectangle(0, 0, 10, 10));
        EXPECT_EQ(rect.GetWidth(), 0); // now empty
    }
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromRight(11), Rectangle(0, 0, 10, 10));
        EXPECT_EQ(rect.GetWidth(), 0); // now empty
    }
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromTop(11), Rectangle(0, 0, 10, 10));
        EXPECT_EQ(rect.GetHeight(), 0); // now empty
    }
    {
        auto rect = Rectangle(0, 0, 10, 10);
        EXPECT_EQ(rect.RemoveFromBottom(11), Rectangle(0, 0, 10, 10));
        EXPECT_EQ(rect.GetHeight(), 0); // now empty
    }
}

TEST(hid_disp_Rectangle, k_alignedWithin)
{
    const auto boundingBox = Rectangle(0, 0, 100, 100);
    const auto srcRect     = Rectangle(10, 10, 10, 10);
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::topLeft),
              Rectangle(0, 0, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::topRight),
              Rectangle(90, 0, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::topCentered),
              Rectangle(45, 0, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::bottomLeft),
              Rectangle(0, 90, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::bottomRight),
              Rectangle(90, 90, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::bottomCentered),
              Rectangle(45, 90, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::centeredLeft),
              Rectangle(0, 45, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::centeredRight),
              Rectangle(90, 45, 10, 10));
    EXPECT_EQ(srcRect.AlignedWithin(boundingBox, Alignment::centered),
              Rectangle(45, 45, 10, 10));
}