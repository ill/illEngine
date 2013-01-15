#include "WavDecoder.h"
#include "Logging/logging.h"

const int32_t RIFF_CHUNK = 0x52494646;   //"RIFF" in big endian form
const int32_t WAVE_CHUNK = 0x57415645;   //"WAVE" in big endian form
const int32_t FMT_CHUNK = 0x666d7420;    //"fmt " in big endian form
const int32_t DATA_CHUNK = 0x64617461;   //"data " in big endian form

const size_t DATA_BEGINNING = 44;       //sound data in .wav starts at byte 44

namespace illSound {
WavDecoder::WavDecoder(const char * path)
    : Decoder(path)
{
    /////////////////////////////
    //read the wav file chunk id
    //expect RIFF in ascii
    {
        uint32_t riff;
        m_file->readB32(riff);

        if(riff != RIFF_CHUNK) {
            LOG_FATAL_ERROR("Wav file is invalid -RIFF chunk-");
        }
    }

    m_file->seekAhead(4);

    /////////////////////////////
    //read the RIFF type
    {
        uint32_t type;
        m_file->readB32(type);

        if(type != WAVE_CHUNK) {
            LOG_FATAL_ERROR("Wav file is invalid -WAVE chunk-");
        }
    }

    ////////////////////////////
    //subchunk1
    ////////////////////////////

    /////////////////////////////
    //read fmt string
    {
        uint32_t fmt;
        m_file->readB32(fmt);

        if(fmt != FMT_CHUNK) {
            LOG_FATAL_ERROR("Wav file is invalid -FMT chunk-");
        }
    }

    m_file->seekAhead(4);

    ////////////////////////////
    //audio format
    {
        uint16_t fmt;
        m_file->readL16(fmt);
        m_format = (WavFormat) fmt;

        switch(m_format)
        {
        case WF_PCM:
            break;

        default:
            LOG_FATAL_ERROR("Wav file is not in PCM format.  Found format: %d", m_format);
            break;
        }
    }

    ////////////////////////////
    //num channels
    {
        uint16_t channels;
        m_file->readL16(channels);

        m_numChannels = (uint8_t) channels;
    }

    ////////////////////////////
    //sample rate
    {
        uint32_t sampleRate;
        m_file->readL32(sampleRate);

        m_sampleRate = (unsigned int) sampleRate;
    }

    m_file->seekAhead(6);

    ////////////////////////////
    //bits per sample
    {
        uint16_t bitPerSample;
        m_file->readL16(bitPerSample);

        m_bitsPerSample = (uint8_t) bitPerSample;
    }

    ////////////////////////////
    //subchunk2
    ////////////////////////////

    /////////////////////////////
    //read data string
    {
        uint32_t data;
        m_file->readB32(data);

        if(data != DATA_CHUNK) {
            LOG_FATAL_ERROR("Wav file is invalid -DATA chunk-");
        }
    }

    ////////////////////////////
    //data size
    {
        uint32_t dataSize;
        m_file->readL32(dataSize);

        m_soundDataLength = (size_t) dataSize;
    }

    m_soundDataBeginning = DATA_BEGINNING;
}
}