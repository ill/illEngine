#ifndef ILL_VECTORMANAGER_H__
#define ILL_VECTORMANAGER_H__

#include <vector>
#include "UidGenerator.h"

template <typename T, typename ID = size_t>
class VectorManager {
public:
    inline VectorManager() {}

    inline VectorManager(UidGenerator<ID>& idGenerator) {
        m_idGenerator = idGenerator;
    }

    inline ~VectorManager() {}

    /**
    */
    inline ID add(T element) {
        bool isReusing;
        ID newId = m_idGenerator.generateId(isReusing);

        if(isReusing) {
            m_vector[(size_t) newId] = element;
        }
        else {
            m_vector.push_back(element);
        }

        return newId;
    }

    inline ID reserveId() {
        bool isReusing;
        ID newId = m_idGenerator.generateId(isReusing);

        if(!isReusing) {
            m_vector.resize(m_vector.size() + 1);   //reserve space in the vector for a reserved id
        }

        return newId;
    }

    /**
    Makes the vector allocate enough space for some size.
    */
    inline void reserveSpace(size_t size) {
        if(m_vector.size() < size) {
            size_t originalSize = m_vector.size();

            m_vector.resize(size);

            //push all new ids, I hope this isn't too inefficient
            for(size_t elem = originalSize - 1; elem < size; elem++) {
                forceAddId();
            }
        }
    }

    inline void replace(ID elementId, T element) {
        m_vector[elementId] = element;
    }
    
    inline void remove(ID id) {
        m_idGenerator.releaseId(id);
    }

    inline void clear() {
        m_idGenerator.releaseAllIds();
    }
    
    inline T get(ID id) const {
        //TODO: make sure id is valid, at least in debug build
        return (m_vector[(size_t) id]);
    }

    inline const std::vector<T>& getVector() const {
        return m_vector;
    }

private:
    UidGenerator<ID> m_idGenerator;
    std::vector<T> m_vector;
};

#endif
