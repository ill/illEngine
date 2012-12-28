#ifndef __PHYSFS_FILE_SYSTEM_H__
#define __PHYSFS_FILE_SYSTEM_H__

#include "illFileSystem/FileSystem.h"

namespace illPhysFs {
class PhysFsFileSystem : public illFileSystem::FileSystem {
public:
    void init(const char * argv0);
    ~PhysFsFileSystem();

    void addPath(const char * path);

    illFileSystem::File * openRead(const char * path) const;
};
}

#endif