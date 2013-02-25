#include <cassert>
#include "Util/serial/VectorManager.h"
#include "tests.h"

void singleThreadedTest() {
   VectorManager<int> testManager;

   //test add
   size_t refA = testManager.add(69);
   assert(refA == 0);

   size_t refB = testManager.add(70);
   assert(refB == 1);

   size_t refC = testManager.add(71);
   assert(refC == 2);

   size_t refD = testManager.add(72);
   assert(refD == 3);

   //test get
   assert(testManager.get(refA) == 69);
   assert(testManager.get(refB) == 70);
   assert(testManager.get(refC) == 71);
   assert(testManager.get(refD) == 72);

   //test reserveID
   size_t refE = testManager.reserveId();
   assert(refE == 4);

   size_t refF = testManager.reserveId();
   assert(refF == 5);

   size_t refG = testManager.reserveId();
   assert(refG == 6);

   //test replace
   testManager.replace(refE, 73);
   testManager.replace(refF, 74);
   testManager.replace(refG, 75);

   //test get after replace
   assert(testManager.get(refE) == 73);
   assert(testManager.get(refF) == 74);
   assert(testManager.get(refG) == 75);

   //test remove
   testManager.remove(refB); 
   testManager.remove(refD);
   testManager.remove(refF);

   //test add and reserve id
   refF = testManager.add(76);
   assert(refF == 1);

   refB = testManager.add(77);
   assert(refB == 3);

   refD = testManager.reserveId();
   assert(refD == 5);
   testManager.replace(refD, 78);
   
   //test all elements to make sure no corrpution
   assert(testManager.get(refA) == 69);
   assert(testManager.get(refB) == 77);
   assert(testManager.get(refC) == 71);
   assert(testManager.get(refD) == 78);
   assert(testManager.get(refE) == 73);
   assert(testManager.get(refF) == 76);
   assert(testManager.get(refG) == 75);
}

void concurrentTest() {
   
}

void testVectorManager() {
   //singleThreadedTest();
   //concurrentTest();
}
