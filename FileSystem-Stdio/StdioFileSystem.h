#ifndef ILL_STDIO_FILE_SYSTEM_H__
#define ILL_STDIO_FILE_SYSTEM_H__

#include "FileSystem/FileSystem.h"

namespace illStdio {
class StdioFileSystem : public illFileSystem::FileSystem {
public:
	~StdioFileSystem() {}
	
	virtual void addPath(const char * path) {}

	virtual bool fileExists(const char * path) const;

    virtual illFileSystem::File * openRead(const char * path) const;
	virtual illFileSystem::File * openWrite(const char * path) const;
    virtual illFileSystem::File * openAppend(const char * path) const;
};
}

#endif
