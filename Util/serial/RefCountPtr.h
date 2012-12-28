#ifndef __REF_COUNT_PTR_H__
#define __REF_COUNT_PTR_H__

#include <cassert>

template <typename T>
struct RefCountPtr;

template <typename T>
struct RefCountPtrRoot;

/**
*/
template <typename T>
struct PtrHelper {
    PtrHelper() {}
    virtual ~PtrHelper() {}

    virtual inline void onZeroReferences() {
        delete m_root;
    }

    virtual inline void onNonZeroReferences() {}

    RefCountPtrRoot<T> * m_root;
};

/**
This is the behind the scenes struct that holds the actual value of a pointer that the Pointers linked to reference.
*/
template <typename T>
struct RefCountPtrRoot {
    inline RefCountPtrRoot(T* pointer, PtrHelper<T>* ptrHelper = new PtrHelper<T>())
        : m_references(0),
        m_pointer(pointer),
        m_helper(ptrHelper)
    {
        m_helper->m_root = this;
    }

    inline ~RefCountPtrRoot() {
        deleterFunc(m_pointer);
        delete m_helper;
    }

    inline void referenceIncrement() {
        //if there were previously no references but there are now
        if(m_references == 0) {
            m_helper->onNonZeroReferences();
        }

        m_references++;
    }

    inline void referenceDecrement() {
        assert(m_references > 0);

        //once all references are gone do something
        if(--m_references == 0) {
            m_helper->onZeroReferences();
        }
    }

    inline static void deleterFunc(T* pointer) {
        delete pointer;
    }

    unsigned int m_references;
    T* m_pointer;
    PtrHelper<T>* m_helper;
};

/**
The actual pointer
*/
template <typename T>
struct RefCountPtr {
    inline RefCountPtr()
        : m_root(NULL)
    {}

    inline RefCountPtr(const RefCountPtr &other)
        : m_root(other.m_root)
    {
        referenceIncrement();
    }

    inline explicit RefCountPtr(T * pointer) {
        m_root = new RefCountPtrRoot<T>(pointer);
        referenceIncrement();
    }

    inline explicit RefCountPtr(T& pointer) {
        m_root = new RefCountPtrRoot<T>(&pointer);
        referenceIncrement();
    }

    template<typename R_T>
    inline explicit RefCountPtr(RefCountPtrRoot<R_T> * pointerRoot) {
        m_root = (RefCountPtrRoot<T> *) pointerRoot;
        referenceIncrement();
    }

    inline ~RefCountPtr() {
        referenceDecrement();
    }

    inline RefCountPtr & operator=(const RefCountPtr & rhs) {
        if(*this != rhs) {
            referenceDecrement();

            m_root = rhs.m_root;

            referenceIncrement();
        }

        return *this;
    }

    inline T * operator->() const {
        //this doesn't need to be safe and check if root is null
        //if you're trying to dereference a null pointer you're already screwed on the calling end anyway
        return m_root->m_pointer;
    }

    inline T & operator*() const {
        //this doesn't need to be safe and check if root is null
        //if you're trying to dereference a null pointer you're already screwed on the calling end anyway
        return *(m_root->m_pointer);
    }

    inline bool operator==(const RefCountPtr & other) const {
        return /*(this == &other) ||*/ (m_root == other.m_root);
    }

    inline bool operator!=(const RefCountPtr & other) const {
        return !(*this == other);
    }

    /**
    *	Deletes what the pointer is pointing to and sets it to NULL
    *	To check if a pointer is pointing to an actual object check if it's pointing to NULL
    */
    inline void erase() {
        if(m_root) {
            delete m_root->m_pointer;
            m_root->m_pointer = NULL;
        }
    }

    /**
    * Makes the pointer point to nothing and decrements the references to whatever it was referencing
    */
    inline void reset() {
        referenceDecrement();
        m_root = NULL;
    }

    /**
    * 	Returns whether or not the pointer is pointing to null
    */
    inline bool isNull() const {
        return !m_root || !m_root->m_pointer;
    }

    /**
    * Returns the pointer this thing points to.  Normally avoid doing this, but
    * sometimes it's better to have the raw pointer for performance related things...
    */
    inline T * get() const {
        return m_root ? m_root->m_pointer.load() : NULL;
    }

    template<typename C>
    inline bool is() const {
        return m_root && m_root->m_pointer ? ::is<C>(m_root->m_pointer) : false;
    }

    template<typename C>
    inline RefCountPtr<C> as() const {
        return RefCountPtr<C>(m_root);
    }

    template<typename C>
    inline RefCountPtr<C> safeAs() const {
        if(!is<C>()) {
            //LOG_FATAL_ERROR("Cannot cast %s to %s", typeid(T).name(), typeid(C).name());

            //TODO: exception
        }

        return as<C>();
    }   

private:
    inline void referenceIncrement() {
        if(m_root) {
            m_root->referenceIncrement();
        }
    }

    inline void referenceDecrement() {
        if(m_root) {
            m_root->referenceDecrement();
        }
    }

    RefCountPtrRoot<T> * m_root;

    friend RefCountPtrRoot<T>;
};

#endif