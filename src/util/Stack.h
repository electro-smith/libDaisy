#pragma once

#include <stdint.h>
#include <initializer_list>

namespace daisy
{
/** A simple FILO (stack) buffer with a fixed size (usefull when allocation
    on the heap is not an option */
template <typename T, uint32_t kBufferSize>
class Stack
{
  public:
    Stack() : buffer_head_(0) {}

    Stack(std::initializer_list<T> valuesToAdd) : buffer_head_(0)
    {
        PushBack(valuesToAdd);
    }

    Stack(const Stack<T, kBufferSize>& other) { *this = other; }

    Stack<T, kBufferSize>& operator=(const Stack<T, kBufferSize>& other)
    {
        buffer_head_ = 0;
        if(!other.IsEmpty())
        {
            for(int i = 0; i < int(other.GetNumElements()); i++)
                buffer_[i] = other[i];
            buffer_head_ = other.GetNumElements();
        }
        return *this;
    }

    ~Stack() {}

    /** Adds an element to the back of the buffer, returning true on
        success */
    bool PushBack(const T& elementToAdd)
    {
        if(!IsFull())
        {
            buffer_[buffer_head_++] = elementToAdd;
            return true;
        }
        return false;
    }

    /** Adds multiple elements and returns the number of elements that were added */
    int PushBack(std::initializer_list<T> valuesToAdd)
    {
        int numAdded = 0;
        for(const auto& v : valuesToAdd)
        {
            if(IsFull())
                return numAdded;

            PushBack(v);
            numAdded++;
        }
        return numAdded;
    }

    /** removes and returns an element from the back of the buffer */
    T PopBack()
    {
        if(IsEmpty())
            return T();
        else
        {
            return buffer_[--buffer_head_];
        }
    }

    /** clears the buffer */
    void Clear() { buffer_head_ = 0; }

    /** returns an element at the given index without checking for the
        index to be within range. */
    T& operator[](uint32_t idx) { return buffer_[idx]; }
    /** returns an element at the given index without checking for the
        index to be within range. */
    const T& operator[](uint32_t idx) const { return buffer_[idx]; }

    /** removes a single element from the buffer and returns true if successfull */
    bool Remove(uint32_t idx)
    {
        if(idx >= buffer_head_)
            return false;

        for(uint32_t i = idx; i < buffer_head_ - 1; i++)
        {
            buffer_[i] = buffer_[i + 1];
        }
        buffer_head_--;
        return true;
    }

    /** removes all elements from the buffer for which
        (buffer(index) == element) returns true and returns the number of
        elements that were removed. */
    int RemoveAllEqualTo(const T& element)
    {
        int numRemoved = 0;
        int idx        = buffer_head_ - 1;
        while(idx >= 0)
        {
            if(buffer_[idx] == element)
            {
                numRemoved++;
                Remove(idx);
                // was that the last element?
                if(decltype(buffer_head_)(idx) == buffer_head_)
                    idx--;
            }
            else
                idx--;
        }
        return numRemoved;
    }

    /** adds a single element to the buffer and returns true if successfull */
    bool Insert(uint32_t idx, const T& item)
    {
        if(buffer_head_ >= kBufferSize)
            return false;
        if(idx > buffer_head_)
            return false;
        if(idx == buffer_head_)
        {
            buffer_[buffer_head_++] = item;
            return true;
        }

        for(uint32_t i = buffer_head_ - 1; i >= idx; i--)
        {
            buffer_[i + 1] = buffer_[i];
        }
        buffer_[idx] = item;
        buffer_head_++;
        return true;
    }

    /** returns true, if the buffer is empty */
    bool IsEmpty() const { return buffer_head_ == 0; }

    /** returns true, if the buffer is Full */
    bool IsFull() const { return buffer_head_ == kBufferSize; }

    /** returns the number of elements in the buffer */
    uint32_t GetNumElements() const { return buffer_head_; }

    /** returns the total capacity */
    uint32_t GetCapacity() const { return kBufferSize; }

  private:
    T        buffer_[kBufferSize];
    uint32_t buffer_head_;
};

} // namespace daisy