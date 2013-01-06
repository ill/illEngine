#ifndef ILL_GRID_VOLUME_3D_H__
#define ILL_GRID_VOLUME_3D_H__

#include <glm/glm.hpp>
#include "Box.h"

/**
A 3D uniform grid volume.
Has some methods for rasterizing other shapes within the 3D volume which would be useful for accessing 3D grid cells in a uniform 3D grid scene.

TODO: use iterators for most of the things, such as iterators for a 3D box rather than just returning a range of 3D cells in a box and doing the same code
all over the place to iterate over all cells.

Also some parallel iterators would be nice.
*/
template<typename T, typename P = glm::mediump_float>
class GridVolume3D {
public:
    GridVolume3D()
        : m_cells(NULL)
    {}

    GridVolume3D(const glm::detail::tvec3<P>& cellDimensions, const glm::uvec3& cellNumber)
        : m_cellDimensions(cellDimensions),
        m_cellNumber(cellNumber)
    {
        m_volumeBounds = Box<P>(glm::detail::tvec3<P>((P) 0), 
            glm::detail::tvec3<P>(m_cellDimensions.x * m_cellNumber.x - (P)0.1, 
                m_cellDimensions.y * m_cellNumber.y - (P)0.1, 
                m_cellDimensions.z * m_cellNumber.z - (P)0.1));
        m_cells = new T[m_cellNumber.x * m_cellNumber.y * m_cellNumber.z];
    }

    ~GridVolume3D() {
        delete[] m_cells;
    }

    inline unsigned int getCellArrayIndex(glm::detail::tvec3<P> position) const {
        return getCellArrayIndex(getCellIndex(position));
    }

    inline unsigned int getCellArrayIndex(glm::uvec3 index) const {
        return index.x + m_cellNumber.x * (index.y + m_cellNumber.z * index.z);
    }

    inline glm::uvec3 getCellIndex(glm::detail::tvec3<P> position) const {
        m_volumeBounds.constrain(position);

        return glm::uvec3(
            (unsigned int) position.x / m_cellDimensions.x,
            (unsigned int) position.y / m_cellDimensions.y,
            (unsigned int) position.z / m_cellDimensions.z);
    }

    inline Box<unsigned int> getCellIndex(Box<P> bounds) const {
        return Box<unsigned int>(getCellIndex(bounds.m_min), getCellIndex(bounds.m_max));
    }

    inline T* getCell(const glm::uvec3& cellIndex) const {
        return getCell(getCellArrayIndex(cellIndex));
    }

    inline T* getCell(unsigned int arrayIndex) const {
        return &m_cells[arrayIndex];
    }

    inline const Box<P>& getVolumeBounds() const {
        return m_volumeBounds;
    }

    inline const glm::detail::tvec3<P>& getCellDimensions() const {
        return m_cellDimensions;
    }

    inline const glm::uvec3& getCellNumber() const {
        return m_cellNumber;
    }

private:
    ///The bounds of the volume
    Box<P> m_volumeBounds;

    ///The width, height, depth dimensions per 3D cell
    glm::detail::tvec3<P> m_cellDimensions;

    ///The number of cells in each x, y, z dimension
    glm::uvec3 m_cellNumber;

    ///The grid of cells
    T * m_cells;
};

#endif