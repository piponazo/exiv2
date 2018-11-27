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
#include "FileIo.hpp"

namespace Exiv2 {

    /*!
      @brief Provides binary IO on blocks of memory by implementing the BasicIo
          interface. A copy-on-write implementation ensures that the data passed
          in is only copied when necessary, i.e., as soon as data is written to
          the MemIo. The original data is only used for reading. If writes are
          performed, the changed data can be retrieved using the read methods
          (since the data used in construction is never modified).

      @note If read only usage of this class is common, it might be worth
          creating a specialized readonly class or changing this one to
          have a readonly mode.
     */
    class EXIV2API MemIo : public BasicIo {
    public:
        //! @name Creators
        //@{
        //! Default constructor that results in an empty object
        MemIo();
        /*!
          @brief Constructor that accepts a block of memory. A copy-on-write
              algorithm allows read operations directly from the original data
              and will create a copy of the buffer on the first write operation.
          @param data Pointer to data. Data must be at least \em size
              bytes long
          @param size Number of bytes to copy.
         */
        MemIo(const byte* data, long size);
        //! Destructor. Releases all managed memory
        virtual ~MemIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Memory IO is always open for reading and writing. This method
                 therefore only resets the IO position to the start.

          @return 0
         */
        virtual int open();
        /*!
          @brief Does nothing on MemIo objects.
          @return 0
         */
        virtual int close();
        /*!
          @brief Write data to the memory block. If needed, the size of the
              internal memory block is expanded. The IO position is advanced
              by the number of bytes written.
          @param data Pointer to data. Data must be at least \em wcount
              bytes long
          @param wcount Number of bytes to be written.
          @return Number of bytes written to the memory block successfully;<BR>
                 0 if failure;
         */
        virtual long write(const byte* data, long wcount);
        /*!
          @brief Write data that is read from another BasicIo instance to
              the memory block. If needed, the size of the internal memory
              block is expanded. The IO position is advanced by the number
              of bytes written.
          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return Number of bytes written to the memory block successfully;<BR>
                 0 if failure;
         */
        virtual long write(BasicIo& src);
        /*!
          @brief Write one byte to the memory block. The IO position is
              advanced by one byte.
          @param data The single byte to be written.
          @return The value of the byte written if successful;<BR>
                 EOF if failure;
         */
        virtual int putb(byte data);
        /*!
          @brief Read data from the memory block. Reading starts at the current
              IO position and the position is advanced by the number of
              bytes read.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return DataBuf instance containing the bytes read. Use the
                DataBuf::size_ member to find the number of bytes read.
                DataBuf::size_ will be 0 on failure.
         */
        virtual DataBuf read(long rcount);
        /*!
          @brief Read data from the memory block. Reading starts at the current
              IO position and the position is advanced by the number of
              bytes read.
          @param buf Pointer to a block of memory into which the read data
              is stored. The memory block must be at least \em rcount bytes
              long.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return Number of bytes read from the memory block successfully;<BR>
                 0 if failure;
         */
        virtual long read(byte* buf, long rcount);
        /*!
          @brief Read one byte from the memory block. The IO position is
              advanced by one byte.
          @return The byte read from the memory block if successful;<BR>
                 EOF if failure;
         */
        virtual int getb();
        /*!
          @brief Clear the memory block and then transfer data from
              the \em src BasicIo object into a new block of memory.

          This method is optimized to simply swap memory block if the source
          object is another MemIo instance. The source BasicIo instance
          is invalidated by this operation and should not be used after this
          method returns. This method exists primarily to be used with
          the BasicIo::temporary() method.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure
         */
        virtual void transfer(BasicIo& src);
        /*!
          @brief Move the current IO position.
          @param offset Number of bytes to move the IO position
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
          @brief Allow direct access to the underlying data buffer. The buffer
                 is not protected against write access in any way, the argument
                 is ignored.
          @note  The application must ensure that the memory pointed to by the
                 returned pointer remains valid and allocated as long as the
                 MemIo object exists.
         */
        virtual byte* mmap(bool /*isWriteable*/ =false);
        virtual int munmap();
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Get the current IO position.
          @return Offset from the start of the memory block
         */
        virtual long tell() const;
        /*!
          @brief Get the current memory buffer size in bytes.
          @return Size of the in memory data in bytes;<BR>
                 -1 if failure;
         */
        virtual size_t size() const;
        //!Always returns true
        virtual bool isopen() const;
        //!Always returns 0
        virtual int error() const;
        //!Returns true if the IO position has reached the end, otherwise false.
        virtual bool eof() const;
        //! Returns a dummy path, indicating that memory access is used
        virtual std::string path() const;
#ifdef EXV_UNICODE_PATH
        /*
          @brief Like path() but returns a unicode dummy path in an std::wstring.
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
        MemIo(MemIo& rhs);
        //! Assignment operator
        MemIo& operator=(const MemIo& rhs);

        // Pimpl idiom
        class Impl;
        std::auto_ptr<Impl> p_;

    }; // class MemIo

// The way to handle data from stdin or data uri path. If EXV_XPATH_MEMIO = 1,
// it uses MemIo. Otherwises, it uses FileIo.
#ifndef EXV_XPATH_MEMIO
#define EXV_XPATH_MEMIO 0
#endif

    /*!
      @brief Provides binary IO for the data from stdin and data uri path.
     */
#if EXV_XPATH_MEMIO
    class EXIV2API XPathIo : public MemIo {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        XPathIo(const std::string& path);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like XPathIo(const std::string& path) but accepts a
              unicode url in an std::wstring.
          @note This constructor is only available on Windows.
         */
        XPathIo(const std::wstring& wpath);
#endif
        //@}
    private:
        /*!
            @brief Read data from stdin and write the data to memory.
            @throw Error if it can't convert stdin to binary.
         */
        void ReadStdin();
        /*!
            @brief Read the data from data uri path and write the data to memory.
            @param path The data uri.
            @throw Error if no base64 data in path.
         */
        void ReadDataUri(const std::string& path);
    }; // class XPathIo
#else
    class EXIV2API XPathIo : public FileIo {
    public:
        /*!
            @brief The extension of the temporary file which is created when getting input data
                    to read metadata. This file will be deleted in destructor.
        */
        static const std::string TEMP_FILE_EXT;
        /*!
            @brief The extension of the generated file which is created when getting input data
                    to add or modify the metadata.
        */
        static const std::string GEN_FILE_EXT;

        //! @name Creators
        //@{
        //! Default constructor that reads data from stdin/data uri path and writes them to the temp file.
        explicit XPathIo(const std::string& orgPath);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like XPathIo(const std::string& orgPath) but accepts a
              unicode url in an std::wstring.
          @note This constructor is only available on Windows.
         */
        XPathIo(const std::wstring& wOrgPathpath);
#endif
        //! Destructor. Releases all managed memory and removes the temp file.
        virtual ~XPathIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
            @brief Change the name of the temp file and make it untemporary before
                    calling the method of superclass FileIo::transfer.
         */
        virtual void transfer(BasicIo& src);

        //@}

        //! @name Static methods
        //@{
        /*!
            @brief Read the data from stdin/data uri path and write them to the file.
            @param orgPath It equals "-" if the input data's from stdin. Otherwise, it's data uri path.
            @return the name of the new file.
            @throw Error if it fails.
         */
        static std::string writeDataToFile(const std::string& orgPath);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like writeDataToFile(const std::string& orgPath) but accepts a
              unicode url in an std::wstring.
          @note This constructor is only available on Windows.
         */
        static std::string writeDataToFile(const std::wstring& wOrgPath);
#endif
        //@}

    private:
        // True if the file is a temporary file and it should be deleted in destructor.
        bool isTemp_;
        std::string tempFilePath_;
    }; // class XPathIo
#endif

}                                       // namespace Exiv2
