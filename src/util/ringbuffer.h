#pragma once
#ifndef DSY_RINGBUFFER_H
#define DSY_RINGBUFFER_H

#include <algorithm>

namespace daisy
{
/** @addtogroup utility
    @{
*/

/**
Utility Ring Buffer \n 
imported from pichenettes/stmlib
*/
template <typename T, size_t size>
class RingBuffer
{
  public:
    RingBuffer() {}

    /** Initializes the Ring Buffer */
    inline void Init() { read_ptr_ = write_ptr_ = 0; }

    /** \return The total size of the ring buffer */
    inline size_t capacity() const { return size; }

    /** \return the number of samples that can be written to ring buffer without overwriting unread data. */
    inline size_t writable() const
    {
        return (read_ptr_ - write_ptr_ - 1) % size;
    }

    /** \return number of unread elements in ring buffer */
    inline size_t readable() const { return (write_ptr_ - read_ptr_) % size; }

    /** \returns True, if the buffer is empty. */
    inline bool isEmpty() const { return write_ptr_ == read_ptr_; }

    /** Writes the value to the next available position in the ring buffer
    \param v Value to write
    */
    inline void Write(T v)
    {
        while(!writable())
            ;
        Overwrite(v);
    }

    /** Writes the new element to the ring buffer, overwriting unread data if necessary. 
    \param v Value to overwrite
     */
    inline void Overwrite(T v)
    {
        size_t w   = write_ptr_;
        buffer_[w] = v;
        write_ptr_ = (w + 1) % size;
    }

    /** Reads the first available element from the ring buffer
    \return read value
     */
    inline T Read()
    {
        while(!readable())
            ;
        return ImmediateRead();
    }

    /** Reads next element from ring buffer immediately
    \return read value
     */
    inline T ImmediateRead()
    {
        size_t r      = read_ptr_;
        T      result = buffer_[r];
        read_ptr_     = (r + 1) % size;
        return result;
    }

    /** Flushes unread elements from the ring buffer */
    inline void Flush() { write_ptr_ = read_ptr_; }

    /** Read enough samples to make it possible to read 1 sample. 
    \param n Size of T ?
     */
    inline void Swallow(size_t n)
    {
        if(writable() >= n)
        {
            return;
        }
        read_ptr_ = (write_ptr_ + 1 + n) % size;
    }

    /** Reads a number of elements into a buffer immediately
    \param destination buffer to write to
    \param num_elements number of elements in buffer    
     */
    inline void ImmediateRead(T* destination, size_t num_elements)
    {
        size_t r    = read_ptr_;
        size_t read = num_elements;

        if(r + read > size)
        {
            read = size - r;
        }
        std::copy(&buffer_[r], &buffer_[r + read], destination);
        if(read != num_elements)
        {
            std::copy(
                &buffer_[0], &buffer_[num_elements - read], destination + read);
        }
        read_ptr_ = (r + num_elements) % size;
    }

    /** Overwrites a number of elements using the source buffer as input. 
    \param source Input buffer
    \param num_elements Number of elements in source
     */
    inline void Overwrite(const T* source, size_t num_elements)
    {
        size_t w       = write_ptr_;
        size_t written = num_elements;

        if(w + written > size)
        {
            written = size - w;
        }
        std::copy(source, source + written, &buffer_[w]);
        if(written != num_elements)
        {
            std::copy(source + written, source + num_elements, &buffer_[0]);
        }

        write_ptr_ = (w + num_elements) % size;
    }

    /**Advances the write pointer, for when a peripheral is writing to the buffer. */
    inline void Advance(size_t num_elements)
    {
        size_t free;
        free         = this->writable();
        num_elements = num_elements < free ? num_elements : free;
        write_ptr_   = write_ptr_ + num_elements;
        if(write_ptr_ > size)
            write_ptr_ = write_ptr_ - size;
    }

    /**Returns a pointer to the actual Ring Buffer
     * Useful for when a peripheral needs direct access to the buffer. */
    inline T* GetMutableBuffer() { return buffer_; }

  private:
    T               buffer_[size];
    volatile size_t read_ptr_;
    volatile size_t write_ptr_;
};

/** Utility Ring Buffer
imported from pichenettes/stmlib
*/
template <typename T>
class RingBuffer<T, 0>
{
  public:
    RingBuffer() {}

    inline void   Init() {}                      /**< Initialize ringbuffer */
    inline size_t capacity() const { return 0; } /**< \return 0 */
    inline size_t writable() const { return 0; } /**< \return 0 */
    inline size_t readable() const { return 0; } /**< \return 0 */
    inline void   Write(T v) { (void)(v); } /**<  \param v Value to write */
    inline void   Overwrite(T v)
    {
        (void)(v);
    }                                   /**< \param v Value to overwrite */
    inline T    Read() { return T(0); } /**< \return Read value */
    inline T    ImmediateRead() { return T(0); } /**< \return Read value */
    inline void Flush() {}                       /**< Flush the buffer */
    inline void ImmediateRead(T* destination, size_t num_elements)
    {
        (void)(destination);
        (void)(num_elements);
    } /**< \param destination & \param num_elements & */
    inline void Overwrite(const T* source, size_t num_elements)
    {
        (void)(source);
        (void)(num_elements);
    } /**< \param source 3 \param num_elements & */

  private:
};

/** @} */
} // namespace daisy

#endif
