#include <cassert>
#include "tests.h"
#include "Util/serial/Pool.h"
#include "Logging/logging.h"

#include <set>

void testPool() {
    {
        Pool<int> testManager;

        //test get Free
        size_t refA = testManager.getFreeId();
        assert(refA == 0);

        size_t refB = testManager.getFreeId();
        assert(refB == 1);

        size_t refC = testManager.getFreeId();
        assert(refC == 2);

        size_t refD = testManager.getFreeId();
        assert(refD == 3);

        //test init
        testManager.get(refA) = 69;
        testManager.get(refB) = 70;
        testManager.get(refC) = 71;
        testManager.get(refD) = 72;

        //test get
        assert(testManager.get(refA) == 69);
        assert(testManager.get(refB) == 70);
        assert(testManager.get(refC) == 71);
        assert(testManager.get(refD) == 72);

        //test get free again
        size_t refE = testManager.getFreeId();
        assert(refE == 4);

        size_t refF = testManager.getFreeId();
        assert(refF == 5);

        size_t refG = testManager.getFreeId();
        assert(refG == 6);

        //test set
        testManager.get(refE) = 73;
        testManager.get(refF) = 74;
        testManager.get(refG) = 75;

        //test get after replace
        assert(testManager.get(refE) == 73);
        assert(testManager.get(refF) == 74);
        assert(testManager.get(refG) == 75);

        //test remove
        testManager.remove(refB); 
        testManager.remove(refD);
        testManager.remove(refF);

        //test add and reserve id
        refF = testManager.getFreeId();
        assert(refF == 5);
        testManager.get(refF) = 76;

        refB = testManager.getFreeId();
        assert(refB == 3);
        testManager.get(refB) = 77;

        refD = testManager.getFreeId();
        assert(refD == 1);
        testManager.get(refD) = 78;

        //test all elements to make sure no corrpution
        assert(testManager.get(refA) == 69);
        assert(testManager.get(refB) == 77);
        assert(testManager.get(refC) == 71);
        assert(testManager.get(refD) == 78);
        assert(testManager.get(refE) == 73);
        assert(testManager.get(refF) == 76);
        assert(testManager.get(refG) == 75);

        //test reserve space
        testManager.reserveSpace(20);
    }


    {
        IterablePool<int> testManager;

        //test get Free
        size_t refA = testManager.getFreeId();
        assert(refA == 0);

        size_t refB = testManager.getFreeId();
        assert(refB == 1);

        size_t refC = testManager.getFreeId();
        assert(refC == 2);

        size_t refD = testManager.getFreeId();
        assert(refD == 3);

        //test init
        testManager.get(refA) = 69;
        testManager.get(refB) = 70;
        testManager.get(refC) = 71;
        testManager.get(refD) = 72;

        //test get
        assert(testManager.get(refA) == 69);
        assert(testManager.get(refB) == 70);
        assert(testManager.get(refC) == 71);
        assert(testManager.get(refD) == 72);

        //test get free again
        size_t refE = testManager.getFreeId();
        assert(refE == 4);

        size_t refF = testManager.getFreeId();
        assert(refF == 5);

        size_t refG = testManager.getFreeId();
        assert(refG == 6);

        //test set
        testManager.get(refE) = 73;
        testManager.get(refF) = 74;
        testManager.get(refG) = 75;

        //test get after replace
        assert(testManager.get(refE) == 73);
        assert(testManager.get(refF) == 74);
        assert(testManager.get(refG) == 75);

        //test remove
        testManager.remove(refB); 
        testManager.remove(refD);
        testManager.remove(refF);

        //test add and reserve id
        refF = testManager.getFreeId();
        assert(refF == 5);
        testManager.get(refF) = 76;

        refB = testManager.getFreeId();
        assert(refB == 3);
        testManager.get(refB) = 77;

        refD = testManager.getFreeId();
        assert(refD == 1);
        testManager.get(refD) = 78;

        //test all elements to make sure no corrpution
        assert(testManager.get(refA) == 69);
        assert(testManager.get(refB) == 77);
        assert(testManager.get(refC) == 71);
        assert(testManager.get(refD) == 78);
        assert(testManager.get(refE) == 73);
        assert(testManager.get(refF) == 76);
        assert(testManager.get(refG) == 75);

        //test reserve space
        testManager.reserveSpace(20);

        //test iterating
        {
            std::set<size_t> liveIdsTest;
            liveIdsTest.insert(refA);
            liveIdsTest.insert(refB);
            liveIdsTest.insert(refC);
            liveIdsTest.insert(refD);
            liveIdsTest.insert(refE);
            liveIdsTest.insert(refF);
            liveIdsTest.insert(refG);

            std::set<int> liveValuesTest;
            liveValuesTest.insert(69);
            liveValuesTest.insert(77);
            liveValuesTest.insert(71);
            liveValuesTest.insert(78);
            liveValuesTest.insert(73);
            liveValuesTest.insert(76);
            liveValuesTest.insert(75);

            for(IterablePool<int>::LiveSet::const_iterator iter = testManager.getLiveSet().begin(); iter != testManager.getLiveSet().end(); iter++) {
                size_t currId = *iter;
                int currVal = testManager.get(currId);

                assert(liveIdsTest.find(currId) != liveIdsTest.end());
                assert(liveValuesTest.find(currVal) != liveValuesTest.end());

                liveIdsTest.erase(currId);
                liveValuesTest.erase(currVal);
            }

            assert(liveIdsTest.empty());
            assert(liveValuesTest.empty());
        }
    }
}