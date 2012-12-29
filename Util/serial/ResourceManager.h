#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include "Logging/logging.h"
#include <string>
#include <map>

#include "LruCache.h"

template<typename Key, typename T, typename Loader>
class ResourceManager {
private:
    typedef T* (ResourceManager<Key, T, Loader>::*CreateFunc)(Key);
    typedef LruCache<Key, T, CreateFunc, /*TODO: get LRU cache template specialization for NONE creator working*/ResourceManager<Key, T, Loader>/**/> LruCacheType;

public:
    ResourceManager(Loader * loader)
        : m_loader(loader)
    {
        m_resourceCache = new LruCacheType(&ResourceManager<Key, T, Loader>::loadResource, this);     //TODO: specify hash bucket size
    }

    ~ResourceManager() {
        delete m_resourceCache;
    }

    inline Loader * getLoader() const {
        return m_loader;
    }

    inline void setLoader(Loader * loader) {
        m_loader = loader;
    }

    inline RefCountPtr<T> getResource(Key key) {
        return m_resourceCache->getElement(key);
    }

private:
    /*static*/ inline T* loadResource(Key key) {    //TODO: get LRU cache template specialization for NONE creator working
        T * resource = new T();
        resource->load(key, m_loader);

        return resource;
    }

    Loader * m_loader;
    LruCacheType * m_resourceCache;
};

/**
*/
template<typename Id, typename T, typename LoadArgs, typename Loader>
class ConfigurableResourceManager {
private:
    typedef T* (ConfigurableResourceManager<Id, T, LoadArgs, Loader>::*CreateFunc)(Id);
    typedef LruCache<Id, T, CreateFunc, ConfigurableResourceManager<Id, T, LoadArgs, Loader> > LruCacheType;

public:
    ConfigurableResourceManager(Loader * loader)      
        : m_loader(loader),
        m_nameMap(NULL),
        m_loadArgs(NULL)
    {
        m_resourceCache = new LruCacheType(&ConfigurableResourceManager<Id, T, LoadArgs, Loader>::loadResource, this);     //TODO: specify hash bucket size
    }

    ~ConfigurableResourceManager() {
        delete m_nameMap;
        delete[] m_loadArgs;      
        delete m_resourceCache;
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
        m_resourceCache->clean();

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
        return m_resourceCache->getElement(resourceId);
    }

private:
    inline T* loadResource(Id id) {
        T * resource = new T();
        resource->load(m_loadArgs[id], m_loader);

        return resource;
    }

    Loader * m_loader;
    LruCacheType * m_resourceCache;
    std::map<std::string, Id> * m_nameMap;   
    LoadArgs * m_loadArgs;
};

#endif
