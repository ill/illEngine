#include <cassert>
#include "PhysFsFile.h"
#include "Logging/logging.h"

namespace illPhysFs {
void PhysFsFile::close() {
    if(!PHYSFS_close(m_file)) {
        LOG_FATAL_ERROR("Failed to close file %s. Error: %s", getFileName(), PHYSFS_getLastError());
    }
}
    
size_t PhysFsFile::getSize() {
    PHYSFS_sint64 res;

    if((res = PHYSFS_fileLength(m_file)) == -1) {
        LOG_FATAL_ERROR("Failed to get file %s length. Error: %s", getFileName(), PHYSFS_getLastError());
    }

    return (size_t) res;
}

size_t PhysFsFile::tell() {
    PHYSFS_sint64 res;

    if((res = PHYSFS_tell(m_file)) == -1) {
        LOG_FATAL_ERROR("Failed to tell offset into file %s. Error: %s", getFileName(), PHYSFS_getLastError());
    }

    return (size_t) res;
}

void PhysFsFile::seek(size_t offset) {
    if(PHYSFS_seek(m_file, (PHYSFS_uint64) offset) == 0) {
        LOG_FATAL_ERROR("Failed to seek into file %s with offset %lu. Error: %s", getFileName(), offset, PHYSFS_getLastError());
    }
}

void PhysFsFile::seekAhead(size_t offset) {
    seek(tell() + offset);
}

bool PhysFsFile::eof() {
    return PHYSFS_eof(m_file) != 0;
}


void PhysFsFile::read(void* destination, size_t size) {
    assert(destination);
	assert(getState() == File::State::ST_READ);

    if(PHYSFS_read(m_file, destination, 1, (PHYSFS_uint32) size) != (PHYSFS_sint64) size) {
        LOG_FATAL_ERROR("Failed to read %u bytes from file %s. Error: %s", size, getFileName(), PHYSFS_getLastError());
    }
}

void PhysFsFile::write(const void* source, size_t size) {
    assert(source);
	assert(getState() == File::State::ST_WRITE || getState() == File::State::ST_APPEND);

    if(PHYSFS_write(m_file, source, 1, (PHYSFS_uint32) size) != (PHYSFS_sint64) size) {
        LOG_FATAL_ERROR("Failed to write %u bytes to file %s. Error: %s", size, getFileName(), PHYSFS_getLastError());
    }
}

}
