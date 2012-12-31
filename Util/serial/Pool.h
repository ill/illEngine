#ifndef ILL_POOL_H_
#define ILL_POOL_H_

#include <vector>
#include <queue>

/**
A pool is usuful for creating a large number of objects and not deleting them constantly.
The objects will simply be dead.  The pool can resize itself if more objects are needed.
*/
template<typename T>
struct Pool {
    /**
    Reserves some number of elements in the pool.
    */
    inline void reserve(size_t size) {
        if(m_pool.size() < size) {
            size_t originalSize = m_pool.size();

            m_pool.resize(size);

            //push all new elements to the queue, I hope this isn't too inefficient
            for(size_t elem = originalSize - 1; elem < size; elem++) {
                m_deadElements.push(&m_pool.at(elem));
            }
        }
    }

    /**
    Returns a new element from the pool.
    It resizes the pool if it's out of elements to use, or reuses old allocated dead elements if they are available.
    */
    inline T * getElement() {        
        if(m_deadElements.empty()) {            //if no dead elements, create a new one
            m_pool.resize(m_pool.size() + 1);
            return &m_pool.back();
        }
        else {                                  //if dead elements, reuse an old one
            T* res = m_deadElements.front();
            m_deadElements.pop();

            return res;
        }
    }

    /**
    Releases a live element back to the pool to be dead.
    */
    inline void releaseElement(T* element) {
        //TODO: add some error checking here to make sure released elements are actually from the pool, not just some arbitrary object
        m_deadElements.push(element);
    }

private:
    std::vector<T> m_pool;
    std::queue<T *> m_deadElements;
};

#endif