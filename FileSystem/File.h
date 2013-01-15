#ifndef ILL_FILE_H__
#define ILL_FILE_H__

#include <stdint.h>
#include <string>

namespace illFileSystem {

///The buffer size for file I/O operations.  4kb is usually a good number.
const size_t IO_BUFFER_SIZE = 4096;

/**
A file opened with the file system.

This is still a work in progress and currently is a wrapper around reading files from PhysFS only.  More useful stuff will come later as needed.
*/
class File {
public:
    File()
        : m_state(ST_CLOSED)
    {}

    virtual ~File() {}

    enum State {
        ST_CLOSED,
        ST_READ,
        ST_WRITE,
        ST_READ_WRITE
    };

    const char * getFileName() const {
        return m_fileName.c_str();
    }

    /**
    Closes the file.  The file can't be reopened from here.  Open a file through the file system.
    */
    virtual void close() = 0;
    
    /**
    Gets the file size.
    */
    virtual size_t getSize() = 0;

    /**
    Gets the current offset into the file.
    */
    virtual size_t tell() = 0;

    /**
    Seeks to an offset in the file relative to the beginning of the file.
    */
    virtual void seek(size_t offset) = 0;

    /**
    Seeks to an offset in the file relative to the current position in the file.
    */
    virtual void seekAhead(size_t offset) = 0;

    /**
    Checks if at end of file
    */
    virtual bool eof() = 0;

    /**
    Gets the current state of the file.
    */
    inline State getState() const {
        return m_state;
    }

    /**
    Reads some data into a buffer.

    @param destination The destination buffer.
    @param size The number of bytes to read.
    */
    virtual void read(void* destination, size_t size) = 0;
    
    ///Convenience method to read a string into destination.
    //TODO: virtual void readString(std::string& destination) = 0;

    ///Convenience method to read a byte into destination.
    virtual void read8(uint8_t& destination) = 0;

    ///Convenience method to platform independently read a 16 bit little endian into destination.
    virtual void readL16(uint16_t& destination) = 0;
    ///Convenience method to platform independently read a 16 bit big endian into destination.
    virtual void readB16(uint16_t& destination) = 0;
    
    ///Convenience method to platform independently read a 32 bit little endian into destination.
    virtual void readL32(uint32_t& destination) = 0;
    ///Convenience method to platform independently read a 32 bit big endian into destination.
    virtual void readB32(uint32_t& destination) = 0;

    ///Convenience method to platform independently read a 64 bit little endian into destination.
    virtual void readL64(uint64_t& destination) = 0;
    ///Convenience method to platform independently read a 64 bit big endian into destination.
    virtual void readB64(uint64_t& destination) = 0;
    
    ///Convenience method to platform independently read a little endian float into destination.
    virtual void readLF(float& destination) = 0;
    ///Convenience method to platform independently read a big endian float into destination.
    virtual void readBF(float& destination) = 0;

    ///Convenience method to platform independently read a little endian double into destination.
    virtual void readLD(double& destination) = 0;
    ///Convenience method to platform independently read a big endian double into destination.
    virtual void readBD(double& destination) = 0;

    //TODO: make write methods and stuff

protected:
    File(State state, const char* fileName)
        : m_state(state),
        m_fileName(fileName)
    {}

    State m_state;
    std::string m_fileName;
};

};

#endif
