#ifndef ILL_FILE_H__
#define ILL_FILE_H__

#include <stdint.h>
#include <string>

#include "Util/endian.h"

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
        : m_state(State::ST_CLOSED)
    {}

    virtual ~File() {}

    enum class State {
        ST_CLOSED,
        ST_READ,
        ST_WRITE,
        ST_APPEND
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
    
	/**
	Writes some data from a buffer.

	@param source The source buffer.
	@param size The number of bytes to write.
	*/
	virtual void write(const void* source, size_t size) = 0;

	/**
	When expecting a string, first read how many bytes will be needed
	to store the read in string.  This would include space needed for the null
	terminating character at the end.  Just call readString to do the work for
	you after calling this method.
	*/
	inline uint16_t readStringBufferLength() {
		//read the string size
		uint16_t strlength;
		readB16(strlength);

		return strlength + 1;
	}

	/**
	First call readStringBufferLength() to make sure a buffer big enough to
	hold the stored string is allocated.

	Next call this to read in the string and pass the value returned from
	readStringBufferLength() as the parameter for stringBufferLength.

	In the binary format, strings are stored as a 16 bit big endian value
	with the length of the string.  The next byte array in the file is
	the string itself but isn't null terminated.  The readString method takes
	care of reading the string and properly appending the null character at the end.
	*/
	inline void readString(char * destination, uint16_t stringBufferLength) {
		--stringBufferLength;

		read(destination, stringBufferLength);		//read data into the buffer
		destination[stringBufferLength] = '\0';		//append the null character
	}
	
    ///Convenience method to read a byte into destination.
    inline void read8(uint8_t& destination) {
		read(&destination, sizeof(uint8_t));
	}

    ///Convenience method to platform independently read a 16 bit little endian into destination.
    inline void readL16(uint16_t& destination) {
		read(&destination, sizeof(uint16_t));
		destination = little16(destination);
	}

    ///Convenience method to platform independently read a 16 bit big endian into destination.
    inline void readB16(uint16_t& destination) {
		read(&destination, sizeof(uint16_t));
		destination = big16(destination);
	}
    
    ///Convenience method to platform independently read a 32 bit little endian into destination.
    inline void readL32(uint32_t& destination) {
		read(&destination, sizeof(uint32_t));
		destination = little32(destination);
	}

    ///Convenience method to platform independently read a 32 bit big endian into destination.
    inline void readB32(uint32_t& destination) {
	    read(&destination, sizeof(uint32_t));
		destination = big32(destination);
	}

    ///Convenience method to platform independently read a 64 bit little endian into destination.
    inline void readL64(uint64_t& destination) {
		read(&destination, sizeof(uint64_t));
		destination = little64(destination);
	}

    ///Convenience method to platform independently read a 64 bit big endian into destination.
    inline void readB64(uint64_t& destination) {
		read(&destination, sizeof(uint64_t));
		destination = big64(destination);
	}
    
    ///Convenience method to platform independently read a little endian float into destination.
    inline void readLF(float& destination) {
	    read(&destination, sizeof(float));
		destination = littleF(destination);
	}

    ///Convenience method to platform independently read a big endian float into destination.
    inline void readBF(float& destination) {
		read(&destination, sizeof(float));
		destination = bigF(destination);
	}

    ///Convenience method to platform independently read a little endian double into destination.
    inline void readLD(double& destination) {
		read(&destination, sizeof(double));
		destination = littleD(destination);
	}

    ///Convenience method to platform independently read a big endian double into destination.
    inline void readBD(double& destination) {
		read(&destination, sizeof(double));
		destination = bigD(destination);
	}


	/**
	Writes a NULL terminated C string to the binary file.

	In the binary format, strings are stored as a 16 bit big endian value
	with the length of the string.  The next byte array in the file is
	the string itself and isn't NULL terminated.
	*/
	inline void writeString(const char * string) {
		writeString(string, (uint16_t) strlen(string));
	}

	/**
	Writes a string to the binary file with a specified length.

	In the binary format, strings are stored as a 16 bit big endian value
	with the length of the string.  The next byte array in the file is
	the string itself.
	*/
	inline void writeString(const char * string, uint16_t length) {
		writeB16(length);
		write(string, (size_t) length);
	}

	///Convenience method to write a byte.
    inline void write8(uint8_t source) {
		write(&source, sizeof(uint8_t));
	}

    ///Convenience method to platform independently write a 16 bit little endian.
    inline void writeL16(uint16_t source) {
		source = little16(source);
		write(&source, sizeof(uint16_t));
	}

    ///Convenience method to platform independently write a 16 bit big endian.
    inline void writeB16(uint16_t source) {
		source = big16(source);
		write(&source, sizeof(uint16_t));
	}
    
    ///Convenience method to platform independently write a 32 bit little endian.
    inline void writeL32(uint32_t source) {
		source = little32(source);
		write(&source, sizeof(uint32_t));
	}

    ///Convenience method to platform independently write a 32 bit big endian.
    inline void writeB32(uint32_t source) {
		source = big32(source);
		write(&source, sizeof(uint32_t));
	}

    ///Convenience method to platform independently write a 64 bit little endian.
    inline void writeL64(uint64_t source) {
		source = little64(source);
		write(&source, sizeof(uint64_t));
	}

    ///Convenience method to platform independently write a 64 bit big endian.
    inline void writeB64(uint64_t source) {
		source = big64(source);
		write(&source, sizeof(uint64_t));
	}
    
    ///Convenience method to platform independently write a little endian float.
    inline void writeLF(float source) {
		source = littleF(source);
		write(&source, sizeof(float));
	}

    ///Convenience method to platform independently write a big endian float.
    inline void writeBF(float source) {
		source = bigF(source);
		write(&source, sizeof(float));
	}

    ///Convenience method to platform independently write a little endian double.
    inline void writeLD(double source) {
		source = littleD(source);
		write(&source, sizeof(double));
	}

    ///Convenience method to platform independently write a big endian double.
    inline void writeBD(double source) {
		source = bigD(source);
		write(&source, sizeof(double));
	}

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
