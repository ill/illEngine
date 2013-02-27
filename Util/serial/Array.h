#ifndef ILL_ARRAY_H_
#define ILL_ARRAY_H_

#include <glm/glm.hpp>
#include <cassert>
#include <cstdlib>

/**
A wrapper around plain arrays.  Simpler than vector since it
simply allocates memory rather than calling constructors and all that crazyness.
This should save some CPU cycles as well ass taking off some limitations by C++
of what kinds of classes can be stored in a collection.
*/
template <typename T>
class Array {
public:
    /**
    Creates an empty array.
    */
    inline Array()
        : m_data(NULL),
        m_size(0)
    {}

    /**
    Creates an array with some size.
    */
    inline Array(size_t size)
        : m_data(NULL)
    {
        resize(size);
    }

    inline ~Array() {
        free(m_data);
    }

    /**
    Forces the array to either shrink or grow to the new size.
    */
    inline void resize(size_t size) {
        m_size = size;
        m_data = (T*) realloc(m_data, m_size * sizeof(T));
    }

    /**
    Reserves space in the array.  If the array is currently too small
    for this amount of data, it doubles the array size, similar to vectors.
    */
    inline void reserve(size_t size) {
        if(m_size < size) {
            resize(glm::max(m_size << 1, size));
        }
    }

    /**
    Returns the size of the array.
    */
    inline size_t size() const {
        return m_size;
    }

    inline T& operator[](size_t index) {
        assert(index >= 0 && index < m_size);

        return m_data[index];
    }

    inline const T& operator[](size_t index) const {
        assert(index >= 0 && index < m_size);

        return m_data[index];
    }

private:
    T* m_data;
    size_t m_size;
};

#endif