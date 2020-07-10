#ifndef GUI_OPENGL___SPATIAL_HASH2D__HPP
#define GUI_OPENGL___SPATIAL_HASH2D__HPP

/*  $Id: spatialhash2d.hpp 35494 2016-05-13 14:16:41Z evgeniev $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/opengl/glpane.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>

#include <float.h>

BEGIN_NCBI_SCOPE

class CGlModel2D;


////////////////////////////////////////////////////////////////////////////////
/// class CSpatialHash2D
/// Class for simple 2D spatial organization that allows for some collision 
/// checking and visibility determination.
///
/// Not really a hash-table since it seems to make more sense in 2D to go with
/// a dense array rather than a hash table where unuused regions would not
/// have a node in the array.  
///
/// Addressing into the spatial datastructure is done simply by subtracting
/// the lower left position of the bounded area from the point (moving it to 0)
/// and then dividing by the resolution to get an index into the table.
///
template<class T>
class  CSpatialHash2D
{
public:
    /// floating point type for actual (real) positions of data
    typedef CVect2<float> TVecf;
    /// Storage index in table
    typedef CVect2<int> TVeci;
    /// Element type stored in table (e.g. indices or pointers to user data)
    typedef T TElemType;
    /// Storage table type
    typedef vector<TElemType> TTableType;

    CSpatialHash2D();
    virtual ~CSpatialHash2D();

    int GetTableIdx(const TVeci& item) const { return (item.Y()*m_Width + item.X()); }

    void Resize() { if (m_Width*m_Height != (int)m_GridTable.size()) m_GridTable.resize( m_Width*m_Height); }
    void ResizeFast() { if (m_Width*m_Height > (int)m_GridTable.size()) m_GridTable.resize( m_Width*m_Height); }
    void Clear() { for (size_t i=0; i<m_GridTable.size(); ++i)  m_GridTable[i].clear(); }

    TElemType& Get(const TVeci& item) { return m_GridTable[GetTableIdx(item)]; }

    const TElemType& Get(const TVeci& item) const { return m_GridTable[GetTableIdx(item)]; }

    /// Return an empty TElemType if index is out of range
    TElemType GetSafe(const TVeci& item) const;

    /// Return true if the index falls within the current table
    bool InBounds(const TVeci& item) const;

    TTableType& GetTable() { return m_GridTable; }
    const TTableType& GetTable() const { return m_GridTable; }

    void SetMin(TVeci& idx) {
        m_MinIdx = idx; 
        m_Width = m_MaxIdx.X()-m_MinIdx.X() + 1;
        m_Height = m_MaxIdx.Y()-m_MinIdx.Y() + 1;
    }
    void SetMax(TVeci& idx) {
        m_MaxIdx = idx;
        m_Width = m_MaxIdx.X()-m_MinIdx.X() + 1;
        m_Height = m_MaxIdx.Y()-m_MinIdx.Y() + 1;
    }

    void ResizeGrid(const CGlPane& pane,
        float resolution, bool tilex = true, bool tiley = true);

    /// Get min/max indices for current (whole) grid
    const TVeci& GetMin() {return m_MinIdx;}
    const TVeci& GetMax() {return m_MaxIdx;}

    /// Get min/max visible indices for grid (as per UpdateVisibility)
    TVeci GetMinVisIndex() const { return m_VisMinIdx; }
    TVeci GetMaxVisIndex() const { return m_VisMaxIdx; }

    /// Get width/height of grid
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    /// Get min/max model coords spanned by the grid
    TVecf& GetMinPos() {return m_MinPos;}
    TVecf& GetMaxPos() {return m_MaxPos;}

    virtual void UpdateVisibility(const TModelRect& /* r */) {}

    const CVect2<float>& GetScale() const { return m_Scale; }   

protected:

    /// Visible rectangle area from previous call to UpdateVisibility
    CVect2<float> m_PrevVisMin;
    CVect2<float> m_PrevVisMax;

    /// Range of visible indices from previous call to UpdateVisibility
    CVect2<int> m_VisMinIdx;
    CVect2<int> m_VisMaxIdx;

    /// Hashtable that stores the data elements
    TTableType  m_GridTable;

    /// Min/Max indicies in the table
    TVeci m_MinIdx;
    TVeci m_MaxIdx;

    /// Geometric region subdivided by this data structure
    TVecf m_MinPos;
    TVecf m_MaxPos;

    /// Width and heigt of table (m_MaxIdx-m_MinIdx)
    int m_Width;
    int m_Height;

    /// Current scaling factor of model for viewing in x and y.
    /// This is needed to determine where to place elements with pixel-based
    /// sizes in model.
    CVect2<float> m_Scale;

    /// Divisor for dividing model-space resolution into table indices
    float m_ResolutionX;
    float m_ResolutionY;

    float m_ResolutionScaler;
};



template <class T>
CSpatialHash2D<T>::CSpatialHash2D()
    : m_PrevVisMin(0.0f, 0.0f)
    , m_PrevVisMax(0.0f, 0.0f)
    , m_VisMinIdx(0, 0)
    , m_VisMaxIdx(0, 0)
    , m_Scale(1.0f, 1.0f)
    , m_ResolutionX(1.0f)
    , m_ResolutionY(1.0f)
    , m_ResolutionScaler(1.0f)
{
}


template <class T>
CSpatialHash2D<T>::~CSpatialHash2D()
{
}

template <class T>
T CSpatialHash2D<T>::GetSafe(const TVeci& item) const
{ 
    int idx = GetTableIdx(item);

    if (idx < (int)m_GridTable.size() && idx >= 0)
        return m_GridTable[idx]; 

    return TElemType();
}

template <class T>
bool CSpatialHash2D<T>::InBounds(const TVeci& item) const
{ 
    int idx = GetTableIdx(item);

    if (idx < (int)m_GridTable.size() && idx >= 0)
        return true;

    return false;
}
   
template <class T>
void CSpatialHash2D<T>::ResizeGrid(const CGlPane& pane,
    float resolution, bool tilex, bool tiley)
{
    CVect2<float> min_pos(pane.GetModelLimitsRect().Left(), pane.GetModelLimitsRect().Bottom());
    CVect2<float> max_pos(pane.GetModelLimitsRect().Right(), pane.GetModelLimitsRect().Top());

    // Reset size of collision grid based on resolution
    float wh = ((max_pos.X()-min_pos.X()) + (max_pos.Y()-min_pos.Y()))*0.5f;

    // m_ResolutionX/Y are devisors needed to subdivide the model-space into
    // a grid (so they are really the resolution divisors,
    // whereas resolution is the actual target resolution).
    float resxy = m_ResolutionScaler * ceilf(wh/resolution);
    m_ResolutionX = resxy;
    m_ResolutionY = resxy;

    // This is for the case where the user is only going to zoom in x
    // or y - we don't need a large number of tiles in the dimension that
    // is not being zoomed.
    if (!tilex)
        m_ResolutionX = pane.GetVisibleRect().Right() - pane.GetVisibleRect().Left() + 1;
    if (!tiley)
        m_ResolutionY = pane.GetVisibleRect().Top() - pane.GetVisibleRect().Bottom() + 1;

    CVect2<int> min_idx(int(floorf(min_pos[0] / m_ResolutionX)),
                        int(floorf(min_pos[1] / m_ResolutionY)));
    CVect2<int> max_idx(int(floorf(max_pos[0] / m_ResolutionX)),
                        int(floorf(max_pos[1] / m_ResolutionY)));

    SetMax(max_idx);
    SetMin(min_idx);

    m_MinPos = min_pos;
    m_MaxPos = max_pos;

    m_PrevVisMin = min_pos + CVect2<float>(FLT_EPSILON, FLT_EPSILON);
    m_PrevVisMax = max_pos - CVect2<float>(FLT_EPSILON, FLT_EPSILON);

    // Resize and clear spatial grid
    ResizeFast();
    Clear();
}




END_NCBI_SCOPE

#endif //GUI_OPENGL___SPATIAL_HASH2D__HPP
