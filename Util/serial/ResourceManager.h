#ifndef ILL_RESOURCEMANAGER_H__
#define ILL_RESOURCEMANAGER_H__

#include "Logging/logging.h"
#include <string>
#include <map>

#include "Util/serial/LruCache.h"
#include "Util/serial/Pool.h"

template<typename Key, typename T, typename Loader>
class ResourceManager {
private:
    typedef LruCache<Key, T> LruCacheType;

public:
    ResourceManager(Loader * loader)
        : m_loader(loader),

        m_resourceCache([this] (Key key) {
            T * resource = new T();
            resource->load(key, this->m_loader);

            return resource;
        })
    {}

    ~ResourceManager() {}

    inline Loader * getLoader() const {
        return m_loader;
    }

    inline void setLoader(Loader * loader) {
        m_loader = loader;
    }

    inline RefCountPtr<T> getResource(Key key) {
        return m_resourceCache.getElement(key);
    }

private:
    Loader * m_loader;
    LruCacheType m_resourceCache;
};

/**
*/
template<typename Id, typename T, typename LoadArgs, typename Loader>
class ConfigurableResourceManager {
private:
    typedef LruCache<Id, T> LruCacheType;

public:
    ConfigurableResourceManager(Loader * loader)      
        : m_loader(loader),
        m_nameMap(NULL),
        m_loadArgs(NULL),

        m_resourceCache([this] (Id id) {
            T * resource = new T();
            resource->load(this->m_loadArgs[id], this->m_loader);

            return resource;
        })
    {}

    ~ConfigurableResourceManager() {
        delete m_nameMap;
        delete[] m_loadArgs;
    }

    inline Loader * getLoader() const {
        return m_loader;
    }

    inline void setLoader(Loader * loader) {
        m_loader = loader;
    }

    inline void initialize(LoadArgs * loadArgs, std::map<std::string, Id>* nameMap) {
        delete m_nameMap;
        delete[] m_loadArgs;
        m_resourceCache.clear();

        m_loadArgs = loadArgs;
        m_nameMap = nameMap;
    }

    inline Id getIdForName(const char * name) const {
        typename std::map<std::string, Id>::iterator idIter = m_nameMap->find(name);

        if(idIter == m_nameMap->end()) {
            LOG_FATAL_ERROR("getIdForName() passed invalid resource name %s.", name);
        }

        return (*idIter).second;
    }

    inline RefCountPtr<T> getResource(Id resourceId) {
        return m_resourceCache.getElement(resourceId);
    }

private:
    Loader * m_loader;
    LruCacheType m_resourceCache;
    std::map<std::string, Id> * m_nameMap;   
    LoadArgs * m_loadArgs;
};

#endif
