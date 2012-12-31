#ifndef ILL_SAMPLE_H_
#define ILL_SAMPLE_H_

#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"

namespace illSound {
struct SampleLoadArgs {
    std::string m_path;

    /**
    If a sound is streaming it's not loaded into RAM all at once but streams in chunks from the media.  Use for music and other large sounds that play once.
    */
    bool m_stream;
};

/**
A sound sample is some sound file
*/
class Sample : public ResourceBase<Sample, SampleLoadArgs, SoundBackend> {
public:

private:

};

}

#endif