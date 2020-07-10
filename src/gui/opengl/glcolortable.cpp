/*  $Id: glcolortable.cpp 42536 2019-03-19 19:13:46Z katargir $
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

#include <ncbi_pch.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <gui/opengl/glresmgr.hpp>


#include <gui/opengl/glutils.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CRgbaGradColorTable

CRgbaGradColorTable::CRgbaGradColorTable()
: m_TextureWidth(0)
, m_TextureAlphaLevels(1)
#ifdef USE_METAL
, m_MaxColorTableSize(16384)
#else
, m_MaxColorTableSize(0)
#endif
{
}

CRgbaGradColorTable::CRgbaGradColorTable(const CRgbaGradColorTable &rhs)
{
    *this = rhs;
}


CRgbaGradColorTable::CRgbaGradColorTable(int size)
: m_TextureWidth(0)
, m_TextureAlphaLevels(1)
#ifdef USE_METAL
, m_MaxColorTableSize(16384)
#else
, m_MaxColorTableSize(0)
#endif
{
    SetSize(size);
}

CRgbaGradColorTable& CRgbaGradColorTable::operator=(const CRgbaGradColorTable& rhs)
{
    m_Colors = rhs.m_Colors;

    // If user was using rhs as a texture, don't force it
    // to be a texture here - have to call create texture again
    // (and don't share textures between objects - they are small..)

    if (m_Texture) {
        m_Texture->Unload();
        m_Texture.Reset();
    }

    m_TextureWidth = 0;
    m_TextureAlphaLevels = 1;

    return *this;
}

CRgbaGradColorTable::~CRgbaGradColorTable()
{
    if (m_Texture)
        m_Texture->Unload();
}


size_t CRgbaGradColorTable::GetSize() const
{
    return m_Colors.size();
}


void CRgbaGradColorTable::SetSize(size_t size)
{
    _ASSERT(size < 0xFFFF);
    m_Colors.resize(size);
}

bool CRgbaGradColorTable::FindColor(const CRgbaColor& c, size_t& idx) const
{
    for (size_t i=0; i<m_Colors.size(); ++i) {
        if (m_Colors[i] == c) {
            idx = i;
            return true;
        }
    }

    return false;
}

float CRgbaGradColorTable::FindClosestColor(const CRgbaColor& c, size_t& idx) const
{
    float min_dist = numeric_limits<float>::max();

    for (size_t i=0; i<m_Colors.size(); ++i) {
        float dist = CRgbaColor::ColorDistance(c, m_Colors[i]);
        if (dist < min_dist) {
            min_dist = dist;
            idx = i;
        }
    }

    return min_dist;
}

void CRgbaGradColorTable::ClearColors()
{
    m_Colors.clear();
}

/// Add a single color to the table.  Do not let table get bigger than maximum texture size.
/// Also, as it gets closer to maximum texture size (4096 or 8192 probably) there will be
/// less room for alpha levels.
size_t CRgbaGradColorTable::AddColor(const CRgbaColor& c)
{
    if (m_MaxColorTableSize == 0)
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxColorTableSize);

    size_t current_size = m_Colors.size();

    if (current_size >= m_MaxColorTableSize) {
        _TRACE("Warning - color table full");
        FindClosestColor(c, current_size);
        return current_size;
    }

    m_Colors.push_back(c);

    return current_size;
}

void CRgbaGradColorTable::FillGradient(const CRgbaColor& start_c, const CRgbaColor& end_c)
{
    if( ! m_Colors.empty())   {
        FillGradient(0, (int)m_Colors.size(), start_c, end_c);
    }
}


void CRgbaGradColorTable::FillGradient(int start_i, int n,
                                     const CRgbaColor& start_c,
                                     const CRgbaColor& end_c)
{
    _ASSERT(start_i >= 0  &&  start_i < (int) m_Colors.size());
    _ASSERT((start_i + n) <= (int) m_Colors.size());
    _ASSERT(n >= 0);

    if(n)   {
        float base_r = start_c.GetRed();
        float base_g = start_c.GetGreen();
        float base_b = start_c.GetBlue();
        float base_a = start_c.GetAlpha();

        float d_r = end_c.GetRed() - base_r;
        float d_g = end_c.GetGreen() - base_g;
        float d_b = end_c.GetBlue() - base_b;
        float d_a = end_c.GetAlpha() - base_a;

        float K = 1.0f / (n - 1);
        for( int i = 0;  i < n;  i++ ) {
            float d = i * K;
            float r = base_r + d * d_r;
            float g = base_g + d * d_g;
            float b = base_b + d * d_b;
            float a = base_a + d * d_a;
            m_Colors[start_i + i].Set(r, g, b, a);
        }
    }
}


void CRgbaGradColorTable::FillGradient(int start_i, int n,
                                     const CRgbaColor& start_c,
                                     const CRgbaColor& mid_c,
                                     const CRgbaColor& end_c)
{
    _ASSERT(start_i >= 0  &&  start_i < (int) m_Colors.size());

    if (n > 0) {
        if ((n & 1) == 0) {
            LOG_POST(Warning << "CRgbaGradColorTable::FillGradient() - \"n\""
                     << "is an even number. Gradient table will not be symmetrical!");
        }
        int half = n / 2;
        // fill the first half
        FillGradient(start_i, half + 1, start_c, mid_c);
        // fill the second half
        FillGradient(start_i + half, n - half, mid_c, end_c);
    }
}

void CRgbaGradColorTable::LoadTexture(int alpha_levels)
{
    // safety first!
    if ( m_Colors.size() == 0 ) {
        _TRACE(" no colors to create texture");
        return;
    }

    if (m_MaxColorTableSize == 0)
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxColorTableSize);

    /// There is at least 1 alpha level - the set of values from m_Color
    if (alpha_levels < 1)
        alpha_levels = 1;

	// Can't go over maximum texture size so if we are too big, reduce alpha levels
	if (alpha_levels * m_Colors.size() > m_MaxColorTableSize) {
		alpha_levels = (int)floorf(((float)m_MaxColorTableSize)/((float)m_Colors.size()));
	}

    m_TextureAlphaLevels = alpha_levels;

    // Get a power-of-2 size for texture that is the smallest
    // power-of-2 size larger than a default minimum size (128)
    // and >= m_Colors.size()*m_TextureAlphalevels
    GLsizei size = (GLsizei)(m_TextureAlphaLevels * (int)m_Colors.size());

    // If we are larger than a normal-sized texture, cut the number of alpha
    // levels (if possible) (This should not normally happen...)
    if (size > 1024*1024) {
        m_TextureAlphaLevels = (1024*1024)/(int)m_Colors.size();
        size = (GLsizei)(m_TextureAlphaLevels * (int)m_Colors.size());
    }

    m_TextureWidth = 128;
    for (int i=0; i<12 && m_TextureWidth < size; ++i)
        m_TextureWidth *= 2;


    // Set up padded buffer of width m_TextureWidth for call to glTexImage1D.
    vector<CRgbaColor> power_of_two_buffer = m_Colors;
    float alpha_factor = 1.0f;
    for (int i=1; i<m_TextureAlphaLevels; ++i) {
        alpha_factor = ((float)(m_TextureAlphaLevels-i))/(float)m_TextureAlphaLevels;

        for (size_t j=0; j<m_Colors.size(); ++j) {
            CRgbaColor c = m_Colors[j];
            c.SetAlpha(c.GetAlpha()*alpha_factor);
            power_of_two_buffer.push_back(c);
        }
    }
    // Add default color to unused space at the end of the buffer
    power_of_two_buffer.resize(m_TextureWidth, CRgbaColor(0.0f, 0.0f, 0.0f, 1.0f));

    if (m_Texture)
        m_Texture->Unload();

    m_Texture.Reset(CGlResMgr::Instance().Create1DRGBATexture(m_TextureWidth, (float*)&power_of_two_buffer[0]));

    CGlUtils::CheckGlError();

    if (m_Texture) {
        m_Texture->SetWrapS(GL_CLAMP);
        m_Texture->SetFilterMin(GL_NEAREST);
        m_Texture->SetFilterMag(GL_NEAREST);
        m_Texture->SetTexEnv(GL_MODULATE);
    }
}

CRgbaColor CRgbaGradColorTable::GetTexCoordColor(float f) const
{

    int color_idx = int(f*(float(m_TextureWidth))-0.5f);
    int alpha_level = color_idx / int(m_Colors.size());
    color_idx = color_idx % int(m_Colors.size());

    if (color_idx >= (int)m_Colors.size()) {
        _TRACE("Texture coord: " << f << " mapped to an invalid color index: " << color_idx);
        return CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    CRgbaColor c = m_Colors[color_idx];

    float alpha_factor = (float(m_TextureAlphaLevels-alpha_level))/(float)m_TextureAlphaLevels;
    c.SetAlpha(c.GetAlpha()*alpha_factor);

    return c;
}

float CRgbaGradColorTable::GetColorTexCoord(short idx, float alpha) const
{
    /// Get alpha level of 0..m_TextureAlphaLevels -1.  Texture map is
    /// stored in order of decreasing alpha, and 'alpha' is in the standard
    /// sense of increasing alpha, so we index for 1.0-alpha.
    int alpha_level = (int)floorf(((1.0f-alpha)*(float)m_TextureAlphaLevels));

    /// should only happen on bad input:
    alpha_level = std::max(alpha_level, 0);
    alpha_level = std::min(alpha_level, m_TextureAlphaLevels-1);

    float level_offset = (float)(m_Colors.size()*(size_t)alpha_level);
    float scaled_width = (float)(m_TextureWidth);

    return (  (((float)idx) + level_offset + 0.5f)/scaled_width );
}

END_NCBI_SCOPE
