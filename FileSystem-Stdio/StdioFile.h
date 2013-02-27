#ifndef ILL_STDIO_FILE_H__
#define ILL_STDIO_FILE_H__

#include <cstdio>
#include "FileSystem/File.h"

namespace illStdio {
class StdioFileSystem;

class StdioFile : public illFileSystem::File {
public:
    virtual ~StdioFile() {
        close();
    }

    virtual void close();
    
    virtual size_t getSize();

    virtual size_t tell();

    virtual void seek(size_t offset);
    virtual void seekAhead(size_t offset);
    virtual bool eof();
    
    virtual void read(void* destination, size_t size);
	virtual void write(const void* source, size_t size);
    
private:
    StdioFile(FILE * file, File::State state, const char * fileName)
        : File(state, fileName),
        m_file(file)
    {}

    FILE * m_file;

friend StdioFileSystem;
};
}

#endif
