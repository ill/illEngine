#ifndef ILL_GRID_VOLUME_3D_H_
#define ILL_GRID_VOLUME_3D_H_

#include <glm/glm.hpp>
#include "Util/Geometry/ConvexMeshIterator.h"
#include "Util/Geometry/geomUtil.h"
#include "Util/Geometry/Box.h"
#include "Util/Geometry/BoxIterator.h"
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
        return index.x + m_cellNumber.x * (index.y + m_cellNumber.z * index.z);
    }

    /**
    Gets the grid cell position for a position in the  world snapped to the grid.
    */
    inline glm::uvec3 cellForWorld(glm::detail::tvec3<W> position) const {
        m_volumeBounds.constrain(position);

        return grid(position, m_cellDimensions);
    }

    /**
    Gets the grid cell positions for a range of world positions
    */
    inline Box<unsigned int> cellBoundsForWorldBounds(Box<W> bounds) const {
        m_volumeBounds.constrain(bounds);

        return bounds.grid<unsigned int>(m_cellDimensions);
    }

    /**
    Returns a box iterator for some world bounds.
    */
    inline BoxIterator<W> boxIterForWorldBounds(const Box<W>& bounds) const {
        return boxIterForCellBounds(cellBoundsForWorldBounds(bounds));
    }

    /**
    Returns a box iterator for some cell bounds.
    */
    inline BoxIterator<W> boxIterForCellBounds(const Box<unsigned int>& bounds) const {
        return BoxIterator<W>(bounds);
    }

    /**
    Returns a convex mesh iterator for a convex mesh stored in a MeshEdgeList.  Be sure the mesh is convex or bad things will happen.
    Also clips the mesh against the bounds of the grid volume, so make a copy of the mesh edge list before passing in if you want to preserve the unclipped version.

    @param meshEdgeList The mesh edge list to get an iterator for
    @param direction What direction vector to slice up the mesh in when iterating
    */
    ConvexMeshIterator<W, unsigned int> meshIteratorForMesh(MeshEdgeList<W>* meshEdgeList, 
            const glm::detail::tvec3<W>& direction = glm::detail::tvec3<W>((W)0, (W)-1, (W)0)) const {
        if(!m_volumeBounds.intersects(meshEdgeList->m_bounds)) {
            return ConvexMeshIterator<W, unsigned int>();
        }

        //clip the mesh against the bounds
        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 1, (W) 0, (W) 0), -m_volumeBounds.m_min.x));
        if(meshEdgeList->m_points.empty()) return ConvexMeshIterator<W, unsigned int>();

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) 1, (W) 0), -m_volumeBounds.m_min.y));
        if(meshEdgeList->m_points.empty()) return ConvexMeshIterator<W, unsigned int>();

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) 0, (W) 1), -m_volumeBounds.m_min.z));
        if(meshEdgeList->m_points.empty()) return ConvexMeshIterator<W, unsigned int>();


        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) -1, (W) 0, (W) 0), m_volumeBounds.m_max.x));
        if(meshEdgeList->m_points.empty()) return ConvexMeshIterator<W, unsigned int>();

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) -1, (W) 0), m_volumeBounds.m_max.y));
        if(meshEdgeList->m_points.empty()) return ConvexMeshIterator<W, unsigned int>();

        meshEdgeList->convexClip(Plane<>(glm::detail::tvec3<W>((W) 0, (W) 0, (W) -1), m_volumeBounds.m_max.z));
        if(meshEdgeList->m_points.empty()) return ConvexMeshIterator<W, unsigned int>();


        meshEdgeList->computeBounds(m_volumeBounds);

        //get intersection of frustum and bounds
        Box<unsigned int> iterBounds(glm::uvec3(0), m_cellNumber);
        Box<unsigned int> frustumGrid(meshEdgeList->m_bounds.grid<unsigned int>(m_cellDimensions));

        if(iterBounds.intersects(frustumGrid)) {
            iterBounds.constrain(frustumGrid);

            return ConvexMeshIterator<W, unsigned int>(meshEdgeList, direction, cellBoundsForWorldBounds(meshEdgeList->m_bounds), m_cellDimensions);
        }
        else {
            return ConvexMeshIterator<W, unsigned int>();
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
    ///The bounds of the volume
    Box<W> m_volumeBounds;

    ///The width, height, depth dimensions per 3D cell
    glm::detail::tvec3<W> m_cellDimensions;

    ///The number of cells in each x, y, z dimension
    glm::uvec3 m_cellNumber;
};

#endif