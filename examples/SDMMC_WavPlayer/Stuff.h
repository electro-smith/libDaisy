/** This is stuff.
 *  It needs to get moved to libDaisy proper once the example is working
 */
#include "daisy.h"

namespace daisy
{


/** FILE IO API for stream-friendly file access 
 *  This assumes f_mount is called externally
 */
struct FileIO
{
    enum class Status
    {
        OK,
        ERR_RX_QUEUE_FULL,
        ERR_TX_QUEUE_FULL,
        ERR_FILE_ERROR,
        ERR_INVALID_REQUEST,
        ERR_GENERIC,
    };

    struct Request
    {
        FIL*   file;        /**< file pointer */
        void*  data;        /**< Memory location to store read data in */
        size_t data_offset; /**< Optional offset to the start of the dataination */
        size_t size;        /**< size in bytes of the transfer */

        /** Default constructor is invalid request */
        Request() : file(nullptr), data(nullptr), data_offset(0), size(0) {}

        /** Request constructor with required data */
        Request(FIL* fileptr, void* mem, size_t size) : file(fileptr), data(mem), size(size) {}

        /** Request constructor with required data and offset */
        Request(FIL* fileptr, void* mem, size_t size, size_t position)
        : file(fileptr), data(mem), size(size), data_offset(position)
        {
        }

        /** Checks if a request has valid memory, and size */
        inline bool IsValid() const
        {
            return data && size > 0 && data_offset <= size - data_offset;
        }
    };

    /** Adds a new read request to the queue */
    Status PushRx(Request& request)
    {
        if(rx_queue.IsFull())
            return Status::ERR_RX_QUEUE_FULL;
        else if(request.IsValid())
            return rx_queue.PushBack(request) ? Status::OK
                                              : Status::ERR_GENERIC;
        else
            return Status::ERR_INVALID_REQUEST;
    }

    Status Read(Request request)
    {
        FRESULT res      = FR_OK;
        UINT    byte_cnt = 0;
        if(f_read(request.file, request.data, request.size, &byte_cnt) != FR_OK) {
            return Status::ERR_FILE_ERROR;
        }
        return Status::OK;
    }

    Status Write(Request request) {
        FRESULT res      = FR_OK;
        UINT    byte_cnt = 0;
        if(f_write(request.file, request.data, request.size, &byte_cnt) != FR_OK) {
            return Status::ERR_FILE_ERROR;
        }
        return Status::OK;
    }

    /** Process requests, removing an item from the top of each queue, and fulfilling it. */
    Status Process()
    {
        Status sta = Status::OK;
        if(!rx_queue.IsEmpty())
        {
            auto req = rx_queue.PopFront();
            sta      = Read(req);
        }
        if(!tx_queue.IsEmpty())
        {
            auto req = tx_queue.PopFront();
            sta      = Write(req);
        }
    }

    FIFO<Request, 64> rx_queue;
    FIFO<Request, 64> tx_queue;
};

/** WAV file playback class
 *  TODO: move this new wavplayer class into the library
 */
class Player
{
    Player() {}


    /** Having FileIO just be static-accessible would probably be best.. 
     *  i.e. FileIO::PushRx(req), etc.
     */
    void AttachFileIO(FileIO& file_io) {
        io_ = &file_io;
    }

    /** assigns a pool of memory to the player to use to read data from the media */
    void SetMemoryPool(void* mem, size_t size_in_bytes) {}

    void OpenFile(const char* filename) {
         
    }

    void CloseFile() {}

    void JumpToStart() {}

    FileIO* io_;
};

} // namespace daisy