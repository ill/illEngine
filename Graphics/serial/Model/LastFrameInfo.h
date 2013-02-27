#ifndef ILL_LAST_FRAME_INFO_H_
#define ILL_LAST_FRAME_INFO_H_

namespace illGraphics {

struct LastFrameInfo {
    LastFrameInfo()
        : m_lastTime(0.0f),
        m_lastPositionKey(0),
        m_lastRotationKey(0),
        m_lastScalingKey(0)
    {}

    float m_lastTime;
    size_t m_lastPositionKey;
    size_t m_lastRotationKey;
    size_t m_lastScalingKey;
};

}

#endif