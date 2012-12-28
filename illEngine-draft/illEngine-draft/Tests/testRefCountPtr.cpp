#include <SDL_assert.h>

#include "serial-illLogging/logging.h"
#include "serial-illUtil/RefCountPtr.h"

#include "tests.h"

//not exactly the best written tests right now, relies on you manually putting breakpoints and debugging, it works though and helped me fix an awful bug

template <typename T>
struct TestPtrHelper : public PtrHelper<T> {
   TestPtrHelper()
      : PtrHelper<T>()
   {}

   virtual ~TestPtrHelper() {}

   virtual inline void onZeroReferences() {
      LOG_INFO("On Zero References");
      SDL_TriggerBreakpoint();

      PtrHelper::onZeroReferences();
   }

   virtual inline void onNonZeroReferences() {
      LOG_INFO("On Non Zero References");
      SDL_TriggerBreakpoint();

      PtrHelper::onNonZeroReferences();
   }
};

void testRefCountPtr() {
   LOG_INFO("About to test RefCountPtr");

   {
      LOG_INFO("Simple Test");

      int * testInt = new int(5);

      RefCountPtr<int> testPtr(testInt);

      LOG_INFO("Should be allocated with 1 reference");
      SDL_TriggerBreakpoint();
   }

   LOG_INFO("Should be freed");
   SDL_TriggerBreakpoint();

   {
      LOG_INFO("Multi reference Test");

      int * testInt = new int(10);

      RefCountPtr<int> testPtr(testInt);
      
      RefCountPtr<int> testPtr2 = testPtr;

      LOG_INFO("Should have 2 references");
      SDL_TriggerBreakpoint();

      testPtr.reset();

      LOG_INFO("Should have 1 references");
      SDL_TriggerBreakpoint();

      testPtr = testPtr2;

      LOG_INFO("Should have 2 references");
      SDL_TriggerBreakpoint();
   }

   LOG_INFO("Should be freed");
   SDL_TriggerBreakpoint();

   {
      LOG_INFO("PtrRoot Test");

      int * testInt = new int(20);
      RefCountPtrRoot<int>* testRoot = new RefCountPtrRoot<int>(testInt, new TestPtrHelper<int>());

      int * testInt2 = new int(40);
      RefCountPtrRoot<int>* testRoot2 = new RefCountPtrRoot<int>(testInt2, new TestPtrHelper<int>());

      RefCountPtr<int> testPtr(testRoot);

      LOG_INFO("Should have 1 references");
      SDL_TriggerBreakpoint();

      RefCountPtr<int> testPtr2 = testPtr;

      LOG_INFO("Should have 2 references");
      SDL_TriggerBreakpoint();

      testPtr = RefCountPtr<int>(testRoot2);

      LOG_INFO("Should have 1 references");
      SDL_TriggerBreakpoint();

      testPtr.reset();

      LOG_INFO("Should have 0 references");
      SDL_TriggerBreakpoint();

      testPtr2.reset();

      LOG_INFO("Should have 0 references");
      SDL_TriggerBreakpoint();
   }

   LOG_INFO("Should be freed");
   SDL_TriggerBreakpoint();
}