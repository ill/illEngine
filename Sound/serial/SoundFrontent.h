#ifndef ILL_SOUND_FRONTEND_H_
#define ILL_SOUND_FRONTEND_H_

#include "Util/serial/Pool.h"

namespace illSound {
class SoundFrontend {
public:


private:
    struct PlayingSound {
        enum Type {
            TY_POSITIONLESS,
            TY_POSITIONAL
        };

        size_t vecManId;
        bool m_active;

        size_t finalVecManId;
        bool m_finalActive;
    };

    Pool<PlayingSound> m_soundPool;

    /**
    The non positional sound channels, these have higher priority than positional since this
    would usually be for things like music, UI, and sounds played in first person perspective.

    Active channels play through the speaker, while virtual channels aren't audible but are still updating in time
    so when they suddenly become active, they pop in at the right time.
    */
    IterablePool<PlayingSound *> m_activeChannels;
    IterablePool<PlayingSound *> m_virtualChannels;

    /**
    The sounds in the world that are positional
    */
    IterablePool<PlayingSound *> m_activePositionalChannels;
    IterablePool<PlayingSound *> m_virtualPositionalChannels;

    /**
    After deciding first which non positional channels are active, then which remaining positional channels are active,
    these are the ones that ultimately play through the speaker.
    */
    IterablePool<PlayingSound *> m_finalActiveChannels;
};
}

#endif