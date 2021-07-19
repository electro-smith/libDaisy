#include <gtest/gtest.h>
#include "util/FIFO.h"

using namespace daisy;

class util_FIFO : public ::testing::Test
{
  protected:
    static constexpr uint32_t fifoSize_ = 3;
    FIFO<int, fifoSize_>      fifo_;
};
constexpr uint32_t util_FIFO::fifoSize_; // requried for C++14...

TEST_F(util_FIFO, a_getCapacity)
{
    EXPECT_EQ(fifo_.GetCapacity(), fifoSize_);
}

TEST_F(util_FIFO, b_simplePushAndPop)
{
    // empty after initialization
    EXPECT_EQ(fifo_.GetNumElements(), 0u);
    EXPECT_TRUE(fifo_.IsEmpty());
    EXPECT_FALSE(fifo_.IsFull());

    // add single item
    EXPECT_TRUE(fifo_.PushBack(1));
    EXPECT_EQ(fifo_.GetNumElements(), 1u);
    EXPECT_FALSE(fifo_.IsEmpty());
    EXPECT_FALSE(fifo_.IsFull());

    // fill
    EXPECT_TRUE(fifo_.PushBack(2));
    EXPECT_TRUE(fifo_.PushBack(3));
    EXPECT_EQ(fifo_.GetNumElements(), 3u);
    EXPECT_FALSE(fifo_.IsEmpty());
    EXPECT_TRUE(fifo_.IsFull());

    // can't push more
    EXPECT_FALSE(fifo_.PushBack(4));

    // values are correct
    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 2);
    EXPECT_EQ(fifo_[2], 3);

    // pop single item
    EXPECT_EQ(fifo_.PopFront(), 1);
    EXPECT_EQ(fifo_.GetNumElements(), 2u);

    // push another one (fifo now wraps around the buffer ends)
    EXPECT_TRUE(fifo_.PushBack(4));

    // values are correct
    EXPECT_EQ(fifo_[0], 2);
    EXPECT_EQ(fifo_[1], 3);
    EXPECT_EQ(fifo_[2], 4);

    // pop items
    EXPECT_EQ(fifo_.PopFront(), 2);
    EXPECT_EQ(fifo_.PopFront(), 3);
    EXPECT_EQ(fifo_.PopFront(), 4);
    EXPECT_TRUE(fifo_.IsEmpty());
}

TEST_F(util_FIFO, c_backAndFront)
{
    fifo_.PushBack({1, 2, 3});
    EXPECT_EQ(fifo_.Front(), 1);
    EXPECT_EQ(fifo_.Back(), 3);
}

TEST_F(util_FIFO, d_clear)
{
    fifo_.PushBack(1);
    EXPECT_FALSE(fifo_.IsEmpty());

    fifo_.Clear();
    EXPECT_TRUE(fifo_.IsEmpty());
}

TEST_F(util_FIFO, e_insert)
{
    fifo_.PushBack(1);
    fifo_.PushBack(3);

    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 3);

    // insert an element
    EXPECT_TRUE(fifo_.Insert(1, 2));
    // cannot insert a second element, fifo's full
    EXPECT_FALSE(fifo_.Insert(1, 4));

    // values are correct
    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 2);
    EXPECT_EQ(fifo_[2], 3);

    // pop last element
    fifo_.PopFront();
    // insert an element at the end
    EXPECT_TRUE(fifo_.Insert(2, 4));

    // values are correct
    EXPECT_EQ(fifo_[0], 2);
    EXPECT_EQ(fifo_[1], 3);
    EXPECT_EQ(fifo_[2], 4);
}

TEST_F(util_FIFO, f_removeByIndex)
{
    fifo_.PushBack(1);
    fifo_.PushBack(2);
    fifo_.PushBack(3);

    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 2);
    EXPECT_EQ(fifo_[2], 3);

    // remove at index 1
    EXPECT_TRUE(fifo_.Remove(1));
    // cannot remove at index 2 (only two items left)
    EXPECT_FALSE(fifo_.Remove(2));

    // values are correct
    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 3);
}

TEST_F(util_FIFO, g_removeByValue)
{
    fifo_.PushBack(1);
    fifo_.PushBack(1);
    fifo_.PushBack(2);

    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 1);
    EXPECT_EQ(fifo_[2], 2);

    // remove all values "1" => two items removed
    EXPECT_EQ(fifo_.RemoveAllEqualTo(1), 2u);
    // remove all values "1" => no items removed
    EXPECT_EQ(fifo_.RemoveAllEqualTo(1), 0u);

    // values are correct
    EXPECT_EQ(fifo_.GetNumElements(), 1u);
    EXPECT_EQ(fifo_[0], 2);
}

TEST_F(util_FIFO, h_passByReference)
{
    fifo_.PushBack({0, 1, 2});
    // make contents wrap around the buffer
    fifo_.PopFront();
    fifo_.PushBack(3);

    EXPECT_EQ(fifo_[0], 1);
    EXPECT_EQ(fifo_[1], 2);
    EXPECT_EQ(fifo_[2], 3);

    // operator[], Back() and Front() pass by reference so that we can
    // modify the values like a plain old C-array.
    fifo_.Front() = 4;
    fifo_[1] = 5;
    fifo_.Back() = 6;
    EXPECT_EQ(fifo_[0], 4);
    EXPECT_EQ(fifo_[1], 5);
    EXPECT_EQ(fifo_[2], 6);
}

TEST_F(util_FIFO, i_contains)
{
    fifo_.PushBack({1, 2, 3});

    EXPECT_TRUE(fifo_.Contains(1));
    EXPECT_TRUE(fifo_.Contains(2));
    EXPECT_TRUE(fifo_.Contains(3));
    EXPECT_FALSE(fifo_.Contains(4));

    // amke fifo contents wrap around the buffers end
    fifo_.PopFront();
    fifo_.PushBack(4);

    EXPECT_FALSE(fifo_.Contains(1));
    EXPECT_TRUE(fifo_.Contains(2));
    EXPECT_TRUE(fifo_.Contains(3));
    EXPECT_TRUE(fifo_.Contains(4));
}

TEST_F(util_FIFO, j_countEqualTo)
{
    fifo_.PushBack({1, 2, 2});

    EXPECT_EQ(fifo_.CountEqualTo(1), 1u);
    EXPECT_EQ(fifo_.CountEqualTo(2), 2u);
    EXPECT_EQ(fifo_.CountEqualTo(3), 0u);

    // amke fifo contents wrap around the buffers end
    fifo_.PopFront();
    fifo_.PushBack(3);

    EXPECT_EQ(fifo_.CountEqualTo(1), 0u);

    EXPECT_EQ(fifo_.CountEqualTo(2), 2u);
    EXPECT_EQ(fifo_.CountEqualTo(3), 1u);
}