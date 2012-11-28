#ifndef __PHYSFSUTIL_H__
#define __PHYSFSUTIL_H__

#include <string>
#include <physfs.h>

#include "Logging\logging.h"
#include "util.h"

//TODO: move this to a PhysFS specific folder thing or something when I get done with the Util/FileIO stuff

/**
Loads a file from path into destination and returns the size of data read.
Allocates memory for destination.  It's up to the caller to clean it up.
*/
inline unsigned int readFile(const std::string& path, char ** destination) {
    if(!PHYSFS_exists(path.c_str())) {
        LOG_FATAL_ERROR("File %s not found", path.c_str());
    }

    PHYSFS_file* openFile;
    PHYSFS_sint64 fileBufferLength;

    if((openFile = PHYSFS_openRead(path.c_str())) == NULL) {
        LOG_FATAL_ERROR("File %s failed to open", path.c_str());
    }

    if((fileBufferLength = PHYSFS_fileLength(openFile)) == -1) {
        LOG_FATAL_ERROR("File %s failed to get file length", path.c_str());
    }

    *destination = new char[(PHYSFS_uint32) fileBufferLength];

    if (PHYSFS_read(openFile, *destination, 1, (PHYSFS_uint32) fileBufferLength) == 0) {
        LOG_FATAL_ERROR("File %s failed to read file into buffer", path.c_str());
    }

    if (!PHYSFS_close(openFile)) {
        LOG_FATAL_ERROR("Failed to close file %s", path.c_str());
    }

    return (unsigned int) fileBufferLength;
}

#endif