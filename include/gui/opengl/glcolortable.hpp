#ifndef GUI_OPENGL___GL_COLOR_TABLE__HPP
#define GUI_OPENGL___GL_COLOR_TABLE__HPP

/*  $Id: glcolortable.hpp 42537 2019-03-19 19:27:54Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *    Incremental and mutually contrastive color table generator.
 */


#include <corelib/ncbiobj.hpp>

#include <gui/opengl.h>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/i3dtexture.hpp>

#include <algorithm>
#include <math.h>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CRgbaGradColorTable
/// Provides a storage for colors (to eliminate color creation overhead) and
/// Functions for initializing the table.
class NCBI_GUIOPENGL_EXPORT CRgbaGradColorTable : public CObject
{
public:
    CRgbaGradColorTable();
    CRgbaGradColorTable(const CRgbaGradColorTable &rhs);
    CRgbaGradColorTable(int size);

    ~CRgbaGradColorTable();

    CRgbaGradColorTable& operator=(const CRgbaGradColorTable& rhs);

    size_t  GetSize() const;
    void    SetSize(size_t size);

    inline CRgbaColor&   GetColor(size_t i);
    inline const CRgbaColor&   GetColor(size_t i) const;

    inline CRgbaColor&   operator[](int i);
    inline const CRgbaColor&   operator[](int i) const;

    /// On exact match, returns true with index in idx, false otherwise
    bool FindColor(const CRgbaColor& c, size_t& idx) const;

    /// Return index of color with minimum color disance (as defined in CRgbaColor) from c.
    /// If the table is empty the return value will be >=2 as sqrt(3.0) is max color distance.
    float FindClosestColor(const CRgbaColor& c, size_t& idx) const;

    /// returns color index by the normalized value; normalized values is a value
    /// in the [0.0, 1.0] range that is mapped to the index range of color table [0, size-1]
    inline int  GetIndex(double norm) const;

    /// Empty color table. 
    void ClearColors();

    /// Add a single color to the table
    size_t AddColor(const CRgbaColor& c);

    /// initialize the whole table with gradient colors in [start_c, end_c] range
    void    FillGradient(const CRgbaColor& start_c, const CRgbaColor& end_c);

    /// initialize part of the table specified by the start index "start_i" and
    /// the number of element "n" with gradient colors in [start_c, end_c] range
    void    FillGradient(int start_i, int n,
                         const CRgbaColor& start_c,
                         const CRgbaColor& end_c);

    /// initialize part of the table specified by the start index "start_i" and
    /// the number of element "n" with gradient colors. First half of the table
    /// is initialized with [start_c, mid_c], the second part with [mid_c, end_c]
    void    FillGradient(int start_i, int n,
                         const CRgbaColor& start_c,
                         const CRgbaColor& mid_c,
                         const CRgbaColor& end_c);

    ///
    /// OpenGL parameters and operations to allow color table to (also) be stored
    /// as a 1D texture.
    ///

    /// Create a 1D texture from the current table and make alpha_levels copies of
    /// the data so that user can access the same color at varying alpha levels
    void LoadTexture(int alpha_levels = 8);

    /// Return object holding the 1D texture
    I3DTexture* GetTexture() { return m_Texture; }

    /// Get position for color with alpha level closest to 'alpha'
    float GetColorTexCoord(short idx, float alpha) const;
    /// Get color given a texture coord
    CRgbaColor GetTexCoordColor(float f) const;

protected:
    vector<CRgbaColor>    m_Colors;

    // Support for (optionally) putting the colors into a 1-dimensional
    // OpenGL texture

    // the texture id
    CIRef<I3DTexture> m_Texture;

    /// Width of the 1D texture (if created).  May be larger than
    /// m_Colors.size() since it must be a power of 2.
    GLsizei m_TextureWidth;

    /// Number of alpha levels stored in the texture
    int m_TextureAlphaLevels;

    /// Maximum size of color texture.  Getting close to or over this number
    /// will reduce color pallette and available alpha levels.
    GLint m_MaxColorTableSize;
};

///////////////////////////////////////////////////////////////////////////////
// inlines

inline CRgbaColor&   CRgbaGradColorTable::GetColor(size_t i)
{
    _ASSERT(i < m_Colors.size());
    return m_Colors[i];
}

inline const CRgbaColor&   CRgbaGradColorTable::GetColor(size_t i) const
{
    _ASSERT(i < m_Colors.size());
    return m_Colors[i];
}


inline CRgbaColor&   CRgbaGradColorTable::operator[](int i)
{
    return GetColor(i);
}


inline const CRgbaColor&   CRgbaGradColorTable::operator[](int i) const
{
    return GetColor(i);
}


inline int  CRgbaGradColorTable::GetIndex(double norm) const
{
    int index = (int) floor(norm * m_Colors.size());
    index = std::min(index, (int) m_Colors.size() - 1);
    return index;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_COLOR_TABLE__HPP
