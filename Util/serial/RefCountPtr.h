#ifndef ILL_REF_COUNT_PTR_H__
#define ILL_REF_COUNT_PTR_H__

#include <cassert>
#include "Util/casting.h"

template <typename T>
struct RefCountPtr;

template <typename T>
struct RefCountPtrRoot;

/**
This is the object that has callbacks to what should happen in certain events.
*/
template <typename T>
struct PtrHelper {
    PtrHelper()
        : m_root(NULL)
    {}

    virtual ~PtrHelper() {}

    /**
    What happens when there are no more references to the smart pointer.
    */
    virtual inline void onZeroReferences() {
        delete m_root;
    }

    /**
    What happens when there are references to the smart pointer again.
    */
    virtual inline void onNonZeroReferences() {}

    RefCountPtrRoot<T> * m_root;
};

/**
This is the behind the scenes struct that holds the actual value of a pointer that the Pointers linked to reference.
*/
template <typename T>
struct RefCountPtrRoot {
    /**
    Usually a smart pointer automatically creates this behind the scenes.
    If you are doing some custom pointer stuff, you may need to create this yourself
    and feed it into the pointer.

    @param pointer The raw C pointer to the object.
    @param initReferences If true initializes the references to 1 instead of 0.
        This can help avoid some unnecessary overhead when creating the pointer for the first time
        by not allowing the onNonZeroReferences() event get called.
        Only set this to true if the pointer really is about to have 1 reference after creation,
        otherwise leave this to false.
    @param ptrHelper The struct with some callbacks for what happens when the pointer
        now has zero references and when it no longer has zero references.
        The default helper just deletes the object.
    */
    inline RefCountPtrRoot(T* pointer, bool initReferences = true, PtrHelper<T>* ptrHelper = new PtrHelper<T>())
        : m_pointer(pointer),
        m_helper(ptrHelper)
    {
        m_references = initReferences ? 1 : 0;
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

        assert(m_references > 0);   //might help catch cases when there's rollover
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
    /**
    Creates an empty null pointer.
    This makes it safe to just create these guys statically.
    */
    inline RefCountPtr()
        : m_root(NULL)
    {}

    /**
    Copy constructor.
    This causes a reference increment on the pointer itself.
    */
    inline RefCountPtr(const RefCountPtr &other)
        : m_root(other.m_root)
    {
        referenceIncrement();
    }

    /**
    Creates a new smart pointer that points to a raw C pointer.
    */
    inline explicit RefCountPtr(T * pointer) {
        m_root = new RefCountPtrRoot<T>(pointer, true);
    }

    /**
    Creates a new smart pointer from a reference.
    */
    inline explicit RefCountPtr(T& pointer) {
        m_root = new RefCountPtrRoot<T>(&pointer, true);
    }

    /**
    Creates a new smart pointer given an existing behind the scenes pointer root.
    This will also increment the reference.
    */
    template<typename R_T>
    inline explicit RefCountPtr(RefCountPtrRoot<R_T> * pointerRoot) {
        m_root = (RefCountPtrRoot<T> *) pointerRoot;
        referenceIncrement();
    }

    /**
    Creates a new smart pointer given an existing behind the scenes pointer root.
    This gives you the option to not increment the reference if the root was
    created with initReferences set to true and this is the first reference.
    */
    template<typename R_T>
    inline explicit RefCountPtr(RefCountPtrRoot<R_T> * pointerRoot, bool refIncrement) {
        m_root = (RefCountPtrRoot<T> *) pointerRoot;

        if(refIncrement) {
            referenceIncrement();
        }
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
        return m_root ? m_root->m_pointer : NULL;
    }

    template<typename C>
    inline bool is() const {
        return m_root && m_root->m_pointer ? ::is<C>(m_root->m_pointer) : false;
    }

    template<typename C>
    inline RefCountPtr<C> as() const {
        return RefCountPtr<C>(m_root);
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
