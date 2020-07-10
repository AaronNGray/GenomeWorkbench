#ifndef GUI_WIDGET_SEQ_GRAPHICS___CNAMEDGROUP__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___CNAMEDGROUP__HPP

/*  $Id:
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
 * Authors:  Liangshou Wu
 *
 */

#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/opengl/gltexturefont.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CTitleGroup is a layout group with a title
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CNamedGroup : public CLayoutGroup
{
public:
    CNamedGroup()
        : m_Indent(0)
        , m_RepeatDist(0)
        , m_RepeatTitle(false)
        , m_ShowTitle(true)
    {}

    CNamedGroup(const string& title)
        : m_Title(title)
        , m_Indent(0)
        , m_RepeatDist(0)
        , m_RepeatTitle(false) 
    {}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    /// @}

    void SetTitle(const string& title);
    void SetTitleColor(const CRgbaColor& color);
    void SetBackgroundColor(const CRgbaColor& color);
    void SetTitleFont(CGlTextureFont* font);
    void SetIndent(int indent);
    void SetRepeatDist(int dist);
    void SetRepeatTitle(bool f);
    void SetShowTitle(bool f);

protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    /// @}

private:
    string               m_Title;
    CRgbaColor           m_TitleColor;
    CRgbaColor           m_BgColor;
    CRef<CGlTextureFont> m_TitleFont;
    int                  m_Indent;
    int                  m_RepeatDist;  ///< didstance between repeated titles
    bool                 m_RepeatTitle; ///< flag indicating if need to repeat title
    /// Used for SViewer mode.
    /// GBench: always True, SViewer: always False (title is rendered on client side).
    bool                 m_ShowTitle;
};


///////////////////////////////////////////////////////////////////////////////
/// CNamedGroup inline methods
///
inline
void CNamedGroup::SetTitle(const string& title)
{ m_Title = title; }

inline
void CNamedGroup::SetTitleColor(const CRgbaColor& color)
{ m_TitleColor = color; }

inline
void CNamedGroup::SetBackgroundColor(const CRgbaColor& color)
{ m_BgColor = color; }

inline
void CNamedGroup::SetTitleFont(CGlTextureFont* font)
{ m_TitleFont.Reset(font); }

inline
void CNamedGroup::SetIndent(int indent)
{ m_Indent = indent; }

inline
void CNamedGroup::SetRepeatDist(int dist)
{ m_RepeatDist = dist; }

inline
void CNamedGroup::SetRepeatTitle(bool f)
{ m_RepeatTitle = f; }

inline
void CNamedGroup::SetShowTitle(bool f)
{ m_ShowTitle = f; }


END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___CNAMEDGROUP__HPP
