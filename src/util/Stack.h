#pragma once

#include <stdint.h>
#include <stddef.h>
#include <initializer_list>

namespace daisy
{
/** Capacity-independent base class for Stack. Use Stack instead. */
template <typename T>
class StackBase
{
  protected:
    StackBase(T* buffer, size_t bufferSize)
    : buffer_(buffer), bufferSize_(bufferSize), bufferHead_(0)
    {
    }

    StackBase(T*                       buffer,
              size_t                   bufferSize,
              std::initializer_list<T> valuesToAdd)
    : buffer_(buffer), bufferSize_(bufferSize), bufferHead_(0)
    {
        PushBack(valuesToAdd);
    }

  public:
    /** Copies all elements from another Stack */
    StackBase<T>& operator=(const StackBase<T>& other)
    {
        bufferHead_ = 0;
        if(!other.IsEmpty())
        {
            const auto numCopy = (other.GetNumElements() < bufferSize_)
                                     ? other.GetNumElements()
                                     : bufferSize_;
            for(size_t i = 0; i < numCopy; i++)
                buffer_[i] = other[i];
            bufferHead_ = other.GetNumElements();
        }
        return *this;
    }

    ~StackBase() {}

    /** Adds an element to the back of the buffer, returning true on
        success */
    bool PushBack(const T& elementToAdd)
    {
        if(!IsFull())
        {
            buffer_[bufferHead_++] = elementToAdd;
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
            return buffer_[--bufferHead_];
        }
    }

    /** clears the buffer */
    void Clear() { bufferHead_ = 0; }

    /** returns an element at the given index without checking for the
        index to be within range. */
    T& operator[](uint32_t idx) { return buffer_[idx]; }
    /** returns an element at the given index without checking for the
        index to be within range. */
    const T& operator[](uint32_t idx) const { return buffer_[idx]; }

    /** removes a single element from the buffer and returns true if successfull */
    bool Remove(uint32_t idx)
    {
        if(idx >= bufferHead_)
            return false;

        for(uint32_t i = idx; i < bufferHead_ - 1; i++)
        {
            buffer_[i] = buffer_[i + 1];
        }
        bufferHead_--;
        return true;
    }

    /** removes all elements from the buffer for which
        (buffer(index) == element) returns true and returns the number of
        elements that were removed. */
    int RemoveAllEqualTo(const T& element)
    {
        int numRemoved = 0;
        int idx        = bufferHead_ - 1;
        while(idx >= 0)
        {
            if(buffer_[idx] == element)
            {
                numRemoved++;
                Remove(idx);
                // was that the last element?
                if(decltype(bufferHead_)(idx) == bufferHead_)
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
        if(bufferHead_ >= bufferSize_)
            return false;
        if(idx > bufferHead_)
            return false;
        if(idx == bufferHead_)
        {
            buffer_[bufferHead_++] = item;
            return true;
        }

        for(uint32_t i = bufferHead_ - 1; i >= idx; i--)
        {
            buffer_[i + 1] = buffer_[i];
        }
        buffer_[idx] = item;
        bufferHead_++;
        return true;
    }

    /** Returns true if the buffer contains an element equal to the provided value */
    bool Contains(const T& element)
    {
        int idx = bufferHead_ - 1;
        while(idx >= 0)
        {
            if(buffer_[idx] == element)
                return true;
            idx--;
        }
        return false;
    }

    /** Returns the number of elements in the buffer that are equal to the provided value */
    size_t CountEqualTo(const T& element)
    {
        size_t result = 0;
        int    idx    = bufferHead_ - 1;
        while(idx >= 0)
        {
            if(buffer_[idx] == element)
                result++;
            idx--;
        }
        return result;
    }

    /** returns true, if the buffer is empty */
    bool IsEmpty() const { return bufferHead_ == 0; }

    /** returns true, if the buffer is Full */
    bool IsFull() const { return bufferHead_ == bufferSize_; }

    /** returns the number of elements in the buffer */
    size_t GetNumElements() const { return bufferHead_; }

    /** returns the total capacity */
    size_t GetCapacity() const { return bufferSize_; }

  private:
    T*           buffer_;
    const size_t bufferSize_;
    size_t       bufferHead_;
};

/** A simple FILO (stack) buffer with a fixed size (usefull when allocation
    on the heap is not an option */
template <typename T, size_t capacity>
class Stack : public StackBase<T>
{
  public:
    /** Creates an empty Stack */
    Stack() : StackBase<T>(buffer_, capacity) {}

    /** Creates a Stack and adds a list of values*/
    explicit Stack(std::initializer_list<T> valuesToAdd)
    : StackBase<T>(buffer_, capacity, valuesToAdd)
    {
    }

    /** Creates a Stack and copies all values from another Stack */
    template <size_t otherCapacity>
    Stack(const Stack<T, otherCapacity>& other)
    : StackBase<T>(buffer_, capacity)
    {
        *this = other;
    }

    /** Copies all values from another Stack */
    template <size_t otherCapacity>
    Stack<T, capacity>& operator=(const Stack<T, otherCapacity>& other)
    {
        StackBase<T>::operator=(other);
        return *this;
    }

  private:
    T buffer_[capacity];
};

} // namespace daisy