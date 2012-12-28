#include <cassert>
#include "PhysFsFile.h"
#include "illUtil/endian.h"
#include "serial-illLogging/logging.h"

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



void PhysFsFile::read(void* destination, size_t size) {
    assert(destination);

    if(PHYSFS_read(m_file, destination, 1, (PHYSFS_uint32) size) < (PHYSFS_sint64) size) {
        LOG_FATAL_ERROR("Failed to read %u bytes from file %s. Error: %s", size, getFileName(), PHYSFS_getLastError());
    }
}



void PhysFsFile::read8(int8_t& destination) {
    read(&destination, sizeof(int8_t));
}



void PhysFsFile::readL16(int16_t& destination) {
    read(&destination, sizeof(int16_t));

    destination = little16(destination);
}

void PhysFsFile::readB16(int16_t& destination) {
    read(&destination, sizeof(int16_t));

    destination = big16(destination);
}


    
void PhysFsFile::readL32(int32_t& destination) {
    read(&destination, sizeof(int32_t));

    destination = little32(destination);
}

void PhysFsFile::readB32(int32_t& destination) {
    read(&destination, sizeof(int32_t));

    destination = big32(destination);
}



void PhysFsFile::readL64(int64_t& destination) {
    read(&destination, sizeof(int64_t));

    destination = little64(destination);
}

void PhysFsFile::readB64(int64_t& destination) {
    read(&destination, sizeof(int64_t));

    destination = big64(destination);
}


    
void PhysFsFile::readLF(float& destination) {
    read(&destination, sizeof(float));

    destination = littleF(destination);
}

void PhysFsFile::readBF(float& destination) {
    read(&destination, sizeof(float));

    destination = bigF(destination);
}



void PhysFsFile::readLD(double& destination) {
    read(&destination, sizeof(double));

    destination = littleD(destination);
}

void PhysFsFile::readBD(double& destination) {
    read(&destination, sizeof(double));

    destination = bigD(destination);
}

}