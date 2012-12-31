#ifndef ILL_SOUND_EFFECT_H_
#define ILL_SOUND_EFFECT_H_

#include <string>
#include <vector>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"

namespace illSound {
struct SoundEffectLoadArgs {
    std::vector<int> m_samples;             ///<ids of sound samples to randomly play for this sound effect
    
    ///The following can always be overridden when beginning to play the sound if needed
    bool m_loop;                            ///<Whether or not the sound effect loops 
    float m_minDistance;                    ///<The minimum distance before the volume starts to fall off (no effect if not playing as a spatial sound)
    float m_maxDistance;                    ///<The maximum distance until the sound is inaudible (no effect if not playing as a spatial sound)
    bool m_omniDirectional;                 ///<Volume fall off with distance only, no 3D sound (no effect if not playing as a spatial sound)
};

/**
A sound effect is what you actually play.  It contains a group of random sound samples to select from and various artist defined properties.
*/
class SoundEffect : public ResourceBase<SoundEffect, SoundEffectLoadArgs, SoundBackend> {
public:

private:

};
}

#endif