// ***************************************************************** -*- C++ -*-
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
/*
  File:      basicio.hpp
 */
#ifndef BASICIO_HPP_
#define BASICIO_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <memory>       // for std::auto_ptr
#include <fstream>      // write the temporary file
#include <fcntl.h>      // _O_BINARY in FileIo::FileIo


// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief An interface for simple binary IO.

      Designed to have semantics and names similar to those of C style FILE*
      operations. Subclasses should all behave the same so that they can be
      interchanged.
     */
    class EXIV2API BasicIo {
    public:
        //! BasicIo auto_ptr type
        typedef std::auto_ptr<BasicIo> AutoPtr;

        //! Seek starting positions
        enum Position { beg, cur, end };

        //! @name Creators
        //@{
        //! Destructor
        virtual ~BasicIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Open the IO source using the default access mode. The
              default mode should allow for reading and writing.

          This method can also be used to "reopen" an IO source which will
          flush any unwritten data and reset the IO position to the start.
          Subclasses may provide custom methods to allow for
          opening IO sources differently.

          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int open() = 0;

        /*!
          @brief Close the IO source. After closing a BasicIo instance can not
              be read or written. Closing flushes any unwritten data. It is
              safe to call close on a closed instance.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int close() = 0;
        /*!
          @brief Write data to the IO source. Current IO position is advanced
              by the number of bytes written.
          @param data Pointer to data. Data must be at least \em wcount
              bytes long
          @param wcount Number of bytes to be written.
          @return Number of bytes written to IO source successfully;<BR>
              0 if failure;
         */
        virtual long write(const byte* data, long wcount) = 0;
        /*!
          @brief Write data that is read from another BasicIo instance to
              the IO source. Current IO position is advanced by the number
              of bytes written.
          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return Number of bytes written to IO source successfully;<BR>
              0 if failure;
         */
        virtual long write(BasicIo& src) = 0;
        /*!
          @brief Write one byte to the IO source. Current IO position is
              advanced by one byte.
          @param data The single byte to be written.
          @return The value of the byte written if successful;<BR>
              EOF if failure;
         */
        virtual int putb(byte data) = 0;
        /*!
          @brief Read data from the IO source. Reading starts at the current
              IO position and the position is advanced by the number of bytes
              read.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return DataBuf instance containing the bytes read. Use the
              DataBuf::size_ member to find the number of bytes read.
              DataBuf::size_ will be 0 on failure.
         */
        virtual DataBuf read(long rcount) = 0;
        /*!
          @brief Read data from the IO source. Reading starts at the current
              IO position and the position is advanced by the number of bytes
              read.
          @param buf Pointer to a block of memory into which the read data
              is stored. The memory block must be at least \em rcount bytes
              long.
          @param rcount Maximum number of bytes to read. Fewer bytes may be
              read if \em rcount bytes are not available.
          @return Number of bytes read from IO source successfully;<BR>
              0 if failure;
         */
        virtual long read(byte* buf, long rcount) = 0;
        /*!
          @brief Read one byte from the IO source. Current IO position is
              advanced by one byte.
          @return The byte read from the IO source if successful;<BR>
              EOF if failure;
         */
        virtual int getb() = 0;
        /*!
          @brief Remove all data from this object's IO source and then transfer
              data from the \em src BasicIo object into this object.

          The source object is invalidated by this operation and should not be
          used after this method returns. This method exists primarily to
          be used with the BasicIo::temporary() method.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure
         */
        virtual void transfer(BasicIo& src) = 0;
        /*!
          @brief Move the current IO position.
          @param offset Number of bytes to move the position relative
              to the starting position specified by \em pos
          @param pos Position from which the seek should start
          @return 0 if successful;<BR>
              Nonzero if failure;
         */
#if defined(_MSC_VER)
        virtual int seek(int64_t offset, Position pos) = 0;
#else
        virtual int seek(long offset, Position pos) = 0;
#endif

        /*!
          @brief Direct access to the IO data. For files, this is done by
                 mapping the file into the process's address space; for memory
                 blocks, this allows direct access to the memory block.
          @param isWriteable Set to true if the mapped area should be writeable
                 (default is false).
          @return A pointer to the mapped area.
          @throw Error In case of failure.
         */
        virtual byte* mmap(bool isWriteable =false) =0;
        /*!
          @brief Remove a mapping established with mmap(). If the mapped area
                 is writeable, this ensures that changes are written back.
          @return 0 if successful;<BR>
                  Nonzero if failure;
         */
        virtual int munmap() =0;

        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Get the current IO position.
          @return Offset from the start of IO if successful;<BR>
                 -1 if failure;
         */
        virtual long tell() const = 0;
        /*!
          @brief Get the current size of the IO source in bytes.
          @return Size of the IO source in bytes;<BR>
                 -1 if failure;
         */
        virtual size_t size() const = 0;
        //!Returns true if the IO source is open, otherwise false.
        virtual bool isopen() const = 0;
        //!Returns 0 if the IO source is in a valid state, otherwise nonzero.
        virtual int error() const = 0;
        //!Returns true if the IO position has reached the end, otherwise false.
        virtual bool eof() const = 0;
        /*!
          @brief Return the path to the IO resource. Often used to form
              comprehensive error messages where only a BasicIo instance is
              available.
         */
        virtual std::string path() const =0;
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like path() but returns a unicode path in an std::wstring.
          @note This function is only available on Windows.
         */
        virtual std::wstring wpath() const =0;
#endif

        /*!
          @brief Mark all the bNone blocks to bKnow. This avoids allocating memory
            for parts of the file that contain image-date (non-metadata/pixel data)

          @note This method should be only called after the concerned data (metadata)
                are all downloaded from the remote file to memory.
         */
        virtual void populateFakeData() {}

        /*!
          @brief this is allocated and populated by mmap()
         */
        byte* bigBlock_;

        //@}

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        BasicIo() : bigBlock_(NULL) {};
        //@}
    }; // class BasicIo

    /*!
      @brief Utility class that closes a BasicIo instance upon destruction.
          Meant to be used as a stack variable in functions that need to
          ensure BasicIo instances get closed. Useful when functions return
          errors from many locations.
     */
    class EXIV2API IoCloser {
    public:
        //! @name Creators
        //@{
        //! Constructor, takes a BasicIo reference
        explicit IoCloser(BasicIo& bio) : bio_(bio) {}
        //! Destructor, closes the BasicIo reference
        virtual ~IoCloser() { close(); }
        //@}

        //! @name Manipulators
        //@{
        //! Close the BasicIo if it is open
        void close() { if (bio_.isopen()) bio_.close(); }
        //@}

        // DATA
        //! The BasicIo reference
        BasicIo& bio_;

    private:
        // Not implemented
        //! Copy constructor
        IoCloser(const IoCloser&);
        //! Assignment operator
        IoCloser& operator=(const IoCloser&);
    }; // class IoCloser


// *****************************************************************************
// template, inline and free functions

    /*!
      @brief replace each substring of the subject that matches the given search string with the given replacement.
      @return the subject after replacing.
     */
    EXIV2API std::string ReplaceStringInPlace(std::string subject, const std::string& search,
                          const std::string& replace);
#ifdef EXV_UNICODE_PATH
    /*!
      @brief Like ReplaceStringInPlace() but accepts a unicode path in an std::wstring.
      @return the subject after replacing.
      @note This function is only available on Windows.
     */
    EXIV2API std::wstring ReplaceStringInPlace(std::wstring subject, const std::wstring& search,
                          const std::wstring& replace);
#endif
}                                       // namespace Exiv2
#endif                                  // #ifndef BASICIO_HPP_
