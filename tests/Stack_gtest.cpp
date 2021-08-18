#include <gtest/gtest.h>
#include "util/Stack.h"

using namespace daisy;

class util_Stack : public ::testing::Test
{
protected:
    static constexpr uint32_t stackSize_ = 3;
    Stack<int, stackSize_> stack_;
};
constexpr uint32_t util_Stack::stackSize_; // requried for C++14...

TEST_F(util_Stack, a_getCapacity)
{
    EXPECT_EQ(stack_.GetCapacity(), stackSize_);
}

TEST_F(util_Stack, b_simplePushAndPop)
{
    // empty after initialization
    EXPECT_EQ(stack_.GetNumElements(), 0u);
    EXPECT_TRUE(stack_.IsEmpty());
    EXPECT_FALSE(stack_.IsFull());

    // add single item
    EXPECT_TRUE(stack_.PushBack(1));
    EXPECT_EQ(stack_.GetNumElements(), 1u);
    EXPECT_FALSE(stack_.IsEmpty());
    EXPECT_FALSE(stack_.IsFull());

    // fill
    EXPECT_TRUE(stack_.PushBack(2));
    EXPECT_TRUE(stack_.PushBack(3));
    EXPECT_EQ(stack_.GetNumElements(), 3u);
    EXPECT_FALSE(stack_.IsEmpty());
    EXPECT_TRUE(stack_.IsFull());

    // can't push more
    EXPECT_FALSE(stack_.PushBack(4));

    // values are correct
    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 2);
    EXPECT_EQ(stack_[2], 3);

    // pop items
    EXPECT_EQ(stack_.PopBack(), 3);
    EXPECT_EQ(stack_.PopBack(), 2);
    EXPECT_EQ(stack_.PopBack(), 1);
    EXPECT_TRUE(stack_.IsEmpty());
}

TEST_F(util_Stack, c_clear)
{
    stack_.PushBack(1);
    EXPECT_FALSE(stack_.IsEmpty());

    stack_.Clear();
    EXPECT_TRUE(stack_.IsEmpty());
}

TEST_F(util_Stack, d_insert)
{
    stack_.PushBack(1);
    stack_.PushBack(3);

    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 3);

    // insert an element
    EXPECT_TRUE(stack_.Insert(1, 2));
    // cannot insert a second element, stack's full
    EXPECT_FALSE(stack_.Insert(1, 4));

    // values are correct
    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 2);
    EXPECT_EQ(stack_[2], 3);

    // pop last element
    stack_.PopBack();
    // insert an element at the end
    EXPECT_TRUE(stack_.Insert(2, 3));

    // values are correct
    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 2);
    EXPECT_EQ(stack_[2], 3);
}

TEST_F(util_Stack, e_removeByIndex)
{
    stack_.PushBack(1);
    stack_.PushBack(2);
    stack_.PushBack(3);

    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 2);
    EXPECT_EQ(stack_[2], 3);

    // remove at index 1
    EXPECT_TRUE(stack_.Remove(1));
    // cannot remove at index 2 (only two items left)
    EXPECT_FALSE(stack_.Remove(2));
    
    // values are correct
    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 3);
}

TEST_F(util_Stack, f_removeByValue)
{
    stack_.PushBack(1);
    stack_.PushBack(2);
    stack_.PushBack(3);

    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 2);
    EXPECT_EQ(stack_[2], 3);

    // remove all values "2" => one item removed
    EXPECT_EQ(stack_.RemoveAllEqualTo(2), 1);
    // remove all values "2" => no items removed
    EXPECT_EQ(stack_.RemoveAllEqualTo(2), 0);
    
    // values are correct
    EXPECT_EQ(stack_[0], 1);
    EXPECT_EQ(stack_[1], 3);
}

TEST_F(util_Stack, g_passByReference)
{
    stack_.PushBack(1);
    EXPECT_EQ(stack_[0], 1);
    
    // operator[] passes by reference so that we can modify the values
    // like a plain old C-array.
    stack_[0] = 4;
    EXPECT_EQ(stack_[0], 4);
}

TEST_F(util_Stack, h_contains)
{
    stack_.PushBack({1, 2, 3});
    
    EXPECT_TRUE(stack_.Contains(1));
    EXPECT_TRUE(stack_.Contains(2));
    EXPECT_TRUE(stack_.Contains(3));
    EXPECT_FALSE(stack_.Contains(4));
}

TEST_F(util_Stack, i_countEqualTo)
{
    stack_.PushBack({1, 2, 2});
    
    EXPECT_EQ(stack_.CountEqualTo(1), 1u);
    EXPECT_EQ(stack_.CountEqualTo(2), 2u);
    EXPECT_EQ(stack_.CountEqualTo(3), 0u);
}