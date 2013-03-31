#ifndef ILL_GRID_VOLUME_3D_H_
#define ILL_GRID_VOLUME_3D_H_

#include <glm/glm.hpp>
#include "Util/Geometry/Iterators/MultiConvexMeshIterator.h"
#include "Util/Geometry/Iterators/ConvexMeshIterator.h"
#include "Util/Geometry/geomUtil.h"
#include "Util/Geometry/Box.h"
#include "Util/Geometry/Iterators/BoxIterator.h"
#include "Util/Geometry/Iterators/BoxOmitIterator.h"
#include "Util/Geometry/Plane.h"

/**
A 3D uniform grid volume.
Has some methods for rasterizing other shapes within the 3D volume which would be useful for accessing 3D grid cells in a uniform 3D grid scene.

TODO: use iterators for most of the things, such as iterators for a 3D box rather than just returning a range of 3D cells in a box and doing the same code
all over the place to iterate over all cells.

@param W The precision of the world space
*/
template <typename W = glm::mediump_float>
class GridVolume3D {
public:
    GridVolume3D() {}

    /**
    Creates the GridVolume

    @param cellDimensions The dimensions of each cell, usually they can just be a perfect cube
    @param cellNumber The number of cells in each dimension, x, y, and z
    */
    GridVolume3D(const glm::detail::tvec3<W>& cellDimensions, const glm::uvec3& cellNumber)
        : m_cellDimensions(cellDimensions),
        m_cellNumber(cellNumber)
    {
        m_volumeBounds = Box<W>(glm::detail::tvec3<W>((W) 0), 
            glm::detail::tvec3<W>(m_cellDimensions.x * m_cellNumber.x - (W)0.01, 
                m_cellDimensions.y * m_cellNumber.y - (W)0.01, 
                m_cellDimensions.z * m_cellNumber.z - (W)0.01));
    }
    
    /**
    Gets the array index for a position in the world snapped to the grid.
    */
    inline unsigned int indexForWorld(const glm::detail::tvec3<W>& position) const {
        return getCellArrayIndex(getCellIndex(position));
    }

    /**
    Gets the array index for a grid cell position
    */
    inline unsigned int indexForCell(const glm::uvec3& index) const {
        return index.x + m_cellNumber.x * (index.y + m_cellNumber.y * index.z);
    }

    /**
    Gets the grid cell position for a position in the  world snapped to the grid.
    */
    inline glm::uvec3 cellForWorld(glm::detail::tvec3<W> position) const {
        m_volumeBounds.constrain(position);

        return gridVec<W, unsigned int>(position, m_cellDimensions);
    }

    /**
    Gets the grid cell positions for a range of world positions
    */
    inline Box<unsigned int> cellBoundsForWorldBounds(Box<W> bounds) const {
        assert(bounds.isNormalized());

        m_volumeBounds.constrain(bounds);

        assert(bounds.isNormalized());

        return bounds.grid<unsigned int>(m_cellDimensions);
    }

    /**
    Returns a box iterator for some world bounds.
    */
    inline BoxIterator<unsigned int> boxIterForWorldBounds(const Box<W>& bounds) const {
        assert(bounds.isNormalized());
        return boxIterForCellBounds(cellBoundsForWorldBounds(bounds));
    }

    /**
    Returns a box omit iterator for some world bounds.
    */
    inline BoxOmitIterator<unsigned int> boxOmitIterForWorldBounds(const Box<W>& bounds, const Box<W>& omitBounds) const {
        assert(bounds.isNormalized());
        assert(omitBounds.isNormalized());
        return boxOmitIterForCellBounds(cellBoundsForWorldBounds(bounds), cellBoundsForWorldBounds(omitBounds));
    }

    /**
    Returns a box iterator for some cell bounds.
    */
    inline BoxIterator<unsigned int> boxIterForCellBounds(const Box<unsigned int>& bounds) const {
        assert(bounds.isNormalized());
        return BoxIterator<unsigned int>(bounds);
    }

    /**
    Returns a box omit iterator for some cell bounds.
    */
    inline BoxOmitIterator<unsigned int> boxOmitIterForCellBounds(const Box<unsigned int>& bounds, const Box<unsigned int>& omitBounds) const {
        assert(bounds.isNormalized());
        assert(omitBounds.isNormalized());
        return BoxOmitIterator<unsigned int>(bounds, omitBounds);
    }

    /**
    Returns a convex mesh iterator for a convex mesh stored in a MeshEdgeList.  Be sure the mesh is convex or bad things will happen.
    Also clips the mesh against the bounds of the grid volume, so make a copy of the mesh edge list before passing in if you want to preserve the unclipped version.

    @param meshEdgeList The mesh edge list to get an iterator for
    @param direction What direction vector to slice up the mesh in when iterating
    */
    ConvexMeshIterator<W, unsigned int> meshIteratorForMesh(MeshEdgeList<W>* meshEdgeList, 
            const glm::detail::tvec3<W>& direction = glm::detail::tvec3<W>((W)0, (W)-1, (W)0)) const {
        if(!clipMeshEdgeList(meshEdgeList)) {
            return ConvexMeshIterator<W, unsigned int>();
        }

        //get intersection of frustum and bounds
        Box<unsigned int> iterBounds(glm::uvec3(0), m_cellNumber);
        Box<unsigned int> frustumGrid(meshEdgeList->m_bounds.grid<unsigned int>(m_cellDimensions));

        if(iterBounds.intersects(frustumGrid)) {
            iterBounds.constrain(frustumGrid);
            return ConvexMeshIterator<W, unsigned int>(meshEdgeList, 
                sortDimensions(direction), vec3cast<glm::mediump_float, int8_t>(signO(direction)),
                cellBoundsForWorldBounds(meshEdgeList->m_bounds), m_cellDimensions);
        }
        else {
            return ConvexMeshIterator<W, unsigned int>();
        }
    }

    /**
    Similar to meshIteratorForMesh except this fans out from the center cell to truly give an approximate front to back
    traversal.  meshIteratorForMesh() is good for when more correct front to back traversal isn't needed.

    This basically splits the mesh into 8 separate meshes around the center grid cell and traverses them in a fanned out order
    to prevent the really wide viewing angles rendering faraway objects in the wide portion before rendering the things near the eye.

    @param newIterator Pass in the iterator so it can be set up for you.
    @param meshEdgeList The mesh edge list to get an iterator for
    @param centerPosition The world center position around where to begin the fanout traversal.
    @param direction What direction vector to slice up the mesh in when iterating
    */
    inline void orderedMeshIteratorForMesh(MultiConvexMeshIterator<W, unsigned int>& newIterator, MeshEdgeList<W>* meshEdgeList, 
            const glm::detail::tvec3<W> centerPosition, const glm::detail::tvec3<W>& direction = glm::detail::tvec3<W>((W)0, (W)-1, (W)0)) const {
        return orderedMeshIteratorForMesh(newIterator, meshEdgeList, cellForWorld(centerPosition), direction);
    }

    /**
    Same as the other method, only this takes a center grid cell. 
    */
    void orderedMeshIteratorForMesh(MultiConvexMeshIterator<W, unsigned int>& newIterator, MeshEdgeList<W>* meshEdgeList, 
            const glm::uvec3& centerCell, const glm::detail::tvec3<W>& direction = glm::detail::tvec3<W>((W)0, (W)-1, (W)0)) const {
        if(!clipMeshEdgeList(meshEdgeList)) {
            return;
        }

        //get intersection of frustum and bounds
        Box<unsigned int> iterBounds(glm::uvec3(0), m_cellNumber);
        Box<unsigned int> frustumGrid(meshEdgeList->m_bounds.grid<unsigned int>(m_cellDimensions));

        if(iterBounds.intersects(frustumGrid)) {
            //figure out the ordering based on view direction
            glm::detail::tvec3<uint8_t> dimensionOrder = sortDimensions(direction);
                        
            //find the origin point
            glm::vec3 splitOrigin = m_cellDimensions * vec3cast<unsigned int, glm::mediump_float>(centerCell);
            
            newIterator.m_iterators.resize(newIterator.m_iterators.size() + 8);
            newIterator.m_meshEdgeListCopies.resize(newIterator.m_meshEdgeListCopies.size() + 8);

            //split into 8
            int currIter = 0;

            for(int iter = 0; iter < 8; iter++) {
                glm::detail::tvec3<int8_t> directionSign;

                Box<> bounds;

                //create a copy of the clipped mesh
                newIterator.m_meshEdgeListCopies[currIter] = *meshEdgeList;

                //now clip it based on the cell where the camera center point is
                if(iter == 0 || iter == 1 || iter == 4 || iter == 5) {  //couldn't think of a clever pattern for this one
                    //primary positive
                    directionSign[0] = 1;

                    glm::vec3 planeNormal;
                    planeNormal[0] = 1.0f;

                    newIterator.m_meshEdgeListCopies[currIter].convexClip(Plane<>(planeNormal, -splitOrigin[0]));

                    bounds.m_min[0] = splitOrigin[0];
                    bounds.m_max[0] = (m_cellDimensions.x * (float)m_cellNumber.x) - 0.001f;
                }
                else {
                    //primary negative
                    directionSign[0] = -1;

                    glm::vec3 planeNormal;
                    planeNormal[0] = -1.0f;

                    newIterator.m_meshEdgeListCopies[currIter].convexClip(Plane<>(planeNormal, splitOrigin[0] - 0.001f));

                    bounds.m_min[0] = -m_cellDimensions.x * (float)m_cellNumber.x;
                    bounds.m_max[0] = splitOrigin[0] - 0.001f;
                }

                if(iter % 2 == 0) {
                    //secondary positive
                    directionSign[1] = 1;

                    glm::vec3 planeNormal;
                    planeNormal[1] = 1.0f;

                    newIterator.m_meshEdgeListCopies[currIter].convexClip(Plane<>(planeNormal, -splitOrigin[1]));

                    bounds.m_min[1] = splitOrigin[1];
                    bounds.m_max[1] = (m_cellDimensions.y * (float)m_cellNumber.y) - 0.001f;
                }
                else {
                    //secondary negative
                    directionSign[1] = -1;

                    glm::vec3 planeNormal;
                    planeNormal[1] = -1.0f;

                    newIterator.m_meshEdgeListCopies[currIter].convexClip(Plane<>(planeNormal, splitOrigin[1] - 0.001f));

                    bounds.m_min[1] = -m_cellDimensions.y * (float)m_cellNumber.y;
                    bounds.m_max[1] = splitOrigin[1] - 0.001f;
                }

                if(iter < 4) {
                    //tertiary positive
                    directionSign[2] = 1;

                    glm::vec3 planeNormal;
                    planeNormal[2] = 1.0f;

                    newIterator.m_meshEdgeListCopies[currIter].convexClip(Plane<>(planeNormal, -splitOrigin[2]));

                    bounds.m_min[2] = splitOrigin[2];
                    bounds.m_max[2] = (m_cellDimensions.z * (float)m_cellNumber.z) - 0.001f;
                }
                else {
                    //tertiary negative
                    directionSign[2] = -1;

                    glm::vec3 planeNormal;
                    planeNormal[2] = -1.0f;

                    newIterator.m_meshEdgeListCopies[currIter].convexClip(Plane<>(planeNormal, splitOrigin[2] - 0.001f));

                    bounds.m_min[2] = -m_cellDimensions.z * (float)m_cellNumber.z;
                    bounds.m_max[2] = splitOrigin[2] - 0.001f;
                }
                
                if(!newIterator.m_meshEdgeListCopies[currIter].m_points.empty()) {
                    newIterator.m_meshEdgeListCopies[currIter].computeBounds(bounds);
                    Box<unsigned int> thisFrustumGrid(newIterator.m_meshEdgeListCopies[currIter].m_bounds.grid<unsigned int>(m_cellDimensions));

                    if(iterBounds.intersects(thisFrustumGrid)) {
                        iterBounds.constrain(thisFrustumGrid);

                        newIterator.m_iterators[currIter].initialize(&newIterator.m_meshEdgeListCopies[currIter], 
                            dimensionOrder, directionSign, 
                            thisFrustumGrid,
                            m_cellDimensions);

                        currIter++;
                    }
                }
            }

            newIterator.m_iterators.resize(currIter);
            newIterator.m_meshEdgeListCopies.resize(currIter);
        }
    }
    
    inline const Box<W>& getVolumeBounds() const {
        return m_volumeBounds;
    }

    inline const glm::detail::tvec3<W>& getCellDimensions() const {
        return m_cellDimensions;
    }

    inline const glm::uvec3& getCellNumber() const {
        return m_cellNumber;
    }

private:
    /**
    Clips a mesh edge list against the bounds of the volume.
    @param True if the mesh edge list isn't empty as a result.
        False if after clipping, the mesh is empty.  This would result
        from the mesh being outside the bounds.
    */
    bool clipMeshEdgeList(MeshEdgeList<W>* meshEdgeList) const {
        if(!m_volumeBounds.intersects(meshEdgeList->m_bounds)) {
            return false;
        }

        //clip the mesh against the bounds
        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 1, (W) 0, (W) 0), -m_volumeBounds.m_min.x));
        if(meshEdgeList->m_points.empty()) {
            return false;
        }

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) 1, (W) 0), -m_volumeBounds.m_min.y));
        if(meshEdgeList->m_points.empty()) {
            return false;
        }

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) 0, (W) 1), -m_volumeBounds.m_min.z));
        if(meshEdgeList->m_points.empty()) {
            return false;
        }


        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) -1, (W) 0, (W) 0), m_volumeBounds.m_max.x));
        if(meshEdgeList->m_points.empty()) {
            return false;
        }

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) -1, (W) 0), m_volumeBounds.m_max.y));
        if(meshEdgeList->m_points.empty()) {
            return false;
        }

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) 0, (W) -1), m_volumeBounds.m_max.z));
        if(meshEdgeList->m_points.empty()) {
            return false;
        }
        
        meshEdgeList->computeBounds(m_volumeBounds);

        return true;
    }

    ///The bounds of the volume
    Box<W> m_volumeBounds;

    ///The width, height, depth dimensions per 3D cell
    glm::detail::tvec3<W> m_cellDimensions;

    ///The number of cells in each x, y, z dimension
    glm::uvec3 m_cellNumber;
};

#endif