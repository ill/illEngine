#ifndef ILL_UIDGENERATOR_H__
#define ILL_UIDGENERATOR_H__

#include <stdexcept>
#include <unordered_set>

/**
A class that generates unique id's of type T.
All it does it it always returns the next id after incrementing from the previous returned id.
It does allow old ids that are no longer in use to be reused though.
*/
template <typename T>
class UidGenerator {
public:
    /**
    Create the id generator
    */
    inline UidGenerator()
        : m_currentId((T) 0)
    {}

    /**
    Creates the id generator from a previous state
    @param currentId What id will the id generator return next.
    @param freeIds A queue of ids that have been released and are safe to reuse.
    */
    /*inline UidGenerator(T currentId, const tbb::concurrent_queue<T>& releasedIds)
    : m_currentId(currentId),
    m_releasedIds(releasedIds)
    {
    #ifndef NDEBUG
    tbb::concurrent_queue<T> releasedIdsCopy = m_releasedIds;

    while(!releasedIdsCopy.empty()) {
    checkReleased(releasedIdsCopy.front());
    releasedIdsCopy.pop();
    }
    #endif
    }*/

    inline void reset() {
        m_currentId = (T) 0;
        m_releasedIds.clear();
    }

    /**
    Call this to get a new unique id.
    */
    inline T generateId(bool& isReusing) {
        T retVal;

        if(m_releasedIds.empty()) {
            isReusing = false;
            return m_currentId++;
        }
        else {
            std::unordered_set<T>::iterator iter = m_releasedIds.first();

            retVal = *iter;
            m_releasedIds.remove(iter);

            isReusing = true;
            
            return retVal;
        }
    }

    /**
    Forces a new released id to be added to the uid generator.
    */
    inline T forceAddId() {
        m_releasedIds.insert(m_currentId++);

        return m_currentId - (T)1;
    }

    /**
    Releases all ids.
    */
    inline void releaseAllIds() {
        for(T id = (T) 0; id < m_currentId; id++) {
            m_releasedIds.insert(id);
        }
    }

    /**
    Call this to release a previously returned id from use.  This means the id is safe to reuse later. 
    */
    inline void releaseId(T id) {
        if(id >= m_currentId) {
            throw new std::runtime_error("Attempting to release an id that was never in use before.");
        }
        checkReleased(id);

        m_releasedIds.insert(id);
    }

    /**
    Checks if the id has been released already.  Used only in the debug build so release build is faster.
    */
    inline void checkReleased(T id) {
        if(m_releasedIds.find(id) != m_releasedIds.end()) {
            throw new std::runtime_error("Attempting to release an id that was already released before.");
        }
    }

private:
    T m_currentId; ///< The next id that will be ruturned when generateId() is called.
    std::unordered_set<T> m_releasedIds;   ///< Ids that are safe to reuse
};

#endif