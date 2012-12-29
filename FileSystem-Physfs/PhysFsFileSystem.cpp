#include <physfs.h>
#include "PhysFsFileSystem.h"
#include "PhysFsFile.h"

#include "Logging/logging.h"

namespace illPhysFs {
void PhysFsFileSystem::init(const char * argv0) {
    if(!PHYSFS_init(argv0)) {
        LOG_FATAL_ERROR("Failed to initialize physFS. Error %s", PHYSFS_getLastError());
    }
}

PhysFsFileSystem::~PhysFsFileSystem() {
    if (!PHYSFS_deinit()) {
        LOG_FATAL_ERROR("Failed to uninitialize physFS. Error %s", PHYSFS_getLastError());
    }
}

void PhysFsFileSystem::addPath(const char * path) {
    if(!PHYSFS_addToSearchPath(path, true)) {
        LOG_FATAL_ERROR("Failed to add search path %s. Error %s", path, PHYSFS_getLastError());
    }
}

illFileSystem::File * PhysFsFileSystem::openRead(const char * path) const {
    PHYSFS_file* file;

    if((file = PHYSFS_openRead(path)) == NULL) {
        LOG_FATAL_ERROR("Failed to open file %s for reading. Error %s", path, PHYSFS_getLastError());
    }

    //set file buffer or reading and writing will be inefficient
    if(PHYSFS_setBuffer(file, illFileSystem::IO_BUFFER_SIZE) == 0) {
        LOG_FATAL_ERROR("Failed to set file %s buffer to %d bytes. Error %s", path, illFileSystem::IO_BUFFER_SIZE, PHYSFS_getLastError());
    }

    return new PhysFsFile(file, illFileSystem::File::ST_READ, path);
}
}
