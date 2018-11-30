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

#include "MemIo.hpp"

#include "error.hpp"

// + standard includes
#include <string>
#include <ctime>
#include <memory>
#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>                       // for remove, rename
#include <cstdlib>                      // for alloc, realloc, free
#include <sys/types.h>                  // for stat, chmod
#include <sys/stat.h>                   // for stat, chmod

#ifdef EXV_HAVE_SYS_MMAN_H
# include <sys/mman.h>                  // for mmap and munmap
#endif
#ifdef EXV_HAVE_PROCESS_H
# include <process.h>
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for getpid, stat
#endif

// Platform specific headers for handling extended attributes (xattr)
#if defined(__APPLE__)
# include <sys/xattr.h>
#endif

#if defined(__MINGW__) || (defined(WIN32) && !defined(__CYGWIN))
// Windows doesn't provide nlink_t
typedef short nlink_t;
# include <windows.h>
# include <io.h>
#endif

namespace Exiv2 {

    //! Internal Pimpl structure of class MemIo.
    class MemIo::Impl {
    public:
        Impl();                            //!< Default constructor
        Impl(const byte* data, long size); //!< Constructor 2

        // DATA
        byte* data_;                       //!< Pointer to the start of the memory area
        size_t idx_;                       //!< Index into the memory area
        size_t size_;                      //!< Size of the memory area
        size_t sizeAlloced_;               //!< Size of the allocated buffer
        bool isMalloced_;                  //!< Was the buffer allocated?
        bool eof_;                         //!< EOF indicator

        // METHODS
        void reserve(size_t wcount);         //!< Reserve memory

    private:
        // NOT IMPLEMENTED
        Impl(const Impl& rhs);             //!< Copy constructor
        Impl& operator=(const Impl& rhs);  //!< Assignment

    }; // class MemIo::Impl

    MemIo::Impl::Impl()
        : data_(0),
          idx_(0),
          size_(0),
          sizeAlloced_(0),
          isMalloced_(false),
          eof_(false)
    {
    }

    MemIo::Impl::Impl(const byte* data, long size)
        : data_(const_cast<byte*>(data)),
          idx_(0),
          size_(size),
          sizeAlloced_(0),
          isMalloced_(false),
          eof_(false)
    {
    }

    void MemIo::Impl::reserve(size_t wcount)
    {
        const size_t need = wcount + idx_;
        size_t blockSize = 32*1024;   // 32768
        const size_t maxBlockSize = 4*1024*1024;

        if (!isMalloced_) {
            // Minimum size for 1st block
            size_t size  = EXV_MAX(blockSize * (1 + need / blockSize), size_);
            byte* data = static_cast<byte*>(std::malloc(size)); /// \todo use new/delete instead??
            if (  data == NULL ) {
                throw Error(kerMallocFailed);
            }
            if (data_ != NULL) {
                std::memcpy(data, data_, size_);
            }
            data_ = data;
            sizeAlloced_ = size;
            isMalloced_ = true;
        }

        if (need > size_) {
            if (need > sizeAlloced_) {
                blockSize = 2*sizeAlloced_ ;
                if ( blockSize > maxBlockSize ) blockSize = maxBlockSize ;
                // Allocate in blocks
                size_t want  = blockSize * (1 + need / blockSize );
                data_ = static_cast<byte*>(std::realloc(data_, want));
                if ( data_ == NULL ) {
                    throw Error(kerMallocFailed);
                }
                sizeAlloced_ = want;
                isMalloced_ = true;
            }
            size_ = need;
        }
    }

    MemIo::MemIo()
        : p_(new Impl())
    {
    }

    MemIo::MemIo(const byte* data, long size)
        : p_(new Impl(data, size))
    {
    }

    MemIo::~MemIo()
    {
        if (p_->isMalloced_) {
            std::free(p_->data_);
        }
    }

    long MemIo::write(const byte* data, long wcount)
    {
        const size_t count = static_cast<size_t>(wcount);
        p_->reserve(count);
        assert(p_->isMalloced_);
        if (data != NULL) {
            std::memcpy(&p_->data_[p_->idx_], data, count);
        }
        p_->idx_ += count;
        return wcount;
    }

    void MemIo::transfer(BasicIo& src)
    {
        MemIo *memIo = dynamic_cast<MemIo*>(&src);
        if (memIo) {
            // Optimization if src is another instance of MemIo
            if (p_->isMalloced_) {
                std::free(p_->data_);
            }
            p_->idx_ = 0;
            p_->data_ = memIo->p_->data_;
            p_->size_ = memIo->p_->size_;
            p_->isMalloced_ = memIo->p_->isMalloced_;
            memIo->p_->idx_ = 0;
            memIo->p_->data_ = 0;
            memIo->p_->size_ = 0;
            memIo->p_->isMalloced_ = false;
        }
        else {
            // Generic reopen to reset position to start
            if (src.open() != 0) {
                throw Error(kerDataSourceOpenFailed, src.path(), strError());
            }
            p_->idx_ = 0;
            write(src);
            src.close();
        }
        if (error() || src.error()) throw Error(kerMemoryTransferFailed, strError());
    }

    long MemIo::write(BasicIo& src)
    {
        if (static_cast<BasicIo*>(this) == &src) return 0;
        if (!src.isopen()) return 0;

        byte buf[4096];
        long readCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            write(buf, readCount);
            writeTotal += readCount;
        }

        return writeTotal;
    }

    int MemIo::putb(byte data)
    {
        p_->reserve(1);
        assert(p_->isMalloced_);
        p_->data_[p_->idx_++] = data;
        return data;
    }

#if defined(_MSC_VER)
    int MemIo::seek( int64_t offset, Position pos )
    {
        uint64_t newIdx = 0;

        switch (pos) {
        case BasicIo::cur: newIdx = p_->idx_ + offset; break;
        case BasicIo::beg: newIdx = offset; break;
        case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        p_->idx_ = static_cast<long>(newIdx);   //not very sure about this. need more test!!    - note by Shawn  fly2xj@gmail.com //TODO
        p_->eof_ = false;
        return 0;
    }
#else
    int MemIo::seek(long offset, Position pos)
    {
        long newIdx = 0;

        switch (pos) {
        case BasicIo::cur: newIdx = p_->idx_ + offset; break;
        case BasicIo::beg: newIdx = offset; break;
        case BasicIo::end: newIdx = p_->size_ + offset; break;
        }

        if (newIdx < 0) return 1;
        p_->idx_ = newIdx;
        p_->eof_ = false;
        return 0;
    }
#endif

    byte* MemIo::mmap(bool /*isWriteable*/)
    {
        return p_->data_;
    }

    int MemIo::munmap()
    {
        return 0;
    }

    long MemIo::tell() const
    {
        return p_->idx_;
    }

    size_t MemIo::size() const
    {
        return p_->size_;
    }

    int MemIo::open()
    {
        p_->idx_ = 0;
        p_->eof_ = false;
        return 0;
    }

    bool MemIo::isopen() const
    {
        return true;
    }

    int MemIo::close()
    {
        return 0;
    }

    DataBuf MemIo::read(long rcount)
    {
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long MemIo::read(byte* buf, long rcount)
    {
        long avail = EXV_MAX(p_->size_ - p_->idx_, 0);
        long allow = EXV_MIN(rcount, avail);
        std::memcpy(buf, &p_->data_[p_->idx_], allow);
        p_->idx_ += allow;
        if (rcount > avail)
            p_->eof_ = true;
        return allow;
    }

    int MemIo::getb()
    {
        if (p_->idx_ >= p_->size_) {
            p_->eof_ = true;
            return EOF;
        }
        return p_->data_[p_->idx_++];
    }

    int MemIo::error() const
    {
        return 0;
    }

    bool MemIo::eof() const
    {
        return p_->eof_;
    }

    std::string MemIo::path() const
    {
        return "MemIo";
    }

#ifdef EXV_UNICODE_PATH
    std::wstring MemIo::wpath() const
    {
        return EXV_WIDEN("MemIo");
    }

#endif
    void MemIo::populateFakeData() {

    }

#if EXV_XPATH_MEMIO
    XPathIo::XPathIo(const std::string& path) {
        Protocol prot = fileProtocol(path);

        if (prot == pStdin)         ReadStdin();
        else if (prot == pDataUri)  ReadDataUri(path);
    }
#ifdef EXV_UNICODE_PATH
    XPathIo::XPathIo(const std::wstring& wpath) {
        std::string path;
        path.assign(wpath.begin(), wpath.end());
        Protocol prot = fileProtocol(path);
        if (prot == pStdin)         ReadStdin();
        else if (prot == pDataUri)  ReadDataUri(path);
    }
#endif

    void XPathIo::ReadStdin() {
        if (isatty(fileno(stdin)))
            throw Error(kerInvalidIccProfile);

#ifdef _O_BINARY
        // convert stdin to binary
        if (_setmode(_fileno(stdin), _O_BINARY) == -1)
            throw Error(kerInvalidXMP);
#endif

        char readBuf[100*1024];
        std::streamsize readBufSize = 0;
        do {
            std::cin.read(readBuf, sizeof(readBuf));
            readBufSize = std::cin.gcount();
            if (readBufSize > 0) {
                write((byte*)readBuf, (long)readBufSize);
            }
        } while(readBufSize);
    }

    void XPathIo::ReadDataUri(const std::string& path) {
        size_t base64Pos = path.find("base64,");
        if (base64Pos == std::string::npos)
            throw Error(kerErrorMessage, "No base64 data");

        std::string data = path.substr(base64Pos+7);
        char* decodeData = new char[data.length()];
        long size = base64decode(data.c_str(), decodeData, data.length());
        if (size > 0)
            write((byte*)decodeData, size);
        else
            throw Error(kerErrorMessage, "Unable to decode base 64.");
        delete[] decodeData;
    }

#else
    const std::string XPathIo::TEMP_FILE_EXT = ".exiv2_temp";
    const std::string XPathIo::GEN_FILE_EXT  = ".exiv2";

    XPathIo::XPathIo(const std::string& orgPath) : FileIo(XPathIo::writeDataToFile(orgPath)) {
        isTemp_ = true;
        tempFilePath_ = path();
    }

#ifdef EXV_UNICODE_PATH
    XPathIo::XPathIo(const std::wstring& wOrgPathpath) : FileIo(XPathIo::writeDataToFile(wOrgPathpath)) {
        isTemp_ = true;
        tempFilePath_ = path();
    }
#endif

    XPathIo::~XPathIo() {
        if (isTemp_ && remove(tempFilePath_.c_str()) != 0) {
            // error when removing file
            // printf ("Warning: Unable to remove the temp file %s.\n", tempFilePath_.c_str());
        }
    }

    void XPathIo::transfer(BasicIo& src) {
        if (isTemp_) {
            // replace temp path to gent path.
            std::string currentPath = path();
            setPath(ReplaceStringInPlace(currentPath, XPathIo::TEMP_FILE_EXT, XPathIo::GEN_FILE_EXT));
            // rename the file
            tempFilePath_ = path();
            if (rename(currentPath.c_str(), tempFilePath_.c_str()) != 0) {
                // printf("Warning: Failed to rename the temp file. \n");
            }
            isTemp_ = false;
            // call super class method
            FileIo::transfer(src);
        }
    }

    std::string XPathIo::writeDataToFile(const std::string& orgPath) {
        Protocol prot = fileProtocol(orgPath);

        // generating the name for temp file.
        std::time_t timestamp = std::time(NULL);
        std::stringstream ss;
        ss << timestamp << XPathIo::TEMP_FILE_EXT;
        std::string path = ss.str();
        std::ofstream fs(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

        if (prot == pStdin) {
            if (isatty(fileno(stdin)))
                throw Error(kerInvalidIccProfile);
#if defined(_MSC_VER) || defined(__MINGW__)
            // convert stdin to binary
            if (_setmode(_fileno(stdin), _O_BINARY) == -1)
                throw Error(kerInvalidXMP);
#endif
            // read stdin and write to the temp file.
            char readBuf[100*1024];
            std::streamsize readBufSize = 0;
            do {
                std::cin.read(readBuf, sizeof(readBuf));
                readBufSize = std::cin.gcount();
                if (readBufSize > 0) {
                    fs.write (readBuf, readBufSize);
                }
            } while(readBufSize);
        } else if (prot == pDataUri) {
            // read data uri and write to the temp file.
            size_t base64Pos = orgPath.find("base64,");
            if (base64Pos == std::string::npos)
                throw Error(kerErrorMessage, "No base64 data");

            std::string data = orgPath.substr(base64Pos+7);
            char* decodeData = new char[data.length()];
            long size = base64decode(data.c_str(), decodeData, data.length());
            if (size > 0)
                fs.write(decodeData, size);
            else
                throw Error(kerErrorMessage, "Unable to decode base 64.");
            delete[] decodeData;
        }

        fs.close();
        return path;
    }

#ifdef EXV_UNICODE_PATH
    std::string XPathIo::writeDataToFile(const std::wstring& wOrgPath) {
        std::string orgPath;
        orgPath.assign(wOrgPath.begin(), wOrgPath.end());
        return XPathIo::writeDataToFile(orgPath);
    }
#endif

#endif
}
