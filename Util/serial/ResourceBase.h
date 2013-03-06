#ifndef ILL_RESOURCEBASE_H__
#define ILL_RESOURCEBASE_H__

#include <cstdlib>
#include <cstdint>

/**
Base class for a Resource like a texture or sound.
@tparam LoadArgs The type of struct that holds the loading arguments for the resource when passed into the load function.
@tparam Loader The backend loader object that is used for loading this resource type. (TODO: make a version that doesn't take a loader)
*/
template <typename LoadArgs, typename Loader>
class ResourceBase 
{
public:
   /**
   Be sure to call unload() in the destructor of the sub class since unload() can't be called from here.
   */
   virtual ~ResourceBase() {}

   /**
   Sets a resource's loading args and loads it.
   This can be used to take an existing resource and just load a different resource in its place.
   Subclasses should implement reload() not this method for the actual loading.
   */
   inline void load(const LoadArgs& loadArgs, Loader * loader) {
      m_loadArgs = loadArgs;

      reload(loader);
   }

   /**
   Returns a resource's loading args.
   */
   inline const LoadArgs& getLoadArgs() const {
      return m_loadArgs;
   }

   /**
   Reloads a resource based on its loading args.
   This is called either when the resource is loaded for the first time or after a subsystem this resource depends on is reinitialized.
   */
   virtual void reload(Loader * loader) = 0;

   /**
   Frees the resource.
   This might be called to temporarily unload a resource while a subsystem restarts, or when the resource is no longer needed.
   Be sure to call unload() in the destructor of the sub class since unload() can't be called from the base class destructor.
   */
   virtual void unload() = 0;
   
protected:
   /**
   Creates a resource in an uninitialized state
   */
   inline ResourceBase()
      : m_state(RES_UNINITIALIZED),
      m_loader(NULL)
   {}
   
   enum State {
      RES_UNINITIALIZED,      ///<The resource isn't ready for use of any sort.  load() hasn't been called yet so it doesn't have loadArgs set up yet.
      RES_UNLOADED,           ///<The resource had load() called and has it's loadArgs initialized.  It might have had unload() called on it.
      RES_LOADING,            ///<The resource is currently loading
      RES_LOADED              ///<The resource is loaded and ready for use
   };

   Loader * m_loader;
   State m_state;
   LoadArgs m_loadArgs;
};

#endif
