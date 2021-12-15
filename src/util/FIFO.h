#pragma once

#include <stdint.h>
#include <stddef.h>
#include <initializer_list>

namespace daisy
{
/** Capacity-independent base class for FIFO. Use FIFO instead. */
template <typename T>
class FIFOBase
{
  protected:
    FIFOBase(T* buffer, size_t bufferSize)
    : buffer_(buffer), bufferSize_(bufferSize), bufferIn_(0), bufferOut_(0)
    {
    }

    FIFOBase(T* buffer, size_t bufferSize, std::initializer_list<T> valuesToAdd)
    : buffer_(buffer), bufferSize_(bufferSize), bufferIn_(0), bufferOut_(0)
    {
        PushBack(valuesToAdd);
    }

  public:
    /** Copies all elements from another FIFO */
    FIFOBase<T>& operator=(const FIFOBase<T>& other)
    {
        bufferIn_ = bufferOut_ = 0;
        if(!other.IsEmpty())
        {
            int readPtr = other.bufferOut_;
            while((readPtr != other.bufferIn_) && (bufferIn_ < bufferSize_))
            {
                buffer_[bufferIn_++] = other.buffer_[readPtr++];
                if(readPtr >= other.bufferSize_)
                    readPtr -= other.bufferSize_;
            }
        }
        return *this;
    }
    ~FIFOBase() {}

    /** Removes all elements from the FIFO */
    void Clear() { bufferIn_ = bufferOut_ = 0; }

    /** Adds an element to the back of the buffer, returning true on
        success */
    bool PushBack(const T& elementToAdd)
    {
        if(!IsFull())
        {
            buffer_[bufferIn_++] = elementToAdd;
            if(bufferIn_ >= bufferSize_)
                bufferIn_ -= bufferSize_;
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
            idx += bufferSize_;
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
            idx += bufferSize_;
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
            if(bufferOut_ >= bufferSize_)
                bufferOut_ -= bufferSize_;
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
            if(idx >= bufferSize_)
                idx -= bufferSize_;
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
            if(idx >= bufferSize_)
                idx -= bufferSize_;
        }
        return result;
    }

    /** returns true, if the buffer is empty */
    bool IsEmpty() const { return bufferIn_ == bufferOut_; }

    /** returns true, if the buffer is Full */
    bool IsFull() const { return GetNumElements() == bufferSize_ - 1; }

    /** returns the number of elements in the buffer */
    size_t GetNumElements() const
    {
        int32_t numElements = bufferIn_ - bufferOut_;
        if(numElements < 0)
            numElements += bufferSize_;
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
        if(index >= bufferSize_)
            index -= bufferSize_;
        size_t nextIndex = index + 1;
        if(nextIndex >= bufferSize_)
            nextIndex -= bufferSize_;

        while(nextIndex != bufferIn_)
        {
            buffer_[index] = buffer_[nextIndex];
            index++;
            nextIndex++;
            if(index >= bufferSize_)
                index -= bufferSize_;
            if(nextIndex >= bufferSize_)
                nextIndex -= bufferSize_;
        }

        int32_t nextBufferIn = int32_t(bufferIn_) - 1;
        if(nextBufferIn < 0)
            nextBufferIn += bufferSize_;
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
        if(idx >= GetNumElements())
            // invalid, but better not pass a temporary T() object as a reference...
            return buffer_[0];

        size_t index = bufferOut_ + idx;
        if(index >= bufferSize_)
            index -= bufferSize_;
        return buffer_[index];
    }

    /** returns the element "idx" positions behind the first element */
    const T& operator[](size_t idx) const
    {
        if(idx >= GetNumElements())
            // invalid, but better not pass a temporary T() object as a reference...
            return buffer_[0];

        size_t index = bufferOut_ + idx;
        if(index >= bufferSize_)
            index -= bufferSize_;
        return buffer_[index];
    }

    /** returns the total capacity */
    size_t GetCapacity() const { return bufferSize_ - 1; }

  private:
    FIFOBase(const FIFOBase<T>&) {} // non copyable

  private:
    T*           buffer_;
    const size_t bufferSize_;
    size_t       bufferIn_;
    size_t       bufferOut_;
};

/** A simple FIFO ring buffer with a fixed size. */
template <typename T, size_t capacity>
class FIFO : public FIFOBase<T>
{
  public:
    /** Creates an empty FIFO */
    FIFO() : FIFOBase<T>(buffer_, capacity + 1) {}

    /** Creates a FIFO and adds a list of values*/
    explicit FIFO(std::initializer_list<T> valuesToAdd)
    : FIFOBase<T>(buffer_, capacity, valuesToAdd)
    {
    }

    /** Creates a FIFO and copies all values from another FIFO */
    template <size_t otherCapacity>
    FIFO(const FIFO<T, otherCapacity>& other)
    {
        *this = other;
    }

    /** Copies all values from another FIFO */
    template <size_t otherCapacity>
    FIFO<T, capacity>& operator=(const FIFO<T, otherCapacity>& other)
    {
        FIFOBase<T>::operator=(other);
        return *this;
    }

  private:
    T buffer_[capacity + 1];
};

} // namespace daisy