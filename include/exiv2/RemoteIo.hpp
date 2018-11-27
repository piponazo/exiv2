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
        @brief Provides remote binary file IO by implementing the BasicIo interface. This is an
            abstract class. The logics for remote access are implemented in HttpIo, CurlIo, SshIo which
            are the derived classes of RemoteIo.
    */
    class EXIV2API RemoteIo : public BasicIo {
    public:
        //! Destructor. Releases all managed memory.
        virtual ~RemoteIo();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Connect to the remote server, get the size of the remote file and
            allocate the array of blocksMap.

            If the blocksMap is already allocated (this method has been called before),
            it just reset IO position to the start and does not flush the old data.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int open();

        /*!
          @brief Reset the IO position to the start. It does not release the data.
          @return 0 if successful;<BR>
              Nonzero if failure.
         */
        virtual int close();
        /*!
          @brief Not support this method.
          @return 0 means failure
         */
        virtual long write(const byte* data, long wcount);
        /*!
          @brief Write data that is read from another BasicIo instance to the remote file.

          The write access is done in an efficient way. It only sends the range of different
          bytes between the current data and BasicIo instance to the remote machine.

          @param src Reference to another BasicIo instance. Reading start
              at the source's current IO position
          @return The size of BasicIo instance;<BR>
                 0 if failure;
          @throw Error In case of failure

          @note The write access is only supported by http, https, ssh.
         */
        virtual long write(BasicIo& src);

        /*!
         @brief Not support
         @return 0 means failure
        */
       virtual int putb(byte data);
       /*!
         @brief Read data from the memory blocks. Reading starts at the current
             IO position and the position is advanced by the number of
             bytes read.
             If the memory blocks are not populated (False), it will connect to server
             and populate the data to memory blocks.
         @param rcount Maximum number of bytes to read. Fewer bytes may be
             read if \em rcount bytes are not available.
         @return DataBuf instance containing the bytes read. Use the
               DataBuf::size_ member to find the number of bytes read.
               DataBuf::size_ will be 0 on failure.
        */
       virtual DataBuf read(long rcount);
       /*!
         @brief Read data from the the memory blocks. Reading starts at the current
             IO position and the position is advanced by the number of
             bytes read.
             If the memory blocks are not populated (!= bMemory), it will connect to server
             and populate the data to memory blocks.
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
         @brief Read one byte from the memory blocks. The IO position is
             advanced by one byte.
             If the memory block is not populated (!= bMemory), it will connect to server
             and populate the data to the memory block.
         @return The byte read from the memory block if successful;<BR>
                EOF if failure;
        */
       virtual int getb();
        /*!
          @brief Remove the contents of the file and then transfer data from
              the \em src BasicIo object into the empty file.

          The write access is done in an efficient way. It only sends the range of different
          bytes between the current data and BasicIo instance to the remote machine.

          @param src Reference to another BasicIo instance. The entire contents
              of src are transferred to this object. The \em src object is
              invalidated by the method.
          @throw Error In case of failure

          @note The write access is only supported by http, https, ssh.
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
         @brief Not support
         @return NULL
        */
       virtual byte* mmap(bool /*isWriteable*/ =false);
        /*!
          @brief Not support
          @return 0
         */
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
       //!Returns true if the memory area is allocated.
       virtual bool isopen() const;
       //!Always returns 0
       virtual int error() const;
       //!Returns true if the IO position has reached the end, otherwise false.
       virtual bool eof() const;
       //!Returns the URL of the file.
       virtual std::string path() const;
#ifdef EXV_UNICODE_PATH
       /*
         @brief Like path() but returns a unicode URL path in an std::wstring.
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

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        RemoteIo() {p_=NULL;}
        //@}

        // Pimpl idiom
        class Impl;
        //! Pointer to implementation
        Impl* p_;
    }; // class RemoteIo

    /*!
        @brief Provides the http read/write access for the RemoteIo.
    */
    class EXIV2API HttpIo : public RemoteIo {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that accepts the http URL on which IO will be
              performed. The constructor does not open the file, and
              therefore never failes.
          @param url The full path of url
          @param blockSize the size of the memory block. The file content is
                divided into the memory blocks. These blocks are populated
                on demand from the server, so it avoids copying the complete file.
         */
        HttpIo(const std::string&  url,  size_t blockSize = 1024);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like HttpIo(const std::string& url, size_t blockSize = 1024) but accepts a
              unicode url in an std::wstring.
          @note This constructor is only available on Windows.
         */
        HttpIo(const std::wstring& wurl, size_t blockSize = 1024);
#endif
        //@}
    protected:
        // NOT IMPLEMENTED
        //! Copy constructor
        HttpIo(HttpIo& rhs);
        //! Assignment operator
        HttpIo& operator=(const HttpIo& rhs);
        // Pimpl idiom
        class HttpImpl;

        //! @name Creators
        //@{
        //! Default Destructor
        virtual ~HttpIo(){}
        //@}
    };

#ifdef EXV_USE_CURL
    /*!
        @brief Provides the http, https read/write access and ftp read access for the RemoteIo.
            This class is based on libcurl.
    */
    class EXIV2API CurlIo : public RemoteIo {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that accepts the URL on which IO will be
              performed.
          @param url The full path of url
          @param blockSize the size of the memory block. The file content is
                divided into the memory blocks. These blocks are populated
                on demand from the server, so it avoids copying the complete file.
          @throw Error if it is unable to init curl pointer.
         */
        CurlIo(const std::string&  url,  size_t blockSize = 0);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like CurlIo(const std::string&  url,  size_t blockSize = 0) but accepts a
              unicode url in an std::wstring.
          @note This constructor is only available on Windows.
         */
        CurlIo(const std::wstring& wurl, size_t blockSize = 0);
#endif
        /*!
          @brief Write access is only available for some protocols. This method
                will call RemoteIo::write(const byte* data, long wcount) if the write
                access is available for the protocol. Otherwise, it throws the Error.
         */
        long write(const byte* data, long wcount);
        /*!
          @brief Write access is only available for some protocols. This method
                will call RemoteIo::write(BasicIo& src) if the write access is available
                for the protocol. Otherwise, it throws the Error.
         */
        long write(BasicIo& src);
    protected:
        // NOT IMPLEMENTED
        //! Copy constructor
        CurlIo(CurlIo& rhs);
        //! Assignment operator
        CurlIo& operator=(const CurlIo& rhs);
        // Pimpl idiom
        class CurlImpl;

        //! @name Creators
        //@{
        //! Default Destructor
        virtual ~CurlIo(){}
        //@}
    };
#endif

#ifdef EXV_USE_SSH
    /*!
        @brief Provides the ssh read/write access and sftp read access for the RemoteIo.
            This class is based on libssh.
    */
    class EXIV2API SshIo : public RemoteIo {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that accepts the URL on which IO will be
              performed.
          @param url The full path of url
          @param blockSize the size of the memory block. The file content is
                divided into the memory blocks. These blocks are populated
                on demand from the server, so it avoids copying the complete file.
          @throw Error if it is unable to init ssh session.
         */
        SshIo(const std::string&  url,  size_t blockSize = 1024);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like SshIo(const std::string&  url,  size_t blockSize = 1024) but accepts a
              unicode url in an std::wstring.
          @note This constructor is only available on Windows.
         */
        SshIo(const std::wstring& wurl, size_t blockSize = 1024);
#endif
        //@}
    protected:
        // NOT IMPLEMENTED
        //! Copy constructor
        SshIo(SshIo& rhs);
        //! Assignment operator
        SshIo& operator=(const SshIo& rhs);
        // Pimpl idiom
        class SshImpl;

        //! @name Creators
        //@{
        //! Default Destructor
        virtual ~SshIo(){}
        //@}
    };
#endif

#ifdef EXV_USE_CURL
    /*!
      @brief The callback function is called by libcurl to write the data
    */
    EXIV2API size_t curlWriter(char* data, size_t size, size_t nmemb, std::string* writerData);
#endif
}