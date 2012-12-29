#ifndef __SKELETON_H__
#define __SKELETON_H__

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Logging/logging.h"

namespace Graphics {

class RendererBackend;

//TODO: put this together right, at the moment it's just quickly thrown together

struct SkeletonLoadArgs {
    std::string m_path; //path of skeleton file

    //TODO: more to come?  Maybe?
};

class Skeleton : public ResourceBase<Skeleton, SkeletonLoadArgs, RendererBackend> { //TODO: RendererBackend not needed for this, figure out how to write the templates to not require a loader
public:
    struct Bone {
        glm::mat4 m_transform;  //TODO: for now it's a transform while I debug this, later use quats and positions
        //glm::mat4 * m_boneOffsetHack;
    };
    
    struct BoneHeirarchy {
        unsigned int m_boneIndex;

        ~BoneHeirarchy() {
            //recursively destroy children
            for(std::vector<BoneHeirarchy *>::iterator iter = m_children.begin(); iter != m_children.end(); iter++) {
                delete *iter;
            }
        }

        BoneHeirarchy * m_parent;
        std::vector<BoneHeirarchy *> m_children;
    };

    Skeleton()
        : ResourceBase(),
        m_numBones(0),
        m_bones(NULL),
        m_heirarchy(NULL)
    {}

    virtual ~Skeleton() {
        unload();
    }

    virtual void unload();
    virtual void reload(RendererBackend * renderer);

    inline unsigned int getNumBones() const {
        return m_numBones;
    }

    inline const Bone* getBone(unsigned int boneIndex) const {
        return m_bones + boneIndex;
    }

    inline unsigned int getBone(const char * boneName) const {
        std::map<std::string, unsigned int>::const_iterator iter = m_boneNameMap.find(boneName);

        if(iter == m_boneNameMap.end()) {
            LOG_ERROR("No bone with name %s in skeleton", boneName);
            return 0;
        }
        else {
            return iter->second;
        }
    }

    inline const BoneHeirarchy* getRootBoneNode() const {
        return m_heirarchy;
    }

    inline const std::map<std::string, unsigned int>& getBoneNameMap() const {
        return m_boneNameMap;
    }
    
private:
    unsigned int m_numBones;
    Bone * m_bones;

    std::map<std::string, unsigned int> m_boneNameMap;

    BoneHeirarchy * m_heirarchy;
};

typedef ConfigurableResourceManager<uint32_t, Skeleton, SkeletonLoadArgs, RendererBackend> SkeletonManager;

}

#endif
