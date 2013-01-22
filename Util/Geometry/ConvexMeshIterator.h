#ifndef ILL_CONVEX_MESH_ITERATOR_H__
#define ILL_CONVEX_MESH_ITERATOR_H__

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <list>

#include "Util/serial/StaticList.h"
#include "Util/geometry/GridVolume3D.h"
#include "Util/geometry/MeshEdgeList.h"
#include "Util/geometry/geomUtil.h"

const bool LEFT_SIDE = true;
const bool RIGHT_SIDE = false;

const uint8_t SLICE_DIM = 0;
const uint8_t X_DIM = 2;
const uint8_t Y_DIM = 1;

/**
Traverses front to back in the a convex mesh edge list that intersects a GridVolume3D.

@param W The precision of the world space
@param P The precision of the 3d grid volume cell subdivision
*/
template <typename W = glm::mediump_float, typename P = int>
class ConvexMeshIterator {
public:
    struct Debugger {
        /**
        A little convenience point to turn those 2D slice points into a 3D point for rendering
        */
        glm::detail::tvec3<W> sliceTo3D(const glm::detail::tvec2<W>& coord, W sliceCoord) const {
            glm::detail::tvec3<W> res;

            res[m_iterator->m_dimensionOrder[SLICE_DIM]] = sliceCoord;
            res[m_iterator->m_dimensionOrder[X_DIM]] = coord.x;
            res[m_iterator->m_dimensionOrder[Y_DIM]] = coord.y;

            return res;
        }

        ConvexMeshIterator * m_iterator;
        MeshEdgeList<W> m_meshEdgeList;

        std::unordered_set<size_t> m_discarededEdges;

        std::vector<W> m_pointListMissingDim[2];  //the missing 3rd dimension from the point after clipping edge against plane, used in the debug draw

        std::vector<glm::detail::tvec2<W>*> m_sortedSlicePoints;
        std::vector<glm::detail::tvec3<W> > m_rasterizedCells;  //cells that were rasterized so far in world coords for the renderer to render

        W m_leftSlicePoint; //the point set by setLeftSlicePoint
        W m_rightSlicePoint; //the point set by setLeftSlicePoint

        glm::detail::tvec2<P> m_sliceMin;

        std::list<std::string> m_messages;
    };

    ConvexMeshIterator()
        : m_isEdgeChecked(NULL) 
    {}

    ConvexMeshIterator(const MeshEdgeList<W>* meshEdgeList, const glm::detail::tvec3<W>& direction, const Box<P>& range, const glm::detail::tvec3<W>& cellDimensions)
        : m_meshEdgeList(meshEdgeList),
        m_range(range),
        m_cellDimensions(cellDimensions)
    {
        //initialize edges lists
        m_isEdgeChecked = new bool[meshEdgeList->m_edges.size()];
        memset(m_isEdgeChecked, 0, sizeof(bool) * meshEdgeList->m_edges.size());

        //initialize a bunch of things
        m_dimensionOrder = sortDimensions(direction);
        m_directionSign = glm::sign(direction);

        //reorder the bounds box according to the direction sign
        for(uint8_t dimension = 0; dimension < 3; dimension++) {
            if(m_directionSign[dimension] < 0) {
                P temp = m_range.m_min[dimension];
                m_range.m_min[dimension] = m_range.m_max[dimension];
                m_range.m_max[dimension] = temp;
            }
        }

        m_debugger.m_iterator = this;
        m_debugger.m_meshEdgeList = MeshEdgeList<W>(*m_meshEdgeList);
        //make iterator itself refer to debugger copy so as I'm looking around it doesn't change
        //TODO: take thsi out
        m_meshEdgeList = &m_debugger.m_meshEdgeList;

        m_spaceRange = Box<W>(m_cellDimensions * vec3cast<P, W>(m_range.m_min), m_cellDimensions * vec3cast<P, W>(m_range.m_max));

        m_currentPosition = m_range.m_min;

        uint8_t sliceDimension = m_dimensionOrder[SLICE_DIM];
        
        m_sliceStart = m_spaceRange.m_min[sliceDimension];
        W sliceEnd = m_sliceStart + m_directionSign[sliceDimension] * m_cellDimensions[sliceDimension];

        //set up the slice plane, it's normal is in the direction of the slice dimension
        m_slicePlane.m_normal = glm::detail::tvec3<W>((W) 0);
        m_slicePlane.m_normal[sliceDimension] = m_directionSign[sliceDimension];
        m_slicePlane.m_distance = -m_directionSign[sliceDimension] * sliceEnd;

        m_currentPointList = false;

        //find points within first slice
        for(size_t point = 0; point < meshEdgeList->m_points.size(); point++) {
            if((m_directionSign[sliceDimension] > 0 && meshEdgeList->m_points[point][sliceDimension] < sliceEnd)
                    || (m_directionSign[sliceDimension] < 0 && meshEdgeList->m_points[point][sliceDimension] >= sliceEnd)) {
                addPoint(point, m_activeEdges);
            }
        }
        
        if(!setupSlice()) {
            while(!advanceSlice()) {}
        }
    }

    ~ConvexMeshIterator() {
        delete[] m_isEdgeChecked;
    }

    inline bool atEnd() const {
        return m_currentPosition[m_dimensionOrder[X_DIM]] == m_sliceMax.x 
            && m_currentPosition[m_dimensionOrder[Y_DIM]] == m_sliceMax.y 
            && m_currentPosition[m_dimensionOrder[SLICE_DIM]] == m_range.m_max[m_dimensionOrder[SLICE_DIM]];
    }

    inline bool forward() {
        if(m_currentPosition[m_dimensionOrder[X_DIM]] == m_sliceMax.x) {
            if(m_currentPosition[m_dimensionOrder[Y_DIM]] == m_sliceMax.y) {
                while(!advanceSlice()) {
                    return atEnd();
                }
            }
            else {
                advanceRow();
            }         
        }
        else {
            m_currentPosition[m_dimensionOrder[X_DIM]] += m_directionSign[m_dimensionOrder[X_DIM]];

            m_debugger.m_rasterizedCells.push_back(m_cellDimensions * vec3cast<P, W>(m_currentPosition) + m_cellDimensions * 0.5f * vec3cast<int8_t, W>(m_directionSign));
        }

        return true;
    }

    inline const glm::detail::tvec3<P>& getCurrentPosition() const {
        if(atEnd()) {
            throw std::runtime_error("calling getCurrentPosition() on mesh iterator when at end");
        }

        return m_currentPosition;
    }

    //TODO: temporarily public while developing
    //private:   
    
    /**
    Returns some distance with the proper sign depending on the dimension.

    @param distance.
    @param dimension The x, y, or z dimension.  Use 0, 1, 2 to index into it.

    @tparam X The type for which distance is being found
    */
    template <typename X>
    inline X distance(X distance, uint8_t dimension) {
        return distance * m_directionSign[dimension];
    }

    /**
    Returns the grid cell for a point in space.

    @param worldLocation The location in the world so it can be snapped to the grid.
    @param dimension The x, y, or z dimension.  Use 0, 1, 2 to index into it.
    */
    inline P gridLocation(W worldLocation, uint8_t dimension) {        
        P res = grid<W, P>(worldLocation, m_cellDimensions[dimension]);

        //if a world location is RIGHT at the edge of positive bounds this will actually set it to the next grid cell and be off by 1
        //this is a BIT of a hack, just a bit
        if(m_directionSign[dimension] > 0) {    //if positive
            if(worldLocation == m_spaceRange.m_max[dimension]) {
                res--;
            }
        }
        else {                                  //if negative
            if(worldLocation == m_spaceRange.m_min[dimension]) {
                res--;
            }
        }

        return res;
    }

    /**
    Returns the distance in grid cells between two world points.

    @param dimension The x, y, or z dimension.  Use 0, 1, 2 to index into it.
    */
    inline P gridDistance(W worldLocationA, W worldLocationB, uint8_t dimension) {
        return distance<P>(gridLocation(worldLocationB, dimension) - gridLocation(worldLocationA, dimension), dimension);
    }

    /**
    Adds a point from the 3D polygon being rasterized.
    */
    void addPoint(size_t point, std::unordered_map<size_t, unsigned int>& activeEdgesDestination) {
        m_pointList[m_currentPointList].push_back(glm::detail::tvec2<W>(m_meshEdgeList->m_points[point][m_dimensionOrder[X_DIM]], m_meshEdgeList->m_points[point][m_dimensionOrder[Y_DIM]]));
        m_debugger.m_pointListMissingDim[m_currentPointList].push_back(m_meshEdgeList->m_points[point][m_dimensionOrder[SLICE_DIM]]);

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
                int sliceNum = gridDistance(m_sliceStart, m_meshEdgeList->m_points[otherPoint][m_dimensionOrder[SLICE_DIM]], m_dimensionOrder[SLICE_DIM]);

                //discard edge if also in this slice
                if(sliceNum <= 0) {
                    m_debugger.m_discarededEdges.insert(edgeIndex);
                }
                else {
                    //add edge to active edges
                    activeEdgesDestination[edgeIndex] = sliceNum;
                    m_activeEdgeDestPoint[edgeIndex] = otherPoint;
                }
            }
        }
    }

    template <typename Iter>
    static void convexHull(Iter& iter, Iter& end, std::vector<glm::detail::tvec2<W>* >& destination, int8_t sign) {
        //TODO: remove redundant edges

        if(iter != end) {
            //init some things first
            glm::detail::tvec2<W>* point = *iter;            
            destination.push_back(point);
            iter++;

            if(iter != end) {
                point = *iter;
                destination.push_back(point);
                iter++;

                //now do the actual loop
                for(; iter != end; iter++) {
                    point = *iter;

                    while(destination.size() >= 2 && leq(cross(*destination[destination.size() - 2] - *point, *destination[destination.size() - 1] - * point), (W) 0, sign)) {
                        destination.pop_back();
                    }

                    destination.push_back(point);
                }
            }
        }
    }

    bool advanceSlice() {
        if(atEnd()) {
            return true;
        }

        m_currentPosition[m_dimensionOrder[SLICE_DIM]] += m_directionSign[SLICE_DIM];      

        //advance slice planes
        m_slicePlane.m_distance -= m_cellDimensions[m_dimensionOrder[SLICE_DIM]];    //TODO: pretty sure this is going to fail if plane is in a negative quadrant
        m_sliceStart += m_directionSign[m_dimensionOrder[SLICE_DIM]] * m_cellDimensions[m_dimensionOrder[SLICE_DIM]];

        //swap current point buffers
        m_currentPointList = !m_currentPointList;

        //count down all active edge counts
        //a copy is needed because addPoint can't be modifying the same list that's being updated, horrible bugs happen
        std::unordered_map<size_t, unsigned int> activeEdgesCopy(m_activeEdges.size());
        
        for(std::unordered_map<size_t, unsigned int>::iterator activeEdgeIter = m_activeEdges.begin(); activeEdgeIter != m_activeEdges.end(); activeEdgeIter++) {
            size_t edgeIndex = activeEdgeIter->first;
            unsigned int& activeEdgeCountdown = activeEdgeIter->second;
            
            if(--activeEdgeCountdown == 0) {    //discard this edge, and add its destination point
                m_debugger.m_discarededEdges.insert(edgeIndex);

                addPoint(m_activeEdgeDestPoint.at(edgeIndex), activeEdgesCopy);
            }
            else {
                activeEdgesCopy[edgeIndex] = activeEdgeCountdown; //keep this edge countdown
            }
        }

        m_activeEdges.swap(activeEdgesCopy);

        return setupSlice();
    }

    /**
    Sets up the current slice to be 2D rasterized
    */
    bool setupSlice() {
        //clear other side point buffer
        m_pointList[!m_currentPointList].clear();
        m_debugger.m_pointListMissingDim[!m_currentPointList].clear();

        //find intersection of active edges against other side of slice and add it to the other points list
        for(std::unordered_map<size_t, unsigned int>::const_iterator activeEdgeIter = m_activeEdges.begin(); activeEdgeIter != m_activeEdges.end(); activeEdgeIter++) {
            size_t activeEdge = activeEdgeIter->first;

            glm::detail::tvec3<W> dest;

            bool intersection = m_slicePlane.lineIntersection(m_meshEdgeList->m_points[m_meshEdgeList->m_edges[activeEdge].m_point[0]], m_meshEdgeList->m_points[m_meshEdgeList->m_edges[activeEdge].m_point[1]], dest);
            assert(intersection);   //this assert definitely helps find some nasty bugs

            m_pointList[!m_currentPointList].push_back(glm::detail::tvec2<W>(dest[m_dimensionOrder[X_DIM]], dest[m_dimensionOrder[Y_DIM]]));
            m_debugger.m_pointListMissingDim[!m_currentPointList].push_back(dest[m_dimensionOrder[SLICE_DIM]]);
        }

        /////
        //find convex hull of 2D projected slice points using modified monotone chain algorithm
        //this algorithm is nice because I also have the 2 sides of the polygon I'm about to rasterize in 2D
        //

        //sort points in order of second highest magnitude dimension
        struct PointComparator {
            inline PointComparator(ConvexMeshIterator& convexMeshIterator)
                : m_convexMeshIterator(convexMeshIterator)
            {}

            inline bool operator() (glm::detail::tvec2<W>* ptA, glm::detail::tvec2<W>* ptB) {
                for(uint8_t dimension = 1; dimension != 0; dimension = 0) {
                    W a = (*ptA)[dimension];
                    W b = (*ptB)[dimension];
                    
                    if(m_convexMeshIterator.m_directionSign[m_convexMeshIterator.m_dimensionOrder[dimension]] > 0) {
                        if(a < b) {
                            return true;
                        } 
                        else if(a > b) {
                            return false;
                        }
                    }
                    else {
                        if(a > b) {
                            return true;
                        }
                        else if(a < b) {
                            return false;
                        }
                    }
                }

                return false;
            }

            ConvexMeshIterator& m_convexMeshIterator;
        };

        std::vector<glm::detail::tvec2<W>*> sortedPoints;

        for(uint8_t list = 0; list < 2; list++) {
            for(uint8_t point = 0; point < m_pointList[list].size(); point++) {
                sortedPoints.push_back(&m_pointList[list][point]);
            }
        }

        std::sort(sortedPoints.begin(), sortedPoints.end(), PointComparator(*this));
        m_debugger.m_sortedSlicePoints = sortedPoints;

        //find the convex hull and split it into lists of edges for 1 side and the other
        uint8_t secondaryDimension = m_dimensionOrder[Y_DIM];
        uint8_t tertiaryDimension = m_dimensionOrder[X_DIM];

        {
            int8_t convexSign = m_directionSign[secondaryDimension] * m_directionSign[tertiaryDimension];

            //"right" edge
            convexHull(sortedPoints.begin(), sortedPoints.end(), m_sliceRasterizeEdges[RIGHT_SIDE], convexSign);

            //"left" edge
            convexHull(sortedPoints.rbegin(), sortedPoints.rend(), m_sliceRasterizeEdges[LEFT_SIDE], convexSign);
        }

        //setup the initial row
        m_activeSliceEdgeIndex[RIGHT_SIDE] = 0;
        m_activeSliceEdgeIndex[LEFT_SIDE] = 0;

        //sets up the min vertical
        int rowNum = gridDistance(m_spaceRange.m_min[secondaryDimension], m_sliceRasterizeEdges[RIGHT_SIDE][0]->y, secondaryDimension);      
        m_currentPosition[secondaryDimension] = m_range.m_min[secondaryDimension] + rowNum * m_directionSign[secondaryDimension];

        m_debugger.m_sliceMin.y = m_currentPosition[secondaryDimension];

        m_lineBottom = m_spaceRange.m_min[secondaryDimension] + rowNum * m_directionSign[secondaryDimension] * m_cellDimensions[secondaryDimension];
        m_lineTop = m_lineBottom + m_directionSign[secondaryDimension] * m_cellDimensions[secondaryDimension];

        //sets up the max vertical
        rowNum = gridDistance(m_spaceRange.m_min[secondaryDimension], m_sliceRasterizeEdges[RIGHT_SIDE][m_sliceRasterizeEdges[RIGHT_SIDE].size() - 1]->y, secondaryDimension);
        m_sliceMax.y = m_range.m_min[secondaryDimension] + rowNum * m_directionSign[secondaryDimension];
        
        setupSliceHelper<LEFT_SIDE>();

        m_debugger.m_messages.push_back(" ");

        setupSliceHelper<RIGHT_SIDE>();

        m_debugger.m_messages.push_back(" ");
        m_debugger.m_messages.push_back(" ");
                
        return true;
    }

    template <bool isLeftSide>
    inline void setupSliceHelper() {
        assert(m_activeSliceEdgeIndex[isLeftSide] + 1 < m_sliceRasterizeEdges[isLeftSide].size());
        m_activeSliceEdgeOutward[isLeftSide] = geq(m_sliceRasterizeEdges[isLeftSide][1]->x, m_sliceRasterizeEdges[isLeftSide][0]->x, -m_directionSign[m_dimensionOrder[X_DIM]]);

        m_debugger.m_messages.push_back(formatString("%s side initially %s", isLeftSide ? "left" : "right", m_activeSliceEdgeOutward[isLeftSide] ? "outward" : "inward"));

        if(m_activeSliceEdgeOutward[isLeftSide]) {           //going outward
            if(preAdvanceOutwardLine<isLeftSide>()) {
                while(advanceOutwardLine<isLeftSide>()) {}
            }
        }
        else {                                              //going inward
            m_debugger.m_messages.push_back(formatString("Set %s side inward edge first line point", isLeftSide ? "left" : "right"));
            //set first point of line as farthest column
            setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][0]->x);

            while(advanceInwardLine<isLeftSide>()) {}
        }
    }
    
    void advanceRow() {
        //advance row locations
        m_lineBottom = m_lineTop;
        m_lineTop += m_directionSign[m_dimensionOrder[Y_DIM]] * m_cellDimensions[m_dimensionOrder[Y_DIM]];

        m_currentPosition[m_dimensionOrder[Y_DIM]] += m_directionSign[m_dimensionOrder[Y_DIM]];
        
        advanceRowHelper<RIGHT_SIDE>();
        advanceRowHelper<LEFT_SIDE>();

        m_debugger.m_messages.push_back(" ");
        m_debugger.m_messages.push_back(" ");
    }

    template <bool isLeftSide>
    inline void advanceRowHelper() {
        m_debugger.m_messages.push_back(formatString("Advance row helper %s side", isLeftSide ? "left" : "right"));

        //count down the active edges
        m_activeSliceEdges[isLeftSide]--;
        m_debugger.m_messages.push_back(formatString("%u more rows until add slice point", m_activeSliceEdges[RIGHT_SIDE]));

        //if reached the end of the current edge being rasterized
        if(m_activeSliceEdges[isLeftSide] == 0) {
            m_debugger.m_messages.push_back("Advance Row Add Slice Point");            
            if(m_activeSliceEdgeOutward[isLeftSide]) {          //if outward
                //if last line
                if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
                    m_debugger.m_messages.push_back("Advance outer Row last line, so setting other point as intersection");

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
                    m_debugger.m_messages.push_back("Advance inner Row last line, so setting other point as intersection");
                    m_activeSliceEdges[isLeftSide] = 1;
                }
                else {
                    while(advanceInwardLine<isLeftSide>()) {}
                }
            }
        }
        else {
            m_debugger.m_messages.push_back(formatString("Advance Side intersection %u more rows until add slice point", m_activeSliceEdges[isLeftSide]));

            //make sure the edge indexes are pointing to no more than the second to last point in each edge list 
            assert(m_activeSliceEdgeIndex[isLeftSide] + 1 < m_sliceRasterizeEdges[isLeftSide].size());

            //find intersection with either the top of the slice or the bottom depending on if the line is inward or outward
            W xIntercept = lineInterceptX(*m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]], 
                *m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1], 
                m_activeSliceEdgeOutward[isLeftSide] ? m_lineTop : m_lineBottom);

            setSliceRowPoint<isLeftSide>(xIntercept);
        }

        m_debugger.m_messages.push_back(" ");
    }

    template <bool isLeftSide>
    inline void setSliceRowPoint(W worldX) {
        //find which column that point is in
        P column = m_range.m_min[m_dimensionOrder[X_DIM]] 
            + gridDistance(m_spaceRange.m_min[m_dimensionOrder[X_DIM]], worldX, m_dimensionOrder[X_DIM]) 
            * m_directionSign[m_dimensionOrder[X_DIM]];
                
        if(isLeftSide) {
            //current position is now this left side column
            m_currentPosition[m_dimensionOrder[X_DIM]] = column;
        }
        else {
            //row maximum is now this column
            m_sliceMax.x = column;
        }
                
        //set up the debugger stuff
        if(isLeftSide) {
            m_debugger.m_leftSlicePoint = worldX;
            m_debugger.m_sliceMin.x = column;
            m_debugger.m_rasterizedCells.push_back(m_cellDimensions * vec3cast<P, W>(m_currentPosition) + m_cellDimensions * 0.5f * vec3cast<int8_t, W>(m_directionSign));
        }
        else {
            m_debugger.m_rightSlicePoint = worldX;
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
        m_debugger.m_messages.push_back(formatString("advance %s outward line start edge index %u", isLeftSide ? "left" : "right", m_activeSliceEdgeIndex[isLeftSide]));
        
        //if last line
        if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
            m_debugger.m_messages.push_back("last outward line, setting point B of this line as farthest column point");

            //set point B as the farthest point
            setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->x);

            return false;
        }

        //advance to the next line
        m_activeSliceEdgeIndex[isLeftSide]++;

        m_debugger.m_messages.push_back(formatString("incremented edge index now %u", m_activeSliceEdgeIndex[isLeftSide]));

        //check what direction next line is going in
        m_activeSliceEdgeOutward[isLeftSide] = geq(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->x, 
            m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->x, 
            isLeftSide 
                ? -m_directionSign[m_dimensionOrder[X_DIM]] 
                : m_directionSign[m_dimensionOrder[X_DIM]]);
        
        //find which row the other point is in relative to this row
        int rowNum = gridDistance(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->y,
            m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->y, 
            m_dimensionOrder[Y_DIM]);

        m_debugger.m_messages.push_back(formatString("%u rows until next point",  rowNum));

        //if inward
        if(!m_activeSliceEdgeOutward[isLeftSide]) {
            m_debugger.m_messages.push_back("next edge inward, set this line's pointA as max");

            //set point B as the farthest point
            setSliceRowPoint<isLeftSide>(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->x);
        }

        //if the current line doesn't end in the same row
        if(rowNum > 0) {
            m_debugger.m_messages.push_back("done advancing outward line");

            m_activeSliceEdges[isLeftSide] = rowNum;

            //if outward, clip against row top and set as the farthest column
            if(m_activeSliceEdgeOutward[isLeftSide]) {
                m_debugger.m_messages.push_back("ending advancing as outward line, clip against top");

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
                m_debugger.m_messages.push_back("transitioned from outward to inward advance loop");
                while(advanceInwardLine<isLeftSide>()) {}
                return false;
            }
            else {
                m_debugger.m_messages.push_back("continuing outward advance");
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
        m_debugger.m_messages.push_back(formatString("pre-advance %s outward line start edge index %u", isLeftSide ? "left" : "right", m_activeSliceEdgeIndex[isLeftSide]));
                
        //find which row the other point is in relative to this row
        int rowNum = gridDistance(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->y, 
            m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->y,
            m_dimensionOrder[Y_DIM]);

        m_debugger.m_messages.push_back(formatString("%u rows until next point",  rowNum));
        
        //if the current line doesn't end in the same row
        if(rowNum > 0) {
            m_debugger.m_messages.push_back("done advancing outward line");

            m_activeSliceEdges[isLeftSide] = rowNum;

            //if outward, clip against row top and set as the farthest column            
            m_debugger.m_messages.push_back("ending advancing as outward line, clip against top");

            //find intersection against row top
            W xIntercept = lineInterceptX(*m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]], 
                *m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1], m_lineTop);

            //set this as the max point
            setSliceRowPoint<isLeftSide>(xIntercept);

            return false;
        }
        else {
            m_debugger.m_messages.push_back("continuing outward advance");
            return true;
        }
    }

    /**
    When rasterizing a 2D cross section slice, this advances to the next segment being rasterized.
    Only call this if the current line segment is going inward.
    Initializes the row bounds of the slice being rasterized.

    @tparam isLeftSide Whether or not being called on the right side or the left side of the polygon beign rasterized
    @return Whether or not advancing needs to continue if the next line segment's second point is still within the same row.
    */
    template <bool isLeftSide>
    inline bool advanceInwardLine() {
        m_debugger.m_messages.push_back(formatString("advance %s inward line start edge index %u", isLeftSide ? "left" : "right", m_activeSliceEdgeIndex[isLeftSide]));

        //if last line
        if(m_activeSliceEdgeIndex[isLeftSide] == m_sliceRasterizeEdges[isLeftSide].size() - 2) {
            m_debugger.m_messages.push_back("last inward line");

            return false;
        }

        //advance to the next line
        m_activeSliceEdgeIndex[isLeftSide]++;

        m_debugger.m_messages.push_back(formatString("incremented edge index now %u", m_activeSliceEdgeIndex[isLeftSide]));
                
        //find which row the other point is in relative to this row
        int rowNum = gridDistance(m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide]]->y,
            m_sliceRasterizeEdges[isLeftSide][m_activeSliceEdgeIndex[isLeftSide] + 1]->y,
            m_dimensionOrder[Y_DIM]);

        m_debugger.m_messages.push_back(formatString("%u rows until next point",  rowNum));

        //if the current line doesn't end in the same row
        if(rowNum > 0) {
            m_debugger.m_messages.push_back("done advancing inward line");

            m_activeSliceEdges[isLeftSide] = rowNum;

            return false;
        }
        else {
            m_debugger.m_messages.push_back("continuing inward advance");
            return true;
        }
    }
    
    ///Range of 3D cells being rasterized
    Box<P> m_range;

    ///The current grid cell the iterator is pointing to
    glm::detail::tvec3<P> m_currentPosition;

    ///The range in physical space, derived from m_range * m_cellDimensions
    Box<W> m_spaceRange;

    ///Dimensions of the cells 
    glm::detail::tvec3<W> m_cellDimensions;

    ///Sorted dimension indeces by order of magnitude of the direction vector the view frustum faces
    glm::detail::tvec3<uint8_t> m_dimensionOrder;
    ///Sign of each dimension in the direction vector the view frustum faces
    glm::detail::tvec3<int8_t> m_directionSign;
    
    ///Edges that have already been processed by the rasterizing algorithm and are either discarded or active
    bool * m_isEdgeChecked;

    ///Edges that are currently being processed by the rasterizing algorithm, mapping of edge to how many slices until the other edge end
    std::unordered_map<size_t, unsigned int> m_activeEdges;

    ///The active edge point index that the slice is going towards
    std::unordered_map<size_t, size_t> m_activeEdgeDestPoint;

    ///The mesh edge list itself
    const MeshEdgeList<W>* m_meshEdgeList;

    ///The points that make up the slice of the mesh currently being rasterized
    std::vector<glm::detail::tvec2<W> > m_pointList[2];

    ///Index of which of the points lists is the one from the previous slice front side
    bool m_currentPointList;

    ///The current backside of the mesh slice
    W m_sliceStart;
    ///The plane representing the sliceEnd so you can clip edges against it 
    Plane<W> m_slicePlane;

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

    ///For debugging, remove later
    Debugger m_debugger;
};

#endif