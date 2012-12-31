#ifndef ILL_DECODER_H_
#define ILL_DECODER_H_

#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"

namespace illSound {

/**
A decoder reads data from some sound file and extracts data from it usually in the form of raw PCM data.
If the audio backend supports some other format other than raw PCM data such as a compressed format this can read that data as well.
*/
class Decoder {
public:
    Decoder(const char * path) {
        m_file = illFileSystem::fileSystem->openRead(path);
    }

    virtual ~Decoder() {
        delete m_file;
    }

    //decodes enough raw PCM data to fill length bytes
    virtual void getData(void * destinationBuffer, size_t length) = 0;
      
    inline void seek(size_t position) {
        m_file->seek(position + m_soundDataBeginning);
    }

    inline size_t tell() {
        return m_file->tell() - m_soundDataBeginning;
    }

    inline size_t getSoundDataLength() {
        return m_soundDataLength;
    }

    inline unsigned char getNumChannels() {
        return m_numChannels;
    }

    inline unsigned int getSampleRate() {
        return m_sampleRate;
    }

    inline unsigned char getBitsPerSample() {
        return m_bitsPerSample;
    }

protected:
    illFileSystem::File * m_file;
    size_t m_soundDataLength;
    size_t m_soundDataBeginning;

    uint8_t m_numChannels;
    unsigned int m_sampleRate;
    uint8_t m_bitsPerSample;
};
}

#endif