#ifndef ILL_WAV_DECODER_H_
#define ILL_WAV_DECODER_H_

#include "Sound/Decoders/Decoder.h"

namespace illSound {
class WavDecoder : public Decoder
{
public:
    enum WavFormat {    //This is for when more .wav formats are available if ever, such as ADPCM
        WF_UNKNOWN,
        WF_PCM
    };

    WavDecoder(const char * path);
    virtual ~WavDecoder() {}

    inline WavFormat getWavFormat() const {
        return m_format;
    }

    void getData(void * destination, size_t length) {
        m_file->read(destination, length);
    }
      
protected:
    WavFormat m_format;
};
}

#endif