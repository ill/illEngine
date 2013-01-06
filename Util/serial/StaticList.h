#ifndef ILL_STATIC_LIST_H_
#define ILL_STATIC_LIST_H_

#include <cassert>

/**
A very lightweight and simple version of vector for use in internal things that require high performance
and don't require any of the crazyness.

This is statically allocated and can't be resized.

@tparam T The type of elements in the list.
@tparam S The statically allocated size of the list.
@tparam ST The type of the size variable.  The type pretty much limits
   the maximum elements in the list.  If passing in an S below 256, just use the default unsigned byte.
   If you need more than 256 elements use the next highest type which would normally be an unsigned short allowing 65536 elements.
*/
template<typename T, unsigned int S, typename ST = uint8_t>
struct StaticList {
   inline StaticList()
      : m_size(0)
   {}
   
   /**
   Adds an element to a list.
   */
   inline void add(const T& element) {
      //TODO: write own assert methods
      assert(m_size < S);

      m_data[m_size++] = element;
   }

   /**
   Clears the list back to size 0.
   */
   inline void clear() {
      m_size = 0;
   }

   ST m_size;
   T m_data[S];
};

#endif //ILL_STATIC_LIST_H_