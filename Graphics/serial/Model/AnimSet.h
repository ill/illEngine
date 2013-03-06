#ifndef ILL_ANIM_SET_H_
#define ILL_ANIM_SET_H_

#include <string>
#include <map>

#include "Util/serial/ResourceBase.h"
#include "Util/serial/ResourceManager.h"

namespace illGraphics {

class GraphicsBackend;

struct AnimSetLoadArgs {
    std::string m_path; //path of anim set file

    //TODO: more to come?  Maybe?
};

/**
An animset is useful if you're ever using bone names for any reason, such as retreiving what bone index
in the character is the hand bone so you can attach a prop such as a gun to its hand.

It's also useful to keep the file when working with exporting using the illMesh tools to ensure bone indexes are consistent
across all meshes, skeletons, and animations in the anim set.  You may not need it for the game at all in some cases.

This way you can have multiple meshes for a common skeleton, or multiple skeletons common to an anim set
and reuse animations between the different skeletons.  This might be useful if you have a male and female version
of a character but want to reuse the same animations.  For example, this way the Left foot bone will always be index 5
in all animations, skeletons, and meshes that belong to an anim set.
*/
class AnimSet : public ResourceBase<AnimSetLoadArgs, GraphicsBackend> { //TODO: RendererBackend not needed for this, figure out how to write the templates to not require a loader
public:
	typedef std::map<std::string, unsigned int> BoneNameMap;

	virtual void unload();
    virtual void reload(GraphicsBackend * backend);

    inline size_t getNumBones() const {
		return m_boneNameMap.size();
    }

	/**
	Gets the bone index for a bone name.
	*/
	inline unsigned int getBone(const char * boneName) const {
        BoneNameMap::const_iterator iter = m_boneNameMap.find(boneName);

        if(iter == m_boneNameMap.end()) {
            LOG_ERROR("No bone with name %s in skeleton", boneName);
            return 0;
        }
        else {
            return iter->second;
        }
    }

private:
	BoneNameMap m_boneNameMap;
};

typedef uint32_t AnimSetId;
typedef ConfigurableResourceManager<AnimSetId, AnimSet, AnimSetLoadArgs, GraphicsBackend> AnimSetManager;

}

#endif