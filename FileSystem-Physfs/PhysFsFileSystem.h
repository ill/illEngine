#ifndef ILL_PHYSFS_FILE_SYSTEM_H__
#define ILL_PHYSFS_FILE_SYSTEM_H__

#include "FileSystem/FileSystem.h"

namespace illPhysFs {
class PhysFsFileSystem : public illFileSystem::FileSystem {
public:
    void init(const char * argv0);
    void deinit();

    virtual ~PhysFsFileSystem() {
        deinit();
    }

    void addPath(const char * path);

	virtual bool fileExists(const char * path) const;

    virtual illFileSystem::File * openRead(const char * path) const;
	virtual illFileSystem::File * openWrite(const char * path) const;
    virtual illFileSystem::File * openAppend(const char * path) const;
};
}

#endif
