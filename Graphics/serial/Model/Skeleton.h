#ifndef ILL_SKELETON_H__
#define ILL_SKELETON_H__

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"
#include "Util/serial/Array.h"
#include "Logging/logging.h"

namespace illGraphics {

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
        m_heirarchy(NULL)
    {}

    virtual ~Skeleton() {
        unload();
    }

    virtual void unload();
    virtual void reload(RendererBackend * renderer);

    inline size_t getNumBones() const {
		return m_bones.size();
    }

    inline const Bone* getBone(unsigned int boneIndex) const {
        return &m_bones[boneIndex];
    }

    inline unsigned int getBone(const char * boneName) const {
        std::unordered_map<std::string, unsigned int>::const_iterator iter = m_boneNameMap.find(boneName);

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

    inline const std::unordered_map<std::string, unsigned int>& getBoneNameMap() const {
        return m_boneNameMap;
    }

	inline const std::unordered_map<unsigned int, std::string>& getBoneIndexMap() const {
        return m_boneIndexMap;
    }
    
private:
	Array<Bone> m_bones;

    std::unordered_map<std::string, unsigned int> m_boneNameMap;
	std::unordered_map<unsigned int, std::string> m_boneIndexMap;

    BoneHeirarchy * m_heirarchy;
};

typedef ConfigurableResourceManager<uint32_t, Skeleton, SkeletonLoadArgs, RendererBackend> SkeletonManager;

}

#endif
