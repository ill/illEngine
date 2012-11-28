#ifndef __FRUSTUM_ITERATOR_H__
#define __FRUSTUM_ITERATOR_H__

#include <list>
#include <stack>
#include "GridVolume3D.h"
#include "Frustum.h"
#include "LineIterator.h"
#include "geomUtil.h"

/**
Traverses front to back in the view frustum that intersects a GridVolume3D.
The frustum should have a field of view less than 180 degrees or else assumptions made about
the traversal will be wrong.

@param T The precision of the GridVolume3D that this iterator is for
*/
template <typename T = glm::mediump_float>
class FrustumIterator {
private:
    struct CellTraverser {
        ///the current plane
        enum Plane {
            PL_XY,
            PL_XZ,
            PL_YZ
        };

        ///the dimension
        enum Dimension {
            DM_X = 0,
            DM_Y = 1,
            DM_Z = 2
        };

        ///the direction
        enum Direction {
            DR_NEG = -1,
            DR_NONE = 0,
            DR_POS = 1
        };

        typedef uint8_t State;

        struct DirectionInfo {
            DirectionInfo(Direction direction = DR_NONE, Dimension dimension = DM_X, bool reflect = false, unsigned int distBeforeCorner = 0)
                : m_direction(direction),
                m_dimension(dimension),
                m_reflect(reflect),
                m_distBeforeCorner(distBeforeCorner)
            {}

            ///-1, 0, or 1 depending on direction
            Direction m_direction;

            Dimension m_dimension;

            bool m_reflect;

            ///Countdown of traversal before hitting a corner
            unsigned int m_distBeforeCorner;
        };

        typedef glm::detail::tvec3<DirectionInfo> DirectionVec;

        CellTraverser(FrustumIterator * iterator, 
            const glm::ivec3& cellIndex, 

            const DirectionInfo& primaryDirection,
            const DirectionInfo& secondaryDirection,

            Direction reflectionDirection, 
            Dimension reflectionDimension)

            : m_iterator(iterator),
            m_cellIndex(cellIndex),

            m_reflectionDirection(reflectionDirection),
            m_reflectionDimension(reflectionDimension)
        {
            m_direction[0] = primaryDirection;
            m_direction[1] = secondaryDirection;
        }

        ///The frustum iterator this is a part of
        FrustumIterator * m_iterator;

        ///The current cell
        glm::ivec3 m_cellIndex;

        DirectionInfo m_direction[2];

        Direction m_reflectionDirection;
        Dimension m_reflectionDimension;
    };

public:
    struct Debugger {
        Debugger(FrustumIterator * iterator)
            : m_iterator(iterator),
            m_centerIndex(0),
            m_frustum(*iterator->m_frustum)
        {}

        struct Traversal {
            Traversal() {}

            Traversal(const glm::ivec3& start, const glm::ivec3& end)
                : m_start(start),
                m_end(end)
            {}

            glm::ivec3 m_start;
            glm::ivec3 m_end;
        };

        const glm::ivec3& getLineStart() const {
            return m_iterator->m_lineIterator.getStart();
        }

        const glm::ivec3& getLineEnd() const {
            return m_iterator->m_lineIterator.getEnd();
        }

        glm::ivec3 m_centerIndex;
        std::list<Traversal> m_traversals;

        FrustumIterator * m_iterator;

        Frustum<T> m_frustum;
    };

    FrustumIterator()
        : m_frustum(NULL),
        m_atEnd(true),
        m_chessDistance(0),
        m_debugger(NULL)
    {}

    FrustumIterator(const Frustum<T>* frustum, const glm::detail::tvec3<T>& cellDimensions, const glm::uvec3& cellNumber, bool createDebugger = false)
        : m_frustum(frustum),
        m_cellNumber(cellNumber),
        m_atEnd(false),
        m_chessDistance(0),
        m_debugger(NULL)
    {
        if(createDebugger) {
            m_debugger = new Debugger(this);
        }

        //create a line iterator starting from frustum near plane to far tip point
        m_lineIterator = LineIterator<int>(vec3cast<glm::mediump_float, int>(glm::floor(m_frustum->m_nearPoint / cellDimensions)),
            vec3cast<glm::mediump_float, int>(glm::floor(m_frustum->m_farTipPoint / cellDimensions)));

        m_lineVector = m_lineIterator.getEnd() - m_lineIterator.getStart();

        //if current point outside volume bounds, iterate line until reached chess metric distance to volume bounds
        Box<int> indexBounds(glm::ivec3(0), vec3cast<unsigned int, int>(m_cellNumber) - glm::ivec3(1));

        for(unsigned int lineAdvance = indexBounds.getChessDistance(m_lineIterator.getCurrentPosition()); lineAdvance > 0; lineAdvance--) {
            m_chessDistance++;
            if(!m_lineIterator.forward()) {
                m_atEnd = true;
                break;
            }
        }

        m_nextCell = m_lineIterator.getCurrentPosition();

        forward();
    }

    ~FrustumIterator() {
        delete m_debugger;
    }

    Debugger * getDebugger() {
        return m_debugger;
    }

    const glm::ivec3& getCurrentCell()  {
        return m_currentCell;
    }

    inline bool atEnd() const {
        return m_cellTraverseStack.empty() && m_atEnd;
    }

    inline const Frustum<T>* getFrustum() const {
        return m_frustum;
    }

    bool forward() {
        while(true) {
            if(atEnd()) {
                return false;
            }

            m_currentCell = m_nextCell;

            //seed the traversal if no cell traversers
            if(m_cellTraverseStack.empty()) {
                if(m_debugger) {
                    m_debugger->m_traversals.clear();
                }

                if(m_lineIterator.atEnd()) {
                    m_atEnd = true;
                    return false;
                }

                glm::ivec3 absDist = glm::abs(m_lineIterator.getCurrentPosition() - m_lineIterator.getStart());

                if(absDist.x > absDist.y && absDist.x > absDist.z) {   //yz plane
                    seedPlane(CellTraverser::PL_YZ);
                }
                else if(absDist.y > absDist.x && absDist.y > absDist.z) {   //xz plane
                    seedPlane(CellTraverser::PL_XZ);
                }
                else if(absDist.z > absDist.x && absDist.z > absDist.y) {   //xy plane
                    seedPlane(CellTraverser::PL_XY);
                }
                else if(absDist.x == 0 && absDist.y == 0 && absDist.z == 0) { //at first cell
                    m_cellTraverseStack.push(CellTraverser(this, m_lineIterator.getCurrentPosition(),
                        CellTraverser::DirectionInfo(),
                        CellTraverser::DirectionInfo(),
                        CellTraverser::DR_NONE, CellTraverser::DM_X));
                }
                else if(absDist.x == absDist.y && absDist.x == absDist.z) { //xy, xz, and yz
                    seedCorner();
                }
                else if(absDist.x == absDist.z) {   //xy and yz
                    seed2Planes(CellTraverser::PL_XY, CellTraverser::PL_YZ);
                }
                else if(absDist.y == absDist.z) {   //xy and xz
                    seed2Planes(CellTraverser::PL_XY, CellTraverser::PL_XZ);
                }
                else if(absDist.x == absDist.y) {   //xz and yz
                    seed2Planes(CellTraverser::PL_XZ, CellTraverser::PL_YZ);
                }

                if(m_debugger) {
                    m_debugger->m_centerIndex = m_lineIterator.getCurrentPosition();
                }

                m_lineIterator.forward();
                m_chessDistance ++;

                m_nextCell = m_lineIterator.getCurrentPosition();

                return true;
            }
            else {   //advance the traverser at the top of the stack
                //if the traverser is outside of the 3d volume, it'll skip around a bit until reaching the volume or dying if determined to never intersect            
                CellTraverser& traverser = m_cellTraverseStack.top();

                //if a direction is at the edge
                bool atEdge[2];
                bool isDiagonal = traverser.m_direction[0].m_direction != CellTraverser::DR_NONE && traverser.m_direction[1].m_direction != CellTraverser::DR_NONE;

                //if currently outside of the volume being iterated over
                //bool isOutside;

                {
                    Box<int> indexBounds(glm::ivec3(0), vec3cast<unsigned int, int>(m_cellNumber) - glm::ivec3(1));

                    //isOutside = !indexBounds.intersects(traverser.m_cellIndex);
                }

                //assign at edge and movingOutside
                for(unsigned int direction = 0; direction < 2; direction++) {
                    if(traverser.m_direction[direction].m_direction == CellTraverser::DR_NONE) {
                        continue;
                    }

                    atEdge[direction] = traverser.m_direction[direction].m_distBeforeCorner == 0;
                }

                //if the current traverser is outside of the volume and the reflection will reflect outside, die

                //TODO: figure this out and make it work for sure
                /*if(isOutside 
                && ((traverser.m_cellIndex[traverser.m_reflectionDimension] < 0 && traverser.m_reflectionDirection == CellTraverser::DR_NEG)
                || (traverser.m_cellIndex[traverser.m_reflectionDimension] >= m_cellNumber[traverser.m_reflectionDimension] && traverser.m_reflectionDirection == CellTraverser::DR_POS))) {
                traverser.m_direction[direction].m_direction = CellTraverser::DR_NONE;
                continue;      //kill and continue with outside loop
                }*/

                //TODO: kill a direction if it won't be inside the volume by reflection time, 

                //check if direction needs to be killed
                for(unsigned int direction = 0; direction < 2; direction++) {
                    if(traverser.m_direction[direction].m_direction == CellTraverser::DR_NONE) {
                        continue;
                    }

                    if(atEdge[direction]) {
                        //check if needs to reflect
                        if(traverser.m_direction[direction].m_reflect) {
                            //if was traversing a corner, kill
                            if(traverser.m_reflectionDirection == CellTraverser::DR_NONE) {
                                traverser.m_direction[direction].m_direction = CellTraverser::DR_NONE;
                                continue;
                            }

                            traverser.m_direction[direction].m_direction = traverser.m_reflectionDirection;
                            traverser.m_direction[direction].m_dimension = traverser.m_reflectionDimension;
                            traverser.m_direction[direction].m_distBeforeCorner = (m_chessDistance << 1) - 2;
                            traverser.m_direction[direction].m_reflect = false;

                            //if a diagonal
                            if(isDiagonal) {
                                //if hit a corner, kill other direction
                                if(atEdge[!direction]) {
                                    isDiagonal = false;
                                    traverser.m_direction[!direction].m_direction = CellTraverser::DR_NONE;
                                }
                                //force no reflections on other direction
                                else {
                                    traverser.m_direction[!direction].m_reflect = false;
                                }
                            }
                        }
                        //check if needs to die
                        else {
                            //if no more reflections, done
                            traverser.m_direction[direction].m_direction = CellTraverser::DR_NONE;

                            //if a diagonal, force no more reflections
                            if(isDiagonal) {
                                isDiagonal = false;
                                traverser.m_reflectionDirection = CellTraverser::DR_NONE;
                            }

                            continue;
                        }
                    }

                    /*if(isOutside) {
                    //if the current traverser is outside of the volume being iterated and heading away from it, die
                    if((traverser.m_cellIndex[traverser.m_direction[direction].m_dimension] < 0 && traverser.m_direction[direction].m_direction == CellTraverser::DR_NEG)
                    || (traverser.m_cellIndex[traverser.m_direction[direction].m_dimension] >= m_cellNumber[traverser.m_direction[direction].m_dimension] && traverser.m_direction[direction].m_direction == CellTraverser::DR_POS)) {
                    traverser.m_direction[direction].m_direction = CellTraverser::DR_NONE;
                    continue;
                    }
                    }*/
                }

                //kill traverser if needed
                if(traverser.m_direction[0].m_direction == CellTraverser::DR_NONE && traverser.m_direction[1].m_direction == CellTraverser::DR_NONE) {
                    m_cellTraverseStack.pop();
                    continue;   //continues outer loop
                }

                //spawn more if diagonal
                if(isDiagonal) {
                    for(unsigned int direction = 0; direction < 2; direction++) {
                        if(traverser.m_direction[direction].m_direction == CellTraverser::DR_NONE) {
                            continue;
                        }

                        glm::ivec3 creationPosition = traverser.m_cellIndex;
                        creationPosition[traverser.m_direction[direction].m_dimension] += traverser.m_direction[direction].m_direction;

                        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
                            traverser.m_direction[direction],
                            CellTraverser::DirectionInfo(),
                            traverser.m_reflectionDirection, traverser.m_reflectionDimension));

                        m_cellTraverseStack.top().m_direction[0].m_distBeforeCorner--;

                        //force to not reflect if spawned onto a corner, TODO: this needs work
                        if(atEdge[!direction]) {
                            m_cellTraverseStack.top().m_direction[0].m_reflect = false;
                        }

                        if(m_debugger) {
                            m_debugger->m_traversals.push_back(Debugger::Traversal(traverser.m_cellIndex, creationPosition));
                        }
                    }
                }

                //move
                glm::ivec3 debugPos = traverser.m_cellIndex;

                for(unsigned int direction = 0; direction < 2; direction++) {
                    if(traverser.m_direction[direction].m_direction == CellTraverser::DR_NONE) {
                        continue;
                    }

                    int moveAmount = 1;

                    traverser.m_cellIndex[traverser.m_direction[direction].m_dimension] += traverser.m_direction[direction].m_direction;
                    traverser.m_direction[direction].m_distBeforeCorner--;
                }

                if(m_debugger) {
                    m_debugger->m_traversals.push_back(Debugger::Traversal(debugPos, traverser.m_cellIndex));
                }

                //if is outside, keep going
                /*Box<int> indexBounds(glm::ivec3(0), vec3cast<unsigned int, int>(m_cellNumber) - glm::ivec3(1));

                if(!indexBounds.intersects(traverser.m_cellIndex)) {
                continue;   //continues outer loop
                }*/

                m_nextCell = traverser.m_cellIndex;

                return true;
            }
        }
    }

private:
    //for the case when the seed cell/voxel is on a plane
    void seedPlane(typename CellTraverser::Plane plane) {
        //figure out directions and dimensions
        CellTraverser::Dimension primaryDimension;
        CellTraverser::Dimension secondaryDimension;

        CellTraverser::Dimension reflectionDimension;
        CellTraverser::Direction reflectionDirection;

        switch(plane) {
        case CellTraverser::PL_XY:
            primaryDimension = m_lineVector.y > m_lineVector.x ? CellTraverser::DM_Y : CellTraverser::DM_X;
            secondaryDimension = m_lineVector.y > m_lineVector.x ? CellTraverser::DM_X : CellTraverser::DM_Y;
            reflectionDimension = CellTraverser::DM_Z;
            break;

        case CellTraverser::PL_XZ:
            primaryDimension = m_lineVector.z > m_lineVector.x ? CellTraverser::DM_Z : CellTraverser::DM_X;
            secondaryDimension = m_lineVector.z > m_lineVector.x ? CellTraverser::DM_X : CellTraverser::DM_Z;
            reflectionDimension = CellTraverser::DM_Y;
            break;

        case CellTraverser::PL_YZ:
            primaryDimension = m_lineVector.z > m_lineVector.y ? CellTraverser::DM_Z : CellTraverser::DM_Y;
            secondaryDimension = m_lineVector.z > m_lineVector.y ? CellTraverser::DM_Y : CellTraverser::DM_Z;
            reflectionDimension = CellTraverser::DM_X;
            break;
        }

        reflectionDirection = (CellTraverser::Direction) -glm::sign(m_lineVector[reflectionDimension]);

        //primary forward     
        glm::ivec3 creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] += signO(m_lineVector[primaryDimension]);

        CellTraverser::DirectionInfo primaryDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) signO(m_lineVector[primaryDimension]), primaryDimension,
            true,
            glm::abs(creationPosition[primaryDimension] - (m_lineIterator.getStart()[primaryDimension] + m_chessDistance * signO(m_lineVector[primaryDimension]))));

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            CellTraverser::DirectionInfo(),
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary reverse
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);

        CellTraverser::DirectionInfo primaryReverse = CellTraverser::DirectionInfo((CellTraverser::Direction) -signO(m_lineVector[primaryDimension]), primaryDimension,
            true,
            glm::abs(creationPosition[primaryDimension] - (m_lineIterator.getStart()[primaryDimension] - m_chessDistance * signO(m_lineVector[primaryDimension]))));

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryReverse,
            CellTraverser::DirectionInfo(),
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary forward
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] += signO(m_lineVector[secondaryDimension]);

        CellTraverser::DirectionInfo secondaryDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) signO(m_lineVector[secondaryDimension]), secondaryDimension,
            true,
            glm::abs(creationPosition[secondaryDimension] - (m_lineIterator.getStart()[secondaryDimension] + m_chessDistance * signO(m_lineVector[secondaryDimension]))));

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryDirection,
            CellTraverser::DirectionInfo(),
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary reverse
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);

        CellTraverser::DirectionInfo secondaryReverse = CellTraverser::DirectionInfo((CellTraverser::Direction) -signO(m_lineVector[secondaryDimension]), secondaryDimension,
            true,
            glm::abs(creationPosition[secondaryDimension] - (m_lineIterator.getStart()[secondaryDimension] - m_chessDistance * signO(m_lineVector[secondaryDimension]))));

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryReverse,
            CellTraverser::DirectionInfo(),
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary forward secondary forward
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] += signO(m_lineVector[primaryDimension]);
        creationPosition[secondaryDimension] += signO(m_lineVector[secondaryDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            secondaryDirection,
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary forward secondary reverse
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] += signO(m_lineVector[primaryDimension]);
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            secondaryReverse,
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary reverse secondary forward
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);
        creationPosition[secondaryDimension] += signO(m_lineVector[secondaryDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryReverse,
            secondaryDirection,
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary reverse secondary reverse
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryReverse,
            secondaryReverse,
            reflectionDirection, reflectionDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }
    }

    //for the case when you're at the corner of 2 planes
    void seed2Planes(typename CellTraverser::Plane plane1, typename CellTraverser::Plane plane2) {
        //figure out directions and dimensions
        CellTraverser::Dimension primaryDimension;
        CellTraverser::Dimension secondaryDimension;
        CellTraverser::Dimension cornerDimension;

        unsigned int edgeChessDistance = (m_chessDistance << 1) - 1;

        if(plane1 == CellTraverser::PL_XY && plane2 == CellTraverser::PL_XZ) {
            cornerDimension = CellTraverser::DM_X;

            primaryDimension = glm::abs(m_lineVector.y) > glm::abs(m_lineVector.z) ? CellTraverser::DM_Y : CellTraverser::DM_Z;
            secondaryDimension = glm::abs(m_lineVector.y) > glm::abs(m_lineVector.z) ? CellTraverser::DM_Z : CellTraverser::DM_Y;
        }
        else if(plane1 == CellTraverser::PL_XY && plane2 == CellTraverser::PL_YZ) {
            cornerDimension = CellTraverser::DM_Y;

            primaryDimension = glm::abs(m_lineVector.x) > glm::abs(m_lineVector.z) ? CellTraverser::DM_X : CellTraverser::DM_Z;
            secondaryDimension = glm::abs(m_lineVector.x) > glm::abs(m_lineVector.z) ? CellTraverser::DM_Z : CellTraverser::DM_X;
        }
        else if(plane1 == CellTraverser::PL_XZ && plane2 == CellTraverser::PL_YZ) {
            cornerDimension = CellTraverser::DM_Z;

            primaryDimension = glm::abs(m_lineVector.x) > glm::abs(m_lineVector.y) ? CellTraverser::DM_X : CellTraverser::DM_Y;
            secondaryDimension = glm::abs(m_lineVector.x) > glm::abs(m_lineVector.y) ? CellTraverser::DM_Y : CellTraverser::DM_X;
        }

        CellTraverser::Direction primaryDir = (CellTraverser::Direction) -signO(m_lineVector[primaryDimension]);
        CellTraverser::Direction secondaryDir = (CellTraverser::Direction) -signO(m_lineVector[secondaryDimension]);

        //corner traverser
        glm::ivec3 creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[cornerDimension] += signO(m_lineVector[cornerDimension]);

        CellTraverser::DirectionInfo cornerDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) signO(m_lineVector[cornerDimension]), cornerDimension,
            false,
            glm::abs(creationPosition[cornerDimension] - (m_lineIterator.getStart()[cornerDimension] + m_chessDistance * signO(m_lineVector[cornerDimension]))));

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            cornerDirection,
            CellTraverser::DirectionInfo(),
            CellTraverser::DR_NONE, CellTraverser::DM_X));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //reverse corner traverser
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[cornerDimension] -= signO(m_lineVector[cornerDimension]);

        CellTraverser::DirectionInfo reverseCornerDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) -signO(m_lineVector[cornerDimension]), cornerDimension,
            false,
            glm::abs(creationPosition[cornerDimension] - (m_lineIterator.getStart()[cornerDimension] - m_chessDistance * signO(m_lineVector[cornerDimension]))));

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            reverseCornerDirection,
            CellTraverser::DirectionInfo(),
            CellTraverser::DR_NONE, CellTraverser::DM_X));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);

        CellTraverser::DirectionInfo primaryDirection = CellTraverser::DirectionInfo(primaryDir, primaryDimension,
            true, edgeChessDistance);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            CellTraverser::DirectionInfo(),
            secondaryDir, secondaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);

        CellTraverser::DirectionInfo secondaryDirection = CellTraverser::DirectionInfo(secondaryDir, secondaryDimension,
            true, edgeChessDistance);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryDirection,
            CellTraverser::DirectionInfo(),
            primaryDir, primaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary forward diagonal
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);
        creationPosition[cornerDimension] += signO(m_lineVector[cornerDimension]);

        //this one needs to reflect
        CellTraverser::DirectionInfo cornerPrimaryDirection = cornerDirection;
        cornerPrimaryDirection.m_reflect = true;

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            cornerPrimaryDirection,
            secondaryDir, secondaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary backward diagonal
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);
        creationPosition[cornerDimension] -= signO(m_lineVector[cornerDimension]);

        //this one needs to reflect
        CellTraverser::DirectionInfo reverseCornerPrimaryDirection = reverseCornerDirection;
        reverseCornerPrimaryDirection.m_reflect = true;

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            reverseCornerPrimaryDirection,
            secondaryDir, secondaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary forward diagonal
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);
        creationPosition[cornerDimension] += signO(m_lineVector[cornerDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryDirection,
            cornerDirection,
            primaryDir, primaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary backward diagonal
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);
        creationPosition[cornerDimension] -= signO(m_lineVector[cornerDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryDirection,
            reverseCornerDirection,
            primaryDir, primaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }
    }

    /**
    For the case when you seed from a corner
    */
    void seedCorner() {
        CellTraverser::Dimension primaryDimension;      //primary dimension at corner has its perpendicular diagonals reflect
        CellTraverser::Dimension secondaryDimension;    //secondary dimension will have have its perpendicular diagonals reflect
        CellTraverser::Dimension tertiaryDimension;     //tertiary dimension will just spawn and never reflect

        unsigned int edgeChessDistance = (m_chessDistance << 1) - 1;

        //primary dimension is maximum dimension distance
        //secondary dimension is second maximum dimension distance
        //tertiary dimension is the last
        {
            //I'd probably find a much cleverer way to do this if this was arbitrary dimensional
            glm::ivec3 lineDimensionMagnitude = glm::abs(m_lineVector);
            int maxMagnitude;
            int secMagnitude;

            tertiaryDimension = CellTraverser::DM_Z;

            if(lineDimensionMagnitude.x > lineDimensionMagnitude.y) {
                primaryDimension = CellTraverser::DM_X;
                secondaryDimension = CellTraverser::DM_Y;

                maxMagnitude = lineDimensionMagnitude.x;
                secMagnitude = lineDimensionMagnitude.y;
            }
            else {
                primaryDimension = CellTraverser::DM_Y;
                secondaryDimension = CellTraverser::DM_X;

                maxMagnitude = lineDimensionMagnitude.y;
                secMagnitude = lineDimensionMagnitude.x;
            }

            if(lineDimensionMagnitude.z > maxMagnitude) {
                tertiaryDimension = secondaryDimension;
                secondaryDimension = primaryDimension;
                primaryDimension = CellTraverser::DM_Z;
            }
            else if(lineDimensionMagnitude.z > secMagnitude) {
                tertiaryDimension = secondaryDimension;
                secondaryDimension = CellTraverser::DM_Z;
            }
        }

        //primary corner traverser
        glm::ivec3 creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);

        CellTraverser::DirectionInfo primaryDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) signO(-m_lineVector[primaryDimension]), primaryDimension,
            false, edgeChessDistance);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            CellTraverser::DirectionInfo(),
            CellTraverser::DR_NONE, CellTraverser::DM_X));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary corner traverser
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);

        CellTraverser::DirectionInfo secondaryDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) signO(-m_lineVector[secondaryDimension]), secondaryDimension,
            false, edgeChessDistance);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryDirection,
            CellTraverser::DirectionInfo(),
            CellTraverser::DR_NONE, CellTraverser::DM_X));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //tertiary corner traverser
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[tertiaryDimension] -= signO(m_lineVector[tertiaryDimension]);

        CellTraverser::DirectionInfo tertiaryDirection = CellTraverser::DirectionInfo((CellTraverser::Direction) signO(-m_lineVector[tertiaryDimension]), tertiaryDimension,
            false, edgeChessDistance);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            tertiaryDirection,
            CellTraverser::DirectionInfo(),
            CellTraverser::DR_NONE, CellTraverser::DM_X));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary and secondary (primary will reflect, secondary will reflect) reflect into tertiary dimension
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);

        CellTraverser::DirectionInfo primaryDirectionReflect = primaryDirection;
        primaryDirectionReflect.m_reflect = true;

        CellTraverser::DirectionInfo secondaryDirectionReflect = secondaryDirection;
        secondaryDirectionReflect.m_reflect = true;

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirectionReflect,
            secondaryDirectionReflect,
            (CellTraverser::Direction) signO(-m_lineVector[tertiaryDimension]), tertiaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //primary and tertiary (primary won't reflect reflect, tertiary will) reflect into secondary dimension
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[primaryDimension] -= signO(m_lineVector[primaryDimension]);
        creationPosition[tertiaryDimension] -= signO(m_lineVector[tertiaryDimension]);

        CellTraverser::DirectionInfo tertiaryDirectionReflect = tertiaryDirection;
        tertiaryDirectionReflect.m_reflect = true;

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            primaryDirection,
            tertiaryDirectionReflect,
            (CellTraverser::Direction) signO(-m_lineVector[secondaryDimension]), secondaryDimension));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }

        //secondary and tertiary no reflection
        creationPosition = m_lineIterator.getCurrentPosition();
        creationPosition[secondaryDimension] -= signO(m_lineVector[secondaryDimension]);
        creationPosition[tertiaryDimension] -= signO(m_lineVector[tertiaryDimension]);

        m_cellTraverseStack.push(CellTraverser(this, creationPosition,
            secondaryDirection,
            tertiaryDirection,
            CellTraverser::DR_NONE, CellTraverser::DM_X));

        if(m_debugger) {
            m_debugger->m_traversals.push_back(Debugger::Traversal(m_lineIterator.getCurrentPosition(), creationPosition));
        }
    }

    const Frustum<T>* m_frustum;
    glm::uvec3 m_cellNumber;

    std::stack<CellTraverser> m_cellTraverseStack;

    LineIterator<int> m_lineIterator;
    int m_chessDistance;
    bool m_atEnd;
    glm::ivec3 m_currentCell;
    glm::ivec3 m_nextCell;

    glm::ivec3 m_lineVector;

    Debugger * m_debugger;

    friend CellTraverser;
    friend Debugger;
};

#endif