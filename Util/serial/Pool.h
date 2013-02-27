#ifndef ILL_POOL_H_
#define ILL_POOL_H_

#include <unordered_set>
#include "illEngine/Util/serial/Array.h"

/**
Useful for objects that are quickly destroyed and created all the time, like particles.
This pool resizes itself it more room is needed.

The pool works by array indeces.  When you reserve a new object, you get its array index in the pool.
When you free an object, you free it using its array index, releasing it back into the pool to be returned later.

The objects in the pool are all preallocated and constructed so when you get a new object id back, just reinitialize that
object from scratch.  2 phase initialization is good for objects managed in this pool.

The IterablePool class defined below additionally keeps track of which ids are live so you can easily iterate over all live
objects in the pool, to maybe update all live particles or something.
*/
template <typename T>
class Pool {
public:
    /**
    Creates the pool with some reserved size of elements.

    @param reserve How many elements to have in the pool.  The pool will resize itself if more are needed.
    */
    Pool(size_t reserve = 10) {
        m_elements.resize(reserve);

        clear();
    }
    
    /**
    Returns an id of a free element in the pool.  Now you can use this id to release the object
    from the pool or for accessing the object.
    */
    inline size_t getFreeId() {
        if(m_freeHead == m_elements.size()) {   //if no free
            m_elements.resize(m_elements.size() + 1);
            return m_elements.size() - 1;
        }
        else {  //if free
            size_t position = m_freeHead;
            m_freeHead = m_elements[m_freeHead].m_nextFree;
            return position;
        }
    }

    /**
    Resizes the pool to hold more elements if needed.  Does nothing if trying to make the pool smaller.
    */
    inline void reserveSpace(size_t size) {
        if(m_elements.size() < size) {
            size_t originalSize = m_elements.size();

            m_elements.resize(size);
            
            //reset the tail to correctly be set to the size of the list
            m_elements[m_freeTail].m_nextFree = size;

            //a slight optimization, to index into the last position            
            --size;

            //add to the free list
            m_elements[size].m_nextFree = m_freeHead;
            m_freeHead = originalSize;

            //push all new ids, I hope this isn't too inefficient            
            for(size_t id = originalSize; id < size; id++) {
                m_elements[id].m_nextFree = id + 1;
            }
        }
    }
        
    /**
    Frees an id to be reused in the future.
    At this point, the object that this id refers to can't be considered valid anymore.
    In fact it'll be corrupted now.
    */
    inline void remove(size_t elementId) {
        m_elements[elementId].m_nextFree = m_freeHead;
        m_freeHead = elementId;
    }

    /**
    Kills all live objects, making the pool empty.  All ids are now invalid and released.
    */
    inline void clear() { 
        m_freeHead = 0;

        for(size_t id = 0; id < m_elements.size(); id++) {
            m_elements[id].m_nextFree = id + 1;
        }

        m_freeTail = m_elements.size() - 1;
    }
    
    /**
    Gets a constant reference to an element in the pool for an id.
    */
    inline const T& get(size_t id) const {
        //TODO: make sure id is valid, at least in debug build
        return m_elements[id].m_element;
    }

    /**
    Gets a reference to an element in the pool for an id.
    */
    inline T& get(size_t id) {
        //TODO: make sure id is valid, at least in debug build
        return m_elements[id].m_element;
    }

    inline bool isIdReleased(size_t id) const {
        //TODO: implement if needed
        //return m_idGenerator.isIdReleased(id);
    }
        
private:    
    union Element {
        T m_element;
        size_t m_nextFree;
    };

    Array<Element> m_elements;

    size_t m_freeHead;                  ///<Set to size of the list if no free
    size_t m_freeTail;                  ///<Set to size of the list if no tail
};


/**
This is a wrapper around the pool class by maintaining a set of which ids are live.
You can iterate over that set to get back all live objects.
*/
template <typename T>
class IterablePool {
public:
    typedef std::unordered_set<size_t> LiveSet;

    IterablePool(size_t reserve = 10)
        : m_pool(reserve),
        m_elementIds(reserve)
    {}

    inline size_t getFreeId() {
        size_t res = m_pool.getFreeId();
        m_elementIds.insert(res);

        return res;
    }

    inline void reserveSpace(size_t size) {
        m_pool.reserveSpace(size);
    }
        
    inline void remove(size_t elementId) {
        m_pool.remove(elementId);
        m_elementIds.erase(elementId);
    }

    inline void clear() {
        m_pool.clear();
        m_elementIds.clear();
    }
    
    inline const T& get(size_t id) const {
        //TODO: make sure id is valid, at least in debug build
        return m_pool.get(id);
    }

    inline T& get(size_t id) {
        //TODO: make sure id is valid, at least in debug build
        return m_pool.get(id);
    }

    inline bool isIdReleased(size_t id) const {
        return m_elementIds.find(id) == m_elementIds.end();
    }

    /**
    Returns the live set so you can iterate over it.  This is the set of ids,
    so to access the object itself call get() for an id first.
    */
    const LiveSet& getLiveSet() const {
        return m_elementIds;
    }

private:
    LiveSet m_elementIds;
    Pool<T> m_pool;
};

#endif