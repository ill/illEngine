#ifndef ILL_POOL_H_
#define ILL_POOL_H_

#include <vector>
#include <unordered_set>

template <typename T>
class Pool {
public:
    Pool(size_t reserve = 10) {
        m_elements.resize(reserve);

        clear();
    }
    
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
    Makes the vector allocate enough space for some size.
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
        
    inline void remove(size_t elementId) {
        m_elements[elementId].m_nextFree = m_freeHead;
        m_freeHead = elementId;
    }

    inline void clear() { 
        m_freeHead = 0;

        for(size_t id = 0; id < m_elements.size(); id++) {
            m_elements[id].m_nextFree = id + 1;
        }

        m_freeTail = m_elements.size() - 1;
    }
    
    inline const T& get(size_t id) const {
        //TODO: make sure id is valid, at least in debug build
        return m_elements[id].m_element;
    }

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

    std::vector<Element> m_elements;
    size_t m_freeHead;                  ///<Set to size of the list if no free
    size_t m_freeTail;                  ///<Set to size of the list if no tail
};

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

    /**
    Makes the vector allocate enough space for some size.
    */
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
        //TODO: implement if needed
        //return m_idGenerator.isIdReleased(id);
    }

    /**
    Returns the live set so you can iterate over it.
    */
    const LiveSet& getLiveSet() const {
        return m_elementIds;
    }

private:
    LiveSet m_elementIds;
    Pool<T> m_pool;
};

#endif