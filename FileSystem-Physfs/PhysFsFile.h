#ifndef ILL_PHYSFS_FILE_H__
#define ILL_PHYSFS_FILE_H__

#include <physfs.h>
#include "FileSystem/File.h"

namespace illPhysFs {
class PhysFsFileSystem;

class PhysFsFile : public illFileSystem::File {
public:
    virtual ~PhysFsFile() {
        close();
    }

    void close();
    
    size_t getSize();

    size_t tell();

    void seek(size_t offset);
    void seekAhead(size_t offset);
    bool eof();
    
    void read(void* destination, size_t size);
    
    void read8(uint8_t& destination);

    void readL16(uint16_t& destination);
    void readB16(uint16_t& destination);
    
    void readL32(uint32_t& destination);
    void readB32(uint32_t& destination);

    void readL64(uint64_t& destination);
    void readB64(uint64_t& destination);
    
    void readLF(float& destination);
    void readBF(float& destination);

    void readLD(double& destination);
    void readBD(double& destination);

private:
    PhysFsFile(PHYSFS_File * file, File::State state, const char * fileName)
        : File(state, fileName),
        m_file(file)
    {}

    PHYSFS_File * m_file;

friend PhysFsFileSystem;
};
}

#endif
