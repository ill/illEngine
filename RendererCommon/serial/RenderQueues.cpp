#include "RenderQueues.h"

namespace illRendererCommon {

RenderQueues::RenderQueues()
    : m_mapMemory(RENDERQUEUE_SMALL_MAPS + RENDERQUEUE_LARGE_MAPS),
    m_pointerVectorMemory(RENDERQUEUE_POINTER_VECTORS),
    m_staticMeshInfoVectorMemory(RENDERQUEUE_STATIC_MESH_INFO_VECTORS)
{
    //the maps
    {
        size_t currMapInd = 0;

        for(int map = 0; map < RENDERQUEUE_SMALL_MAPS; map++, currMapInd++) {
            m_smallMapPool.push(&m_mapMemory[currMapInd]);
            m_mapMemory[currMapInd].rehash(RENDERQUEUE_SMALL_MAP_BUCKETS);
        }

        for(int map = 0; map < RENDERQUEUE_LARGE_MAPS; map++, currMapInd++) {
            m_largeMapPool.push(&m_mapMemory[currMapInd]);
            m_mapMemory[currMapInd].rehash(RENDERQUEUE_LARGE_MAP_BUCKETS);
        }
    }

    //the vectors
}

RenderQueues::~RenderQueues() {
}

}