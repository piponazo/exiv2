/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "basicio.hpp"

namespace Exiv2 {
    /*!
      @brief Provides binary file IO by implementing the BasicIo
          interface.
     */
    class EXIV2API FileIo : public BasicIo {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that accepts the file path on which IO will be
              performed. The constructor does not open the file, and
              therefore never failes.
          @param path The full path of a file
         */
        explicit FileIo(const std::string& path);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like FileIo(const std::string& path) but accepts a
              unicode path in an std::wstring.
          @note This constructor is only available on Windows.
         */
        FileIo(const std::wstring& wpath);
#endif
        //! Destructor. Flushes and closes an open file.
        virtual ~FileIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Open the file using using the specified mode.

          This method can also be used to "reopen" a file which will flush any
          unwritten data and reset the IO position to the start. Although
          files can be opened in binary or text mode, this class has
          only been tested carefully in binary mode.

          @param mode Specified that type of access allowed on the file.
              Valid values match those of the C fopen command exactly.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        int open(const std::string& mode);
        /*!
          @brief Open the file using using the default access mode of "rb".
              This method can also be used to "reopen" a file which will flush
              any unwritten data and reset the IO position to the start.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int open();
        /*!
          @brief Flush and unwritten data and close the file . It is
              safe to call close on an already closed instance.
          @return 0 if successful;<BR>
                 Nonzero if failure;
         */
        virtual int close();
        /*!
          @brief Write data to the file. The file position is advanced
              by the number of bytes written.
          @param data Pointer to data. Data must be at least \em wcount
              bytes long
          @param wcount Number of bytes to be written.
          @return Number of bytes written to the file successfully;<BR>
                 0 if failure;
         */
        virtual long write(const byte* data, long wcount);
        /*!
          @brief Write data that is read from another BasicIo instance to
              the file. The file position is advanced by the number
              of bytes written.
          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return Number of bytes written to the file successfully;<BR>
                 0 if failure;
         */
        virtual long write(BasicIo& src);
        /*!
          @brief Write one byte to the file. The file position is
              advanced by one byte.
          @param data The single byte to be written.
          @return The value of the byte written if successful;<BR>
                 EOF if failure;
         */
        virtual int putb(byte data);
        /*!
          @brief Read data from the file. Reading starts at the current
              file position and the position is advanced by the number of
              bytes read.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return DataBuf instance containing the bytes read. Use the
                DataBuf::size_ member to find the number of bytes read.
                DataBuf::size_ will be 0 on failure.
         */
        virtual DataBuf read(long rcount);
        /*!
          @brief Read data from the file. Reading starts at the current
              file position and the position is advanced by the number of
              bytes read.
          @param buf Pointer to a block of memory into which the read data
              is stored. The memory block must be at least \em rcount bytes
              long.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return Number of bytes read from the file successfully;<BR>
                 0 if failure;
         */
        virtual long read(byte* buf, long rcount);
        /*!
          @brief Read one byte from the file. The file position is
              advanced by one byte.
          @return The byte read from the file if successful;<BR>
                 EOF if failure;
         */
        virtual int getb();
        /*!
          @brief Remove the contents of the file and then transfer data from
              the \em src BasicIo object into the empty file.

          This method is optimized to simply rename the source file if the
          source object is another FileIo instance. The source BasicIo object
          is invalidated by this operation and should not be used after this
          method returns. This method exists primarily to be used with
          the BasicIo::temporary() method.

          @note If the caller doesn't have permissions to write to the file,
              an exception is raised and \em src is deleted.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure
         */
        virtual void transfer(BasicIo& src);
        /*!
          @brief Move the current file position.
          @param offset Number of bytes to move the file position
              relative to the starting position specified by \em pos
          @param pos Position from which the seek should start
          @return 0 if successful;<BR>
                 Nonzero if failure;
         */
#if defined(_MSC_VER)
        virtual int seek(int64_t offset, Position pos);
#else
        virtual int seek(long offset, Position pos);
#endif
        /*!
          @brief Map the file into the process's address space. The file must be
                 open before mmap() is called. If the mapped area is writeable,
                 changes may not be written back to the underlying file until
                 munmap() is called. The pointer is valid only as long as the
                 FileIo object exists.
          @param isWriteable Set to true if the mapped area should be writeable
                 (default is false).
          @return A pointer to the mapped area.
          @throw Error In case of failure.
         */
        virtual byte* mmap(bool isWriteable =false);
        /*!
          @brief Remove a mapping established with mmap(). If the mapped area is
                 writeable, this ensures that changes are written back to the
                 underlying file.
          @return 0 if successful;<BR>
                  Nonzero if failure;
         */
        virtual int munmap();
        /*!
          @brief close the file source and set a new path.
         */
        virtual void setPath(const std::string& path);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like setPath(const std::string& path) but accepts a
              unicode path in an std::wstring.
          @note This method is only available on Windows.
         */
        virtual void setPath(const std::wstring& wpath);
#endif
        //@}
        //! @name Accessors
        //@{
        /*!
          @brief Get the current file position.
          @return Offset from the start of the file if successful;<BR>
                 -1 if failure;
         */
        virtual long tell() const;
        /*!
          @brief Flush any buffered writes and get the current file size
              in bytes.
          @return Size of the file in bytes;<BR>
                 -1 if failure;
         */
        virtual size_t size() const;
        //! Returns true if the file is open, otherwise false.
        virtual bool isopen() const;
        //! Returns 0 if the file is in a valid state, otherwise nonzero.
        virtual int error() const;
        //! Returns true if the file position has reached the end, otherwise false.
        virtual bool eof() const;
        //! Returns the path of the file
        virtual std::string path() const;
#ifdef EXV_UNICODE_PATH
        /*
          @brief Like path() but returns the unicode path of the file in an std::wstring.
          @note This function is only available on Windows.
         */
        virtual std::wstring wpath() const;
#endif

        /*!
          @brief Mark all the bNone blocks to bKnow. This avoids allocating memory
            for parts of the file that contain image-date (non-metadata/pixel data)

          @note This method should be only called after the concerned data (metadata)
                are all downloaded from the remote file to memory.
         */
        virtual void populateFakeData();
        //@}

    private:
        // NOT IMPLEMENTED
        //! Copy constructor
        FileIo(FileIo& rhs);
        //! Assignment operator
        FileIo& operator=(const FileIo& rhs);

        // Pimpl idiom
        class Impl;
        std::auto_ptr<Impl> p_;

    }; // class FileIo

    /*!
      @brief Read file \em path into a DataBuf, which is returned.
      @return Buffer containing the file.
      @throw Error In case of failure.
     */
    EXIV2API DataBuf readFile(const std::string& path);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like readFile() but accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API DataBuf readFile(const std::wstring& wpath);
#endif

    /*!
      @brief Write DataBuf \em buf to file \em path.
      @return Return the number of bytes written.
      @throw Error In case of failure.
     */
    EXIV2API long writeFile(const DataBuf& buf, const std::string& path);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like writeFile() but accepts a unicode path in an std::wstring.
      @note This function is only available on Windows.
     */
    EXIV2API long writeFile(const DataBuf& buf, const std::wstring& wpath);
#endif

}