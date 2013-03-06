#ifndef ILL_LRU_CACHE_H__
#define ILL_LRU_CACHE_H__

#include <cassert>
#include <functional>
#include <unordered_map>
#include <list>
#include <chrono>

#include "RefCountPtr.h"

/**
http://timday.bitbucket.org/lru.html
http://www.codeproject.com/Articles/23396/A-High-Performance-Multi-Threaded-LRU-Cache
*/

/**
A least recently used cache that returns smart pointers to objects inside.
Once there are no more references to an object through the smart pointers, the object gets put into the least recently used list.
Objects that are in the least recently used list for a specified time get evicted when new elements get added.

@tparam Key The key to uniquely identify objects in the cache
@tparam T The object type
*/
template <typename Key, typename T>
class LruCache {
public:
    typedef RefCountPtrRoot<T> HandleRoot;

private:
    /**
    Implementation of the PtrHelper so the smart pointers refer to the cache instead of to nothing
    as is by default.
    */
    struct LruPtrHelper : public PtrHelper<T> {
        inline void onZeroReferences() {
            m_cache->releaseElement(m_key);
        }

        inline void onNonZeroReferences() {
            m_cache->unreleaseElement(m_key);
        }

        Key m_key;
        LruCache * m_cache;
    };

    /**
    Info stored about each element.
    */
    struct ElementInfo {
        ~ElementInfo() {
            delete m_element;
        }

        HandleRoot * m_element;
        std::chrono::steady_clock::time_point m_timeFreed;
        typename std::list<Key>::iterator m_freeListIterator;
    };

public:
    /**
    Constructor for the LRU cache.

    @param createFunc A lambda function that takes a Key and returns a pointer to the object managed by the cache
        to create a new instance of the object.
    @param initialSize The initial size of the internal hash map.
    @param evictionSeconds How many seconds should elements be in the cache without any references before they get
        destroyed.
    */
    LruCache(std::function<T* (Key)> createFunc, size_t initialSize = 50, std::chrono::steady_clock::duration evictionSeconds = std::chrono::steady_clock::duration(2))
        : m_createFunc(createFunc),
        m_evictionSeconds(evictionSeconds),
        m_elements(initialSize)
    {}

    /**
    Returns a smart pointer to an element in the cache.
    Whether or not the element is in the cache or needs to be created before being returned is totally abstract.
    */
    inline RefCountPtr<T> getElement(Key key) {
        auto iter = m_elements.find(key);

        //if element in cache, return it
        if(iter != m_elements.end()) {
            return RefCountPtr<T>(iter->second.m_element);
        }

        //load the element
        LruPtrHelper * helper = new LruPtrHelper();
        helper->m_key = key;
        helper->m_cache = this;

        RefCountPtrRoot<T> * element = new RefCountPtrRoot<T>(m_createFunc(key), helper);
        m_elements[key].m_element = element;

        evictElements();

        return RefCountPtr<T>(element);
    }

    /**
    Clears the contents of the cache.
    */
    inline void clear() {
        m_elements.clear();
        m_freeOrder.clear();
    }

private:
    /**
    Releases an element, placing it into the LRU list.
    The element isn't deleted just yet in case it ends up being needed again.
    */
    void releaseElement(Key key) {
        auto time = std::chrono::steady_clock::now();

        m_freeOrder.push_front(key);

        ElementInfo& elementInfo = m_elements.at(key);

        assert(elementInfo.m_freeListIterator == m_freeOrder.end());
        
        elementInfo.m_timeFreed = time;
        elementInfo.m_freeListIterator = m_freeOrder.begin();
    }

    /**
    Unreleases an element, removing it from the LRU list.
    */
    void unreleaseElement(Key key) {
        ElementInfo& elementInfo = m_elements.at(key);

        assert(elementInfo.m_freeListIterator != m_freeOrder.end());

        m_freeOrder.erase(elementInfo.m_freeListIterator);

        elementInfo.m_freeListIterator = m_freeOrder.end();
    }

    /**
    Evicts all elements in the cache who have been in for at least the number of
    eviction seconds.
    */
    void evictElements() {
        auto time = std::chrono::steady_clock::now();
        auto freeIter = m_freeOrder.begin();

        for(; freeIter != m_freeOrder.end(); freeIter++) {
            auto elementIter = m_elements.find(*freeIter);

            if(time - elementIter->second.m_timeFreed < m_evictionSeconds) {
                break;
            }
            else {
                m_elements.erase(elementIter);
            }
        }

        m_freeOrder.erase(m_freeOrder.begin(), freeIter);
    }

    std::function<T* (Key)> m_createFunc;
    std::chrono::steady_clock::duration m_evictionSeconds;
    std::unordered_map<Key, ElementInfo> m_elements;
    std::list<Key> m_freeOrder;      //one of the few places in my code where I use a list
};

#endif
