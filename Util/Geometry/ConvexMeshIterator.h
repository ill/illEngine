#ifndef ILL_CONVEX_MESH_ITERATOR_H_
#define ILL_CONVEX_MESH_ITERATOR_H_

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <list>

#include "Util/serial/StaticList.h"
#include "Util/geometry/MeshEdgeList.h"
#include "Util/geometry/geomUtil.h"

/**
Traverses front to back in the a convex mesh edge list that intersects a GridVolume3D.

@param W The precision of the world space
@param P The precision of the 3d grid volume cell subdivision
*/
template <typename W = glm::mediump_float, typename P = unsigned int>
class ConvexMeshIterator {
private:
    const static bool LEFT_SIDE = true;
    const static bool RIGHT_SIDE = false;

public:
    ConvexMeshIterator()
        : m_isEdgeChecked(NULL),
        m_atEnd(true)
    {}

    ConvexMeshIterator(MeshEdgeList<W>* meshEdgeList, const glm::detail::tvec3<W>& direction, const Box<P>& bounds, const glm::detail::tvec3<W>& cellDimensions) {
        initialize(meshEdgeList, direction, bounds, cellDimensions);
    }

    void initialize(MeshEdgeList<W>* meshEdgeList, const glm::detail::tvec3<W>& direction, const Box<P>& bounds, const glm::detail::tvec3<W>& cellDimensions) {
        m_meshEdgeList = meshEdgeList;
        m_bounds = bounds;
        m_atEnd = false;

        //initialize edges lists
        m_isEdgeChecked = new bool[meshEdgeList->m_edges.size()];
        memset(m_isEdgeChecked, 0, sizeof(bool) * meshEdgeList->m_edges.size());
        
        //initialize world bounds, they're based on the grid not the world bounds of the volume itself
        m_worldBounds.m_min = vec3cast<P, W>(m_bounds.m_min) * cellDimensions;
        m_worldBounds.m_max = vec3cast<P, W>(m_bounds.m_max + glm::detail::tvec3<P>((P) 1)) * cellDimensions - (W)0.001;        //When using floats, you have 7 significant figures of precision

        //initialize remapping of world to algorithm space
        m_dimensionOrder = sortDimensions(direction);
        m_directionSign = vec3cast<glm::mediump_float, int8_t>(signO(direction));

        //initialize the inverse dimension mapping
        for(uint8_t inverseDim = 0; inverseDim < 3; inverseDim++) {
            for(int dim = 0; dim < 3; dim++) {
                if(m_dimensionOrder[dim] == inverseDim) {
                    m_dimensionOrderInverse[inverseDim] = dim;
                }
            }
        }
        
        //remap things into algorithm space
        for(uint8_t dim = 0; dim < 3; dim++) {
            uint8_t mappedDimension = m_dimensionOrder[dim];

            m_algorithmBounds[dim] = m_bounds.m_max[mappedDimension] - m_bounds.m_min[mappedDimension];
            m_algorithmWorldBounds[dim] = m_worldBounds.m_max[mappedDimension] - m_worldBounds.m_min[mappedDimension];
            m_cellDimensions[dim] = cellDimensions[mappedDimension];
        }
                
        //remap meshEdgeList into algorithm space
        for(size_t point = 0; point < m_meshEdgeList->m_points.size(); point++) {
            m_meshEdgeList->m_points[point] = worldToAlgorithmPoint(m_meshEdgeList->m_points[point]);
        }
                
        //start at the origin in algorithm space
        m_currentPosition = glm::detail::tvec3<P>((P) 0);
                
        m_sliceStart = (W) 0;
        m_sliceEnd = m_cellDimensions.z;

        m_currentPointList = false;
        
        //find points within first slice
        for(size_t point = 0; point < meshEdgeList->m_points.size(); point++) {
            if(meshEdgeList->m_points[point].z < m_sliceEnd) {
                addPoint(point, m_activeEdges);
            }
        }
                
        setupSlice();
    }

    ~ConvexMeshIterator() {
        delete[] m_isEdgeChecked;
    }

    inline bool atEnd() const {
        return m_atEnd;
    }

    inline bool forward() {
        if(atEnd()) {
            throw std::runtime_error("calling forward() on mesh iterator when at end");
        }

        if(m_currentPosition.x == m_sliceMax.x) {
            if(m_currentPosition.y == m_sliceMax.y) {
                if(m_currentPosition.z == m_algorithmBounds.z) {
                    m_atEnd = true;
                    return false;
                }
                
                advanceSlice();
            }
            else {
                advanceRow();
            }         
        }
        else {
            m_currentPosition.x++;
        }

        return true;
    }

    inline glm::detail::tvec3<P> getCurrentPosition() const {
        if(atEnd()) {
            throw std::runtime_error("calling getCurrentPosition() on mesh iterator when at end");
        }

        return algorithmToWorldCell(m_currentPosition);
    }

private:
    /**
    Maps a point in world space to algorithm space.
    @param worldPoint The point in world space
    @return The point mapped to algorithm space
    */
    inline glm::detail::tvec3<W> worldToAlgorithmPoint(const glm::detail::tvec3<W>& worldPoint) const {
        glm::detail::tvec3<W> res;

        for(uint8_t dim = 0; dim < 3; ++dim) {
            uint8_t mappedDimension = m_dimensionOrder[dim];

            res[dim] = m_directionSign[mappedDimension] > 0
                ? (worldPoint[mappedDimension] - m_worldBounds.m_min[mappedDimension])
                : (m_worldBounds.m_max[mappedDimension] - worldPoint[mappedDimension]);
        }

        return res;
    }
    
    /**
    Maps a point in algorithm space to world space.
    @param algorithmPoint The point in algorithm space
    @return The point mapped to world space
    */
    inline glm::detail::tvec3<W> algorithmToWorldPoint(const glm::detail::tvec3<W>& algorithmPoint) const {
        glm::detail::tvec3<W> res;
        
        for(uint8_t dim = 0; dim < 3; ++dim) {
            uint8_t mappedDimension = m_dimensionOrderInverse[dim];

            res[dim] = m_directionSign[dim] > 0
                ? (m_worldBounds.m_min[dim] + algorithmPoint[mappedDimension])
                : (m_worldBounds.m_max[dim] - algorithmPoint[mappedDimension]);
        }

        return res;
    }
    
    /**
    Maps a point in world space to algorithm space.
    @param worldPoint The point in world space
    @return The point mapped to algorithm space
    */
    inline glm::detail::tvec3<P> worldToAlgorithmCell(const glm::detail::tvec3<P>& worldCell) const {
        glm::detail::tvec3<P> res;

        for(uint8_t dim = 0; dim < 3; ++dim) {
            uint8_t mappedDimension = m_dimensionOrder[dim];

            res[dim] = m_directionSign[mappedDimension] > 0
                ? (worldCell[mappedDimension] - m_bounds.m_min[mappedDimension])
                : (m_bounds.m_max[mappedDimension] - worldCell[mappedDimension]);
        }

        return res;
    }
    
    /**
    Maps a cell in algorithm space to world space.
    @param algorithmCell The cell in algorithm space
    @return The cell in to world space
    */
    inline glm::detail::tvec3<P> algorithmToWorldCell(const glm::detail::tvec3<P>& algorithmCell) const {
        glm::detail::tvec3<P> res;
        
        for(uint8_t dim = 0; dim < 3; ++dim) {
            uint8_t mappedDimension = m_dimensionOrderInverse[dim];

            res[dim] = m_directionSign[dim] > 0
                ? (m_bounds.m_min[dim] + algorithmCell[mappedDimension])
                : (m_bounds.m_max[dim] - algorithmCell[mappedDimension]);
        }

        return res;
    }
    
    /**
    Should be called only from within addPoint()
    */
    void addPointRecursive(size_t point, std::unordered_map<size_t, P>& activeEdgesDestination) {
        //find all inactive edges for a point
        MeshEdgeList<W>::PointEdgeMapIterators edgeIters = m_meshEdgeList->m_pointEdgeMap.equal_range(point);

        for(MeshEdgeList<W>::PointEdgeMapIterator edgeIter = edgeIters.first; edgeIter != edgeIters.second; edgeIter++) {
            size_t edgeIndex = edgeIter->second;

            //check if the edge is already checked
            if(!m_isEdgeChecked[edgeIndex]) {
                m_isEdgeChecked[edgeIndex] = true;

                const MeshEdgeList<W>::Edge& edge = m_meshEdgeList->m_edges[edgeIndex];

                size_t otherPoint = edge.m_point[point == edge.m_point[0]];
                        
                //find which slice the other point is in relative to this slice
                P sliceNum = grid<W, P>(m_meshEdgeList->m_points[otherPoint].z, m_cellDimensions.z) - grid<W, P>(m_sliceStart, m_cellDimensions.z);
                
                //discard edge if also in this slice
                if(sliceNum <= 0) {
                    //keep recursively adding other points eminating from this edge
                    addPointRecursive(otherPoint, activeEdgesDestination);
                }
                else {
                    //add edge to active edges
                    activeEdgesDestination[edgeIndex] = sliceNum;
                    m_activeEdgeDestPoint[edgeIndex] = otherPoint;
                }
            }
        }
    }

    /**
    Adds a point from the 3D polygon being rasterized.
    */
    inline void addPoint(size_t point, std::unordered_map<size_t, P>& activeEdgesDestination) {
        m_pointList[m_currentPointList].push_back(fixRasterPointPrecision(glm::detail::tvec2<W>(m_meshEdgeList->m_points[point].x, m_meshEdgeList->m_points[point].y)));
        addPointRecursive(point, activeEdgesDestination);
    }

    template <bool isLeftSide>
    void convexHull(const std::vector<glm::detail::tvec2<W>*>& sortedPoints) {
        std::vector<glm::detail::tvec2<W>* >& destination = m_sliceRasterizeEdges[isLeftSide];

        std::vector<glm::detail::tvec2<W>*>::const_iterator iter = sortedPoints.begin();
        std::vector<glm::detail::tvec2<W>*>::const_iterator end = sortedPoints.end();

        for(; iter != end; iter++) {
            glm::detail::tvec2<W>* point = *iter;

            if(m_sliceRasterizeEdges[isLeftSide].size() >= 1) {
                glm::detail::tvec2<W>* prevPoint = destination.back();

                //omit point if same point as previous
                if(eq(point->x, prevPoint->x) && eq(point->y, prevPoint->y)) {
                    continue;
                }

                while(destination.size() >= 2 && leq(cross(*destination[destination.size() - 2] - *point, *destination[destination.size() - 1] - * point), (W) 0, isLeftSide ? -1 : 1)) {
                    destination.pop_back();
                }
            }

            destination.push_back(point);
        }

        //omit first redundant edge (TODO: removing from beginning of vector isn't the most efficient thing ever, is it worth using a dequeue?)
        while(destination.size() >= 2 && eq(destination[0]->y, destination[1]->y)) {
            destination.erase(destination.begin());
        }

        //omit last redundant edge
        while(destination.size() >= 2 && eq(destination[destination.size() - 2]->y, destination[destination.size() - 1]->y)) {
            destination.pop_back();
        }
    }

    void advanceSlice() {
        ++m_currentPosition.z;      

        //advance slice planes
        m_sliceStart = m_sliceEnd;
        m_sliceEnd += m_cellDimensions.z;

        //swap current point buffers
        m_currentPointList = !m_currentPointList;

        //count down all active edge counts
        //a copy is needed because addPoint can't be modifying the same list that's being updated, horrible bugs happen
        std::unordered_map<size_t, P> activeEdgesCopy(m_activeEdges.size());

        for(std::unordered_map<size_t, P>::iterator activeEdgeIter = m_activeEdges.begin(); activeEdgeIter != m_activeEdges.end(); activeEdgeIter++) {
            size_t edgeIndex = activeEdgeIter->first;
            P& activeEdgeCountdown = activeEdgeIter->second;
            
            if(--activeEdgeCountdown == 0) {    //discard this edge, and add its destination point
                addPoint(m_activeEdgeDestPoint.at(edgeIndex), activeEdgesCopy);
            }
            else {
                activeEdgesCopy[edgeIndex] = activeEdgeCountdown; //keep this edge countdown
            }
        }
        
        m_activeEdges.swap(activeEdgesCopy);
        
        setupSlice();
    }

    /**
    Due to floating point imprecision, things get pretty messed up sometimes.
    This makes sure to at least constrain points that are being rasterized to the world bounds.
    */
    inline glm::detail::tvec2<W> fixRasterPointPrecision(glm::detail::tvec2<W>& point) {
        if(eq(point.x, (W) 0)) {
            point.x = (W) 0;
        }
        else if(eq(point.x, m_algorithmWorldBounds.x)) {
            point.x = m_algorithmWorldBounds.x;
        }

        if(eq(point.y, (W) 0)) {
            point.y = (W) 0;
        }
        else if(eq(point.y, m_algorithmWorldBounds.y)) {
            point.y = m_algorithmWorldBounds.y;
        }

        return point;
    }

    /**
    Sets up the current slice to be 2D rasterized
    */
    void setupSlice() {
        //clear other side point buffer
        m_pointList[!m_currentPointList].clear();
        
        //find intersection of active edges against other side of slice and add it to the other points list
        for(std::unordered_map<size_t, P>::const_iterator activeEdgeIter = m_activeEdges.begin(); activeEdgeIter != m_activeEdges.end(); activeEdgeIter++) {
            size_t activeEdge = activeEdgeIter->first;
                        
            assert(m_meshEdgeList->m_points[m_meshEdgeList->m_edges[activeEdge].m_point[0]].z != m_meshEdgeList->m_points[m_meshEdgeList->m_edges[activeEdge].m_point[1]].z);

            glm::detail::tvec3<W> intersection = lineInterceptXY(m_meshEdgeList->m_points[m_meshEdgeList->m_edges[activeEdge].m_point[0]], m_meshEdgeList->m_points[m_meshEdgeList->m_edges[activeEdge].m_point[1]], m_sliceEnd);
                        
            m_pointList[!m_currentPointList].push_back(fixRasterPointPrecision(glm::detail::tvec2<W>(intersection.x, intersection.y)));
        }
        
        /////
        //find convex hull of 2D projected slice points using modified monotone chain algorithm
        //this algorithm is nice because I also have the 2 sides of the polygon I'm about to rasterize in 2D
        //

        //sort points in order of second highest magnitude dimension
        struct PointComparator {
            inline bool operator() (glm::detail::tvec2<W>* ptA, glm::detail::tvec2<W>* ptB) {
                for(uint8_t dimension = 1; dimension < 2; --dimension) {
                    W a = (*ptA)[dimension];
                    W b = (*ptB)[dimension];
                    
                    if(a < b) {
                        return true;
                    } 
                    else if(a > b) {
                        return false;
                    }
                }

                return false;
            }
        };

        std::vector<glm::detail::tvec2<W>*> sortedPoints;

        assert(m_pointList[0].size() + m_pointList[1].size() >= 3);

        for(uint8_t list = 0; list < 2; list++) {
            for(uint8_t point = 0; point < m_pointList[list].size(); point++) {
                sortedPoints.push_back(&m_pointList[list][point]);
            }
        }

        assert(sortedPoints.size() >= 3);

        std::sort(sortedPoints.begin(), sortedPoints.end(), PointComparator());

        //find the convex hull and split it into lists of edges for 1 side and the other
        m_sliceRasterizeEdges[RIGHT_SIDE].clear();
        m_sliceRasterizeEdges[LEFT_SIDE].clear();

        {
            //"right" edge
            convexHull<RIGHT_SIDE>(sortedPoints);

            //"left" edge
            convexHull<LEFT_SIDE>(sortedPoints);
        }
        
        assert(m_sliceRasterizeEdges[LEFT_SIDE].size() >= 2);
        assert(m_sliceRasterizeEdges[RIGHT_SIDE].size() >= 2);

        //setup the initial row
        m_activeSliceEdgeIndex[RIGHT_SIDE] = 0;
        m_activeSliceEdgeIndex[LEFT_SIDE] = 0;
        
        //sets up the min vertical
        P rowNum = grid<W, P>(m_sliceRasterizeEdges[RIGHT_SIDE][0]->y, m_cellDimensions.y);
        m_currentPosition.y = rowNum;
        
        m_lineBottom = rowNum * m_cellDimensions.y;
        m_lineTop = m_lineBottom + m_cellDimensions.y;

        //sets up the max vertical
        rowNum = grid<W, P>(m_sliceRasterizeEdges[RIGHT_SIDE][m_sliceRasterizeEdges[RIGHT_SIDE].size() - 1]->y, m_cellDimensions.y);            
        m_sliceMax.y = rowNum;
        
        assert(m_currentPosition.y >= 0);
        assert(m_sliceMax.y <= m_algorithmBounds.y);

        setupSliceHelper<LEFT_SIDE>();

        assert(m_currentPosition.x >= (P) 0);
        
        setupSliceHelper<RIGHT_SIDE>();

        assert(m_sliceMax.x <= m_algorithmBounds.x);
    }

    template <bool isLeftSide>
    inline void setupSliceHelper() {
        assert(m_activeSliceEdgeIndex[isLeftSide] + 1 < m_sliceRasterizeEdges[isLeftSide].size());
        m_activeSliceEdgeOutward[isLeftSide] = geq(m_sliceRasterizeEdges[isLeftSide][1]->x, m_sliceRasterizeEdges[isLeftSide][0]->x, isLeftSide ? -1 : 1);
        
        if(m_activeSliceEdgeOutward[isLeftSide]) {           //going outward
            if(preAdvanceOutwardLine<isLeftSide>()) {
                while(advanceOutwardLine<isLeftSide>()) {}
            }
        }
        else {                                              //going inward
            //set first point of line as farthest column
            setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][0]->x);

            while(advanceInwardLine<isLeftSide, true>()) {}
        }
    }
    
    void advanceRow() {
        //advance row locations
        m_lineBottom = m_lineTop;
        m_lineTop += m_cellDimensions.y;

        ++m_currentPosition.y;
        
        advanceRowHelper<RIGHT_SIDE>();
        advanceRowHelper<LEFT_SIDE>();
    }

    template <bool isLeftSide>
    inline void advanceRowHelper() {
        //count down the active edges
        --m_activeSliceEdges[isLeftSide];

        //if reached the end of the current edge being rasterized
        if(m_activeSliceEdges[isLeftSide] == 0) {
            if(m_activeSliceEdgeOutward[isLeftSide]) {          //if outward
                //if last line
                if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
                    m_activeSliceEdges[isLeftSide] = 1;

                    setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->x);
                }
                else {
                    while(advanceOutwardLine<isLeftSide>()) {}
                }
            }
            else {                                              //if inward
                //make sure the edge indexes are pointing to no more than the second to last point in each edge list 
                assert(m_activeSliceEdgeIndex[isLeftSide] + 1 < m_sliceRasterizeEdges[isLeftSide].size());

                //find intersection with the bottom of the slice
                W xIntercept = lineInterceptX(*m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]], 
                    *m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1], 
                    m_lineBottom);

                setSliceRowPoint<isLeftSide>(xIntercept);

                //if last line
                if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
                    m_activeSliceEdges[isLeftSide] = 1;
                }
                else {
                    while(advanceInwardLine<isLeftSide, false>()) {}
                }
            }
        }
        else {
            //make sure the edge indexes are pointing to no more than the second to last point in each edge list 
            assert(m_activeSliceEdgeIndex[isLeftSide] + 1 < m_sliceRasterizeEdges[isLeftSide].size());

            //find intersection with either the top of the slice or the bottom depending on if the line is inward or outward
            W xIntercept = lineInterceptX(*m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]], 
                *m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1], 
                m_activeSliceEdgeOutward[isLeftSide] ? m_lineTop : m_lineBottom);

            setSliceRowPoint<isLeftSide>(xIntercept);
        }
    }

    template <bool isLeftSide>
    inline void setSliceRowPoint(W worldX) {
        //find which column that point is in
        P column = grid<W, P>(worldX, m_cellDimensions.x);

        if(isLeftSide) {
            //current position is now this left side column
            m_currentPosition.x = column;
        }
        else {
            //row maximum is now this column
            m_sliceMax.x = column;
        }
    }
    
    /**
    When rasterizing a 2D cross section slice, this advances to the next segment being rasterized.
    Only call this if the current line segment is going outward.
    Initializes the row bounds of the slice being rasterized.

    @tparam isLeftSide Whether or not being called on the right side or the left side of the polygon beign rasterized
    @return Whether or not advancing needs to continue if the next line segment's second point is still within the same row.
    */
    template <bool isLeftSide>
    inline bool advanceOutwardLine() {        
        //if last line
        if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
            //set point B as the farthest point
            setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->x);

            return false;
        }

        //advance to the next line
        m_activeSliceEdgeIndex[isLeftSide]++;
        
        //check what direction next line is going in
        m_activeSliceEdgeOutward[isLeftSide] = geq(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->x, 
            m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->x, 
            isLeftSide ? -1 : 1);
        
        //find which row the other point is in relative to this row
        P rowNum = grid<W, P>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->y, m_cellDimensions.y) 
            - grid<W, P>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->y, m_cellDimensions.y);
        
        //if inward
        if(!m_activeSliceEdgeOutward[isLeftSide]) {
            //set point B as the farthest point
            setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->x);
        }

        //if the current line doesn't end in the same row
        if(rowNum > 0) {
            m_activeSliceEdges[isLeftSide] = rowNum;

            //if outward, clip against row top and set as the farthest column
            if(m_activeSliceEdgeOutward[isLeftSide]) {
                //find intersection against row top
                W xIntercept = lineInterceptX(*m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]], 
                    *m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1], m_lineTop);

                //set this as the max point
                setSliceRowPoint<isLeftSide>(xIntercept);
            }

            return false;
        }
        else {
            //keep advancing

            //if inward, just loop the inward advance code here
            if(!m_activeSliceEdgeOutward[isLeftSide]) {
                while(advanceInwardLine<isLeftSide, false>()) {}
                return false;
            }
            else {
                return true;
            }
        }
    }

    /**
    Similar to advanceOutwardLine but is called at the start of the slice only.
    Only call this if the current line segment is going outward.
    Initializes the row bounds of the slice being rasterized.

    @tparam isLeftSide Whether or not being called on the right side or the left side of the polygon beign rasterized
    @return Whether or not advancing needs to continue if the next line segment's second point is still within the same row.
    */
    template <bool isLeftSide>
    inline bool preAdvanceOutwardLine() {                
        //find which row the other point is in relative to this row
        P rowNum = grid<W, P>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->y, m_cellDimensions.y) 
            - grid<W, P>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->y, m_cellDimensions.y);
                
        //if the current line doesn't end in the same row
        if(rowNum > 0) {
            m_activeSliceEdges[isLeftSide] = rowNum;

            //if outward, clip against row top and set as the farthest column

            //find intersection against row top
            W xIntercept = lineInterceptX(*m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]], 
                *m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1], m_lineTop);

            //set this as the max point
            setSliceRowPoint<isLeftSide>(xIntercept);

            return false;
        }
        else {
            return true;
        }
    }

    /**
    When rasterizing a 2D cross section slice, this advances to the next segment being rasterized.
    Only call this if the current line segment is going inward.
    Initializes the row bounds of the slice being rasterized.

    @tparam isLeftSide Whether or not being called on the right side or the left side of the polygon beign rasterized
    @tparam isFirstTime Whether or not this is happening when the slice is first being set up

    @return Whether or not advancing needs to continue if the next line segment's second point is still within the same row.
    */
    template <bool isLeftSide, bool isFirstTime>
    inline bool advanceInwardLine() {
        //if last line
        if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
            return false;
        }

        //advance to the next line
        if(!isFirstTime) {
            m_activeSliceEdgeIndex[isLeftSide]++;
        }
                        
        //find which row the other point is in relative to this row
        P rowNum = grid<W, P>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->y, m_cellDimensions.y) 
            - grid<W, P>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->y, m_cellDimensions.y);
        
        //if the current line doesn't end in the same row
        if(rowNum > 0) {
            m_activeSliceEdges[isLeftSide] = rowNum;

            return false;
        }
        else {
            //if is first run just keep going
            if(isFirstTime) {
                while(advanceInwardLine<isLeftSide, false>()) {}

                return false;
            }
            else {
                return true;
            }
        }
    }
    
    ///Range of 3D cells being rasterized in world space
    Box<P> m_bounds;

    ///The max range of 3D cells in algorithm space.  No need for a box since the min is always the origin.
    glm::detail::tvec3<P> m_algorithmBounds;

    ///The current grid cell the iterator is pointing to in algorithm space
    glm::detail::tvec3<P> m_currentPosition;

    ///The world in physical space, derived from m_bounds * (m_cellDimensions mapped from algorithm space to world space)
    Box<W> m_worldBounds;

    ///The max world bounds in algorithm space.  No need for a box since the min is always the origin.
    glm::detail::tvec3<W> m_algorithmWorldBounds;

    ///Dimensions of the cells in algorithm space
    glm::detail::tvec3<W> m_cellDimensions;

    /**
    Sorted dimension indeces by order of magnitude of the direction vector the view frustum faces
    Used to map points from world space into algorithm space as well.
    */
    glm::detail::tvec3<uint8_t> m_dimensionOrder;

    /**
    Inverse of dimensionOrder and used to map points from algorithm space back to world space
    */
    glm::detail::tvec3<uint8_t> m_dimensionOrderInverse;

    ///Sign of each dimension in the direction vector the view frustum faces in world space, used during mapping between the two spaces
    glm::detail::tvec3<int8_t> m_directionSign;
    
    ///Edges that have already been processed by the rasterizing algorithm and are either discarded or active
    bool * m_isEdgeChecked;

    ///Edges that are currently being processed by the rasterizing algorithm, mapping of edge to how many slices until the other edge end
    std::unordered_map<size_t, P> m_activeEdges;

    ///The active edge point index that the slice is going towards
    std::unordered_map<size_t, size_t> m_activeEdgeDestPoint;

    /*
    The mesh edge list itself, its points become mapped to algorithm space when starting the algorithm
    Create a copy of the mesh edge list.
    */
    MeshEdgeList<W>* m_meshEdgeList;

    ///The points that make up the slice of the mesh currently being rasterized
    std::vector<glm::detail::tvec2<W> > m_pointList[2];

    ///Index of which of the points lists is the one from the previous slice front side
    bool m_currentPointList;

    ///The current backside of the mesh slice
    W m_sliceStart;

    ///The current front side of the mesh slice
    W m_sliceEnd;

    ///The current "bottom" of the rasterizing polygon
    W m_lineBottom;

    ///The current "top" of the rasterizing polygon
    W m_lineTop;

    ///The current maximums for the 2D rasterizing, the x value changes at every row, the slice is done being rasterized when m_currentPosition[m_dimensionOrder[1]] y of sliceMax is reached
    glm::detail::tvec2<P> m_sliceMax;

    ///The edge lists for the convex polygon rasterizing
    std::vector<glm::detail::tvec2<W>* > m_sliceRasterizeEdges[2];

    ///The index of the first point of the active edge
    size_t m_activeSliceEdgeIndex[2];

    ///The countdown of how many rows until the next point edge is reached
    int m_activeSliceEdges[2];

    ///Whether or not the current active edge for each side is heading away from or towards the center of the polygon's middle vertical axis
    bool m_activeSliceEdgeOutward[2];

    ///Whether or not the iterator is done and no more points are left to rasterize
    bool m_atEnd;
};

#endif