#ifndef __LRU_CACHE_H__
#define __LRU_CACHE_H__

#include <map>
#include <list>

#include "RefCountPtr.h"

/**
http://timday.bitbucket.org/lru.html
*/

struct NONE {};      //TODO: put this in util?

template <typename Key, typename T, typename CreateFunc = T *(*)(Key), typename Creator = NONE>
class LruCache {
private:
   struct ElementInfo;
   struct LruPtrHelper;

public:
   typedef RefCountPtrRoot<T> HandleRoot;

private:
   typedef std::map<Key, ElementInfo> ElementsMap;      //TODO: use an unordred map so it uses hashing instead
   //typedef std::list<tbb::concurrent_hash_map<Key, ElementInfo<Key, T> >::accessor> UseOrderList;

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

private:
   struct ElementInfo {

      ElementInfo() 
         : m_element(NULL),
         m_timeFreed(0.0)
      {}

      ~ElementInfo() {
         delete m_element;
      }

      HandleRoot * m_element;
      double m_timeFreed;
      //UseOrderList::iterator m_useOrderIter;
   };

public:
   LruCache(CreateFunc createFunc, size_t bucketSize = 20)
      : m_createFunc(createFunc),
      m_creator(NULL)
   {
      m_elements = ElementsMap(bucketSize);
   }

   LruCache(CreateFunc createFunc, Creator * creator)
      : m_createFunc(createFunc),
      m_creator(creator)
   {
      m_elements = ElementsMap();
   }

   inline RefCountPtr<T> getElement(Key key) {
      typename ElementsMap::iterator iter = m_elements.find(key);

      //if element in cache, return it
      if(iter != m_elements.end()) {
         return RefCountPtr<T>(iter->second.m_element);
      }

      //load the element
      evictElements();      

      LruPtrHelper * helper = new LruPtrHelper();
      helper->m_key = key;
      helper->m_cache = this;

      RefCountPtrRoot<T> * element = createElement(key, helper);

      m_elements[key].m_element = element;

      return RefCountPtr<T>(element);
   }
   
   inline void clean() {
      //TODO:
   }

private:
   inline RefCountPtrRoot<T> * createElement(Key key, LruPtrHelper* helper) {
      return new RefCountPtrRoot<T>((m_creator->*m_createFunc)(key), helper);
   }

   void releaseElement(Key key) {
      //TODO: track that the element is no longer being used
   }

   void unreleaseElement(Key key) {
      //TODO: track that the element is now used again
   }

   void evictElements() {
      //TODO: evict unused elements after a certain time
   }

   CreateFunc m_createFunc;
   Creator * m_creator;

   ElementsMap m_elements;
   //UseOrderList m_useOrder;         //TODO: replace with a concurrent sorted linked list, sorted by eviction time
};

//TODO: figure out how to do this with template specialization
/*template <typename Key, typename T, typename CreateFunc>
RefCountPtrRoot<T, typename LruCache<Key, T, CreateFunc, NONE>::LruPtrHelper> *  
LruCache<Key, T, CreateFunc, NONE>::createElement(Key key, LruPtrHelper& helper) {
   return new RefCountPtrRoot<T, LruPtrHelper>((*m_createFunc)(key), helper);
}*/



//TODO: this may be unneeded
/*template <typename Key, typename T, typename CreateFunc, typename Creator>
RefCountPtrRoot<T, typename LruCache<Key, T, CreateFunc, Creator>::LruPtrHelper> *  
LruCache<Key, T, CreateFunc, Creator>::createElement(Key key, LruPtrHelper& helper) {
   return new RefCountPtrRoot<T, LruPtrHelper>((m_creator->*m_createFunc)(key), helper);
}*/

#endif
