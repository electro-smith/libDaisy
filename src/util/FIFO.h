#pragma once

#include <stdint.h>
#include <stddef.h>
#include <initializer_list>

namespace daisy
{
/** A simple FIFO ring buffer with a fixed size. */
template <typename T, size_t capacity>
class FIFO
{
  public:
    FIFO() : bufferIn_(0), bufferOut_(0) {}

    FIFO(const FIFO<T, capacity>& other) { *this = other; }

    FIFO<T, capacity>& operator=(const FIFO<T, capacity>& other)
    {
        bufferIn_ = bufferOut_ = 0;
        if(!other.IsEmpty())
        {
            int readPtr = other.bufferOut_;
            while(readPtr != other.bufferIn_)
            {
                buffer_[bufferIn_++] = other.buffer_[readPtr++];
                if(readPtr >= kBufferSize)
                    readPtr -= kBufferSize;
            }
        }
        return *this;
    }
    ~FIFO() {}

    void Clear() { bufferIn_ = bufferOut_ = 0; }

    /** Adds an element to the back of the buffer, returning true on
        success */
    bool PushBack(const T& elementToAdd)
    {
        if(!IsFull())
        {
            buffer_[bufferIn_++] = elementToAdd;
            if(bufferIn_ >= kBufferSize)
                bufferIn_ -= kBufferSize;
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

    /** returns a reference to the last element */
    T& Back()
    {
        if(IsEmpty())
            // invalid, but better not pass a temporary T() object as a reference...
            return buffer_[0];
        int idx = bufferIn_ - 1;
        if(idx < 0)
            idx += kBufferSize;
        return buffer_[idx];
    }

    /** returns a reference to the last element */
    const T& Back() const
    {
        if(IsEmpty())
            // invalid, but better not pass a temporary T() object as a reference...
            return buffer_[0];
        int idx = bufferIn_ - 1;
        if(idx < 0)
            idx += kBufferSize;
        return buffer_[idx];
    }

    /** removes and returns an element from the front of the buffer */
    T PopFront()
    {
        if(IsEmpty())
            return T();
        else
        {
            const auto result = buffer_[bufferOut_];
            bufferOut_++;
            if(bufferOut_ >= kBufferSize)
                bufferOut_ -= kBufferSize;
            return result;
        }
    }

    /** returns a copy of the first element */
    T& Front()
    {
        if(IsEmpty())
            // invalid, but better not pass a temporary T() object as a reference...
            return buffer_[0];
        return buffer_[bufferOut_];
    }

    /** returns a reference to the first element */
    const T& Front() const
    {
        if(IsEmpty())
            // invalid, but better not pass a temporary T() object as a reference...
            return buffer_[0];
        return buffer_[bufferOut_];
    }

    /** Returns true if the buffer contains an element equal to the provided value */
    bool Contains(const T& element)
    {
        auto idx = bufferOut_;
        while(idx != bufferIn_)
        {
            if(buffer_[idx] == element)
                return true;
            idx++;
            if(idx >= kBufferSize)
                idx -= kBufferSize;
        }
        return false;
    }

    /** Returns the number of elements in the buffer that are equal to the provided value */
    size_t CountEqualTo(const T& element)
    {
        size_t result = 0;
        size_t idx    = bufferOut_;
        while(idx != bufferIn_)
        {
            if(buffer_[idx] == element)
                result++;
            idx++;
            if(idx >= kBufferSize)
                idx -= kBufferSize;
        }
        return result;
    }

    /** returns true, if the buffer is empty */
    bool IsEmpty() const { return bufferIn_ == bufferOut_; }

    /** returns true, if the buffer is Full */
    bool IsFull() const { return GetNumElements() == kBufferSize - 1; }

    /** returns the number of elements in the buffer */
    size_t GetNumElements() const
    {
        int32_t numElements = bufferIn_ - bufferOut_;
        if(numElements < 0)
            numElements += kBufferSize;
        return size_t(numElements);
    }

    /** inserts an element "idx" positions behind the first element and returns true if successful */
    bool Insert(size_t idx, const T& element)
    {
        if(idx > GetNumElements())
            return false;
        if(IsFull())
            return false;
        if(idx == GetNumElements())
        {
            PushBack(element);
            return true;
        }
        // copy last element
        PushBack(Back());
        // move remaining elements: n => n+1
        for(int i = GetNumElements() - 2; i > int(idx); i--)
            (*this)[i] = (*this)[i - 1];
        // insert element
        (*this)[idx] = element;
        return true;
    }

    /** removes the element "idx" positions behind the first element 
     *  and returns true if successful */
    bool Remove(size_t idx)
    {
        if(idx >= GetNumElements())
            return false;

        size_t index = bufferOut_ + idx;
        if(index >= kBufferSize)
            index -= kBufferSize;
        size_t nextIndex = index + 1;
        if(nextIndex >= kBufferSize)
            nextIndex -= kBufferSize;

        while(nextIndex != bufferIn_)
        {
            buffer_[index] = buffer_[nextIndex];
            index++;
            nextIndex++;
            if(index >= kBufferSize)
                index -= kBufferSize;
            if(nextIndex >= kBufferSize)
                nextIndex -= kBufferSize;
        }

        int32_t nextBufferIn = int32_t(bufferIn_) - 1;
        if(nextBufferIn < 0)
            nextBufferIn += kBufferSize;
        bufferIn_ = size_t(nextBufferIn);

        return true;
    }

    /** removes all elements from the buffer for which
        (buffer(index) == element) returns true and returns the number of
        elements that were removed. */
    size_t RemoveAllEqualTo(const T& element)
    {
        size_t numRemoved = 0;
        int    idx        = GetNumElements() - 1;
        while(idx >= 0)
        {
            if((*this)[idx] == element)
            {
                numRemoved++;
                Remove(idx);
                // was that the last element?
                if(idx == int(GetNumElements()) - 1)
                    idx--;
            }
            else
                idx--;
        }
        return numRemoved;
    }

    /** returns the element "idx" positions behind the first element */
    T& operator[](size_t idx)
    {
        if(idx >= GetNumElements()) // TODO: optimize!
        {
            default_ = T();
            return default_;
        }
        size_t index = bufferOut_ + idx;
        if(index >= kBufferSize)
            index -= kBufferSize;
        return buffer_[index];
    }

    /** returns the element "idx" positions behind the first element */
    const T& operator[](size_t idx) const
    {
        if(idx >= GetNumElements()) // TODO: optimize!
        {
            default_ = T();
            return default_;
        }
        size_t index = bufferOut_ + idx;
        if(index >= kBufferSize)
            index -= kBufferSize;
        return buffer_[index];
    }

    /** returns the total capacity */
    uint32_t GetCapacity() const { return kBufferSize - 1; }

  private:
    static constexpr auto kBufferSize = capacity + 1;
    T                     default_;
    T                     buffer_[kBufferSize];
    uint32_t              bufferIn_;
    uint32_t              bufferOut_;
};

} // namespace daisy