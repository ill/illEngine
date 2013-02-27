#include <sys/stat.h>

#include "StdioFileSystem.h"
#include "StdioFile.h"

#include "Logging/logging.h"

namespace illStdio {
bool StdioFileSystem::fileExists(const char * path) const {
	struct stat st;
	return stat(path, &st) == 0;
}

illFileSystem::File * StdioFileSystem::openRead(const char * path) const {
    FILE* file;

    if((file = fopen(path, "rb")) == NULL) {
        LOG_FATAL_ERROR("Failed to open file %s for reading.", path);
    }

    return new StdioFile(file, illFileSystem::File::State::ST_READ, path);
}

illFileSystem::File * StdioFileSystem::openWrite(const char * path) const {
    FILE* file;

    if((file = fopen(path, "wb")) == NULL) {
        LOG_FATAL_ERROR("Failed to open file %s for writing.", path);
    }

    return new StdioFile(file, illFileSystem::File::State::ST_WRITE, path);
}

illFileSystem::File * StdioFileSystem::openAppend(const char * path) const {
    FILE* file;

    if((file = fopen(path, "ab")) == NULL) {
        LOG_FATAL_ERROR("Failed to open file %s for appending.", path);
    }

    return new StdioFile(file, illFileSystem::File::State::ST_APPEND, path);
}
}