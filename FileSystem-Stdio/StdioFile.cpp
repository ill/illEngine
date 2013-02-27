#include <cassert>
#include <sys/stat.h>

#include "StdioFile.h"
#include "Logging/logging.h"

namespace illStdio {
void StdioFile::close() {
    if(fclose(m_file) == EOF) {
        LOG_FATAL_ERROR("Failed to close file %s.", getFileName());
    }
}
    
size_t StdioFile::getSize() {
	struct stat st;

	if(fstat(fileno(m_file), &st) != 0) {
		LOG_FATAL_ERROR("Failed to get file %s length.", getFileName());
	}
	
    return st.st_size;
}

size_t StdioFile::tell() {
    long int res;

    if((res = ftell(m_file)) == -1) {
        LOG_FATAL_ERROR("Failed to tell offset into file %s.", getFileName());
    }

    return (size_t) res;
}

void StdioFile::seek(size_t offset) {
    if(fseek(m_file, (long int) offset, SEEK_SET) != 0) {
        LOG_FATAL_ERROR("Failed to seek into file %s with offset %lu.", getFileName(), offset);
    }
}

void StdioFile::seekAhead(size_t offset) {
    seek(tell() + offset);
}

bool StdioFile::eof() {
    return feof(m_file) != 0;
}


void StdioFile::read(void* destination, size_t size) {
    assert(destination);
	assert(getState() == File::State::ST_READ);

    if(fread(destination, 1, size, m_file) != size) {
        LOG_FATAL_ERROR("Failed to read %u bytes from file %s.", size, getFileName());
    }
}


void StdioFile::write(const void* source, size_t size) {
    assert(source);
	assert(getState() == File::State::ST_WRITE || getState() == File::State::ST_APPEND);

    if(fwrite(source, 1, size, m_file) != size) {
        LOG_FATAL_ERROR("Failed to write %u bytes to file %s.", size, getFileName());
    }
}

}
