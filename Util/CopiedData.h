#ifndef ILL_COPIED_DATA_H_
#define ILL_COPIED_DATA_H_

#include <cstdlib>
#include <cstring>

/**
Data that copies around.  Very useful for sending data between threads.

If you know what you are doing this is a good wrapper around straight void*.
You need to be careful with this since it's not reference counted or anything.
If the destructor is called, the data will be deleted.

For example if passing this into a function, the data pointed to inside will be deleted
after the function returns even if there are other pointers to this data.
This may be useful if you just want to send the data and forget about it, but very
dangerous otherwise.

There are things like smart pointers and RefCountPtr to avoid this.
It's a bit hard to use void* with those in some situations though so that's why this exists.
And yes, there are situations when you need void*.
*/
struct CopiedData {
   CopiedData()
      : m_data(NULL),
      m_size(0)
   {}

   /*
   Actually nvm I don't want a copy constructor, There were some issues
   CopiedData(const CopiedData& other)
      : m_data(NULL)
   {
      setData(other.m_data, other.m_size);
   }*/

   explicit CopiedData(size_t size)
      : m_data(NULL) 
   {
      allocateData(size);
   }

   CopiedData(void * sourceData, size_t size)
      : m_data(NULL)
   {
      setData(sourceData, size);
   }

   ~CopiedData() {
      free(m_data);
   }

   inline void allocateData(size_t size) {
      m_size = size;
      m_data = realloc(m_data, size);
   }

   inline void setData(void * sourceData, size_t size) {
      allocateData(size);
      memcpy(m_data, sourceData, size);
   }

   inline void setData(const CopiedData& other) {
      setData(other.m_data, other.m_size);
   }

   void * m_data;
   size_t m_size;
};

#endif