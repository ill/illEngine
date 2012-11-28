#ifndef __UIDGENERATOR_H__
#define __UIDGENERATOR_H__

#include <stdexcept>
#include <queue>

#ifndef NDEBUG
#include <set>
#endif

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
        : m_currentId(0)
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
            retVal = m_releasedIds.front();
            m_releasedIds.pop();

            isReusing = true;

#ifndef NDEBUG
            m_releaseIdSet.erase(retVal);
#endif

            return retVal;
        }
    }

    /**
    Call this to release a previously returned id from use.  This means the id is safe to reuse later. 
    */
    inline void releaseId(T id) {
#ifndef NDEBUG
        if(id >= m_currentId) {
            throw new std::runtime_error("Attempting to release an id that was never in use before.");
        }

        checkReleased(id);
#endif

        m_releasedIds.push(id);
    }

#ifndef NDEBUG
    /**
    Checks if the id has been released already.  Used only in the debug build so release build is faster.
    */
    inline void checkReleased(T id) {
        if(m_releaseIdSet.find(id) != m_releaseIdSet.end()) {
            throw new std::runtime_error("Attempting to release an id that was already released before.");
        }

        m_releaseIdSet.insert(id);
    }
#endif

private:
    T m_currentId; ///< The next id that will be ruturned when generateId() is called.
    std::queue<T> m_releasedIds;   ///< Ids that are safe to reuse, I use a queue and not a set because sets don't allow for safe concurrent removal

    //For debugging
#ifndef NDEBUG
    std::set<T> m_releaseIdSet;
#endif
};

#endif