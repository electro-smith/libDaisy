#pragma once
#pragma once
#include <cctype>
#include <cstring>

#include "ff.h"

namespace daisy
{
/** Utility class for creating an index of file names
 *  and lengths from a given directory.
 *
 *  Useful for grouping .wav files for playback, etc.
 */
template <size_t max_slots>
class FileTable
{
  public:
    static constexpr const size_t kMaxCustomFileNameLen = _MAX_LFN;
    static constexpr const size_t kMaxFileSlots         = max_slots;

    /** Reset the table to its initial, empty state.  */
    void Clear()
    {
        for(auto &item : table)
        {
            std::fill(item.name, item.name + kMaxCustomFileNameLen, 0x00);
            item.size = 0;
        }
        num_files_found = 0;
    }

    /** Search the path provided, and fill the table with files that match the pattern provided.
     *  The loaded files are sorted alphabetically by filename
     *
     * @param path path to the directory to search.
     *
     * @param endswith string suffix to compare to, often a file extension (i.e. ".wav")
     * if this is null, then all files will be loaded.
     *
     * @return true if files are loaded, otherwise false
     */
    bool Fill(const char *path, const char *endswith = nullptr)
    {
        FRESULT res = FR_OK;
        if(path == nullptr)
        {
            return false;
        }
        FILINFO fno;
        res        = f_opendir(&dir, path);
        size_t cnt = 0;
        if(res == FR_OK)
        {
            for(;;)
            {
                res = f_readdir(&dir, &fno);
                if(fno.fname[0] == 0)
                    break; //< escape w/ no file
                bool valid_file_attrs = !(fno.fattrib & AM_HID)
                                        && !(fno.fattrib & AM_DIR)
                                        && !(fno.fattrib & AM_SYS);

                bool valid_size = fno.fsize > 0;

                bool valid_name = false;
                if(endswith != nullptr)
                {
                    uint32_t suffix_len = strlen(endswith);
                    valid_name          = strstr(fno.fname, endswith) != nullptr
                                 && strlen(fno.fname) > suffix_len
                                 && strlen(fno.fname) < kMaxCustomFileNameLen;
                }
                else
                {
                    valid_name = strlen(fno.fname) < kMaxCustomFileNameLen;
                }

                if(valid_file_attrs && valid_size && valid_name)
                {
                    // Copy this file into the slot of the table, and increment
                    strcpy(table[cnt].name, fno.fname);
                    table[cnt].size = fno.fsize;
                    cnt++;
                }
                if(cnt > kMaxFileSlots - 1)
                    break;
            }
            f_closedir(&dir);
        }
        num_files_found = cnt;
        SortTable();
        return res == FR_OK;
    }

    /** Generates a simple log file, and writes it to the destination
     *
     *  The file will contain a list of all of the files loaded, with their
     *  slot position, and file size.
     */
    bool WriteLog(const char *log_file_name)
    {
        FRESULT res
            = f_open(&file, log_file_name, (FA_CREATE_ALWAYS | FA_WRITE));
        if(res == FR_OK)
        {
            if(num_files_found > 0)
            {
                for(size_t i = 0; i < num_files_found; i++)
                {
                    char line_buff[kMaxCustomFileNameLen + 32];
                    std::fill(line_buff, line_buff + sizeof(line_buff), 0x00);
                    sprintf(line_buff,
                            "%d:\t%s\t%d bytes\n",
                            i + 1,
                            table[i].name,
                            table[i].size);
                    UINT bw = 0;
                    res     = f_write(&file, line_buff, strlen(line_buff), &bw);
                    if(res != FR_OK)
                    {
                        return f_close(&file) == FR_OK;
                    }
                }
            }
            else
            {
                const char *text = "No matching files found...";
                UINT        bw   = 0;
                res              = f_write(&file, text, strlen(text), &bw);
            }
            f_close(&file);
        }
        return res == FR_OK;
    }

    /** Returns whether there is a file present at the index  */
    inline bool IsFileInSlot(size_t idx) const { return table[idx].size > 0; }

    /** Returns the size of the file in the specified slot. */
    inline size_t GetFileSize(size_t idx) const { return table[idx].size; }

    /** Returns the name of the file in the specified slot. */
    inline const char *GetFileName(size_t idx) const { return table[idx].name; }

    /** Returns the number of files found in the table. */
    inline size_t GetNumFiles() const { return num_files_found; }

    /** Flag-y bits for Loading and Saving
     *  This class can act like an interface for the actual I/O.
     *
     *  This has to be manually managed, but can be used to coordinate
     *  loading/storing data.
     */

    inline bool IsLoadPending() const { return load_pending_; }

    inline void ClearLoadPending()
    {
        load_pending_       = false;
        slot_for_load_save_ = -1;
    }
    inline void SetLoadPending(int slot)
    {
        load_pending_       = true;
        slot_for_load_save_ = slot;
    }

    inline bool IsSavePending() const { return save_pending_; }

    inline void ClearSavePending()
    {
        save_pending_       = false;
        slot_for_load_save_ = -1;
    }
    inline void SetSavePending(int slot)
    {
        save_pending_       = true;
        slot_for_load_save_ = slot;
    }

    inline int GetSlotForSaveLoad() const { return slot_for_load_save_; }

  private:
    struct FileInfo
    {
        char   name[kMaxCustomFileNameLen];
        size_t size;
    };

    /** Sorts the table by each FileInfo's name member. */
    void SortTable()
    {
        if(num_files_found < 2)
            return;

        for(size_t i = 1; i < num_files_found; ++i)
        {
            FileInfo key = table[i];
            size_t   j   = i;
            while(j > 0 && CaseInsensitiveCmp(table[j - 1].name, key.name) > 0)
            {
                table[j] = table[j - 1];
                --j;
            }
            table[j] = key;
        }
    }

    FIL      file;
    DIR      dir;
    FileInfo table[kMaxFileSlots];
    size_t   num_files_found;

    // Flags for saving/loading of files (not the best place for these)
    // but this saves adding more back-and-forth between the new UiPage
    // and the actual diskio
    bool load_pending_;
    bool save_pending_;
    int  slot_for_load_save_;

    // Internal helper for sorting files on to a known order.
    static inline int CaseInsensitiveCmp(const char *a, const char *b)
    {
        // ASCII-only case fold adequate for FAT volume typical usage
        unsigned char ca, cb;
        while(*a && *b)
        {
            ca = static_cast<unsigned char>(*a);
            cb = static_cast<unsigned char>(*b);
            ca = static_cast<unsigned char>(std::tolower(ca));
            cb = static_cast<unsigned char>(std::tolower(cb));
            if(ca != cb)
                return (ca < cb) ? -1 : 1;
            ++a;
            ++b;
        }
        if(*a == *b)
            return 0;
        return (*a == '\0') ? -1 : 1;
    }
};

} // namespace daisy