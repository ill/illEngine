#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

namespace illFileSystem {

class File;

/**
File system for reading archived resources and read/writing to files elsewhere.

This is still a work in progress and currently is a wrapper around reading files from PhysFS only.  More useful stuff will come later as needed.
*/
class FileSystem {
public:
    virtual ~FileSystem() {}

    /**
    Adds a search path in which files can be found.
    */
    virtual void addPath(const char * path) = 0;

    /**
    Opens an file for reading relative to one of the search paths added.
    To 
    */
    virtual File * openRead(const char * path) const = 0;
};

//a public global variable, problem?
extern FileSystem * fileSystem;

}

#endif