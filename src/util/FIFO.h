#pragma once

#include <stdint.h>

namespace daisy
{
/** A simple FIFO ring buffer with a fixed size. */
template <typename T, int kBufferSize>
class FIFO
{
  public:
    FIFO() : bufferIn_(0), bufferOut_(0) {}

    FIFO(const FIFO<T, kBufferSize>& other) { *this = other; }

    FIFO<T, kBufferSize>& operator=(const FIFO<T, kBufferSize>& other)
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

    void Clear()
    {
        // TODO: overwrite elements with default value T() ?
        bufferIn_ = bufferOut_ = 0;
    }

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

    /** returns a reference to the last element */
    T Back()
    {
        if(IsEmpty())
            return T();
        int idx = bufferIn_ - 1;
        if(idx < 0)
            idx += kBufferSize;
        return buffer_[idx];
    }

    /** returns a reference to the last element */
    const T Back() const
    {
        if(IsEmpty())
            return T();
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
            int32_t indexToReturn = bufferOut_;
            bufferOut_++;
            if(bufferOut_ >= kBufferSize)
                bufferOut_ -= kBufferSize;
            return buffer_[indexToReturn];
        }
    }

    /** returns a copy of the first element */
    T Front()
    {
        if(IsEmpty())
            return T();
        return buffer_[bufferOut_];
    }

    /** returns a reference to the first element */
    const T Front() const
    {
        if(IsEmpty())
            return T();
        return buffer_[bufferOut_];
    }

    /** returns true, if the buffer is empty */
    bool IsEmpty() const { return bufferIn_ == bufferOut_; }

    /** returns true, if the buffer is Full */
    bool IsFull() const { return GetNumElements() == kBufferSize; }

    /** returns the number of elements in the buffer */
    int32_t GetNumElements() const
    {
        int32_t numElements = bufferIn_ - bufferOut_;
        if(numElements < 0)
            numElements += kBufferSize;
        return numElements;
    }

    /** removes the element "idx" positions behind the first element */
    void Remove(int idx)
    {
        if(idx < GetNumElements()) // TODO: optimize!
        {
            int index = bufferOut_ + idx;
            if(index >= kBufferSize)
                index -= kBufferSize;
            int nextIndex = index + 1;
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

            bufferIn_--;
            if(bufferIn_ < 0)
                bufferIn_ += kBufferSize;
        }
    }

    /** returns the element "idx" positions behind the first element */
    T& operator[](int idx)
    {
        if(idx >= GetNumElements()) // TODO: optimize!
        {
            default_ = T();
            return default_;
        }
        int index = bufferOut_ + idx;
        if(index >= kBufferSize)
            index -= kBufferSize;
        return buffer_[index];
    }

    /** returns the element "idx" positions behind the first element */
    const T& operator[](int idx) const
    {
        if(idx >= GetNumElements()) // TODO: optimize!
        {
            default_ = T();
            return default_;
        }
        int index = bufferOut_ + idx;
        if(index >= kBufferSize)
            index -= kBufferSize;
        return buffer_[index];
    }

    /** returns the total capacity */
    uint32_t GetCapacity() const { return kBufferSize; }

  private:
    T        default_;
    T        buffer_[kBufferSize];
    uint32_t bufferIn_;
    uint32_t bufferOut_;
};

} // namespace daisy