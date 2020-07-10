#ifndef GUI_WIDGETS_SEQ_GRAPHIC___COMMENT_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___COMMENT_GLYPH__HPP

/*  $Id: comment_glyph.hpp 35878 2016-07-05 18:06:44Z evgeniev $
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
 * File Description:
 */


#include <corelib/ncbiobj.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/utils/rgba_color.hpp>


BEGIN_NCBI_SCOPE

class CCommentConfig;

///////////////////////////////////////////////////////////////////////////////
/// CCommentGlyph -- utility class for having comments in graphical glyphs.
/// For most of cases, a comment glyph may serve as a label for another glyph.
/// In those cases, a comment will be linked to a glyph, and the comment will
/// behave like a delegate of that linked glyph.  Since the linked glyph can
/// be a object-based glyph, so is the comment.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCommentGlyph
    : public CSeqGlyph
    //, public IObjectBasedGlyph
{
public:
    CCommentGlyph(const string& comment, CRef<CSeqGlyph> glyph);
    CCommentGlyph(const string& comment, const TModelPoint& pos, bool repeated = false);

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    // do we need this for comment glyh
    //virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    /// @}

    ///// @name IObjectBasedGlyph interface implement
    ///// @{
    ///// Access the remapped location.
    //const objects::CSeq_loc&  GetLocation(void) const;
    ///// Access the data as a CObject.
    //virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    //virtual void GetObjects(vector<CConstRef<CObject> >& objs) const;
    //virtual bool HasObject(CConstRef<CObject> obj) const;
    //virtual const TIntervals& GetIntervals(void) const;
    ///// @}
    const TModelPoint& GetTargetPos() const;
    void SetTargetPos(const TModelPoint& pos);
    void SetConfig(CConstRef<CCommentConfig> config);
    
    CRef<CSeqGlyph> GetLinkedGlyph();

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

protected:
    CRef<CSeqGlyph> m_LinkedGlyph;      ///< associated glyph
    string          m_Comment;          ///< label
    TModelPoint     m_TargetPos;        ///< target position (if glyph not set)
    bool            m_Repeated;

    CConstRef<CCommentConfig> m_Config;
};

///////////////////////////////////////////////////////////////////////////////
/// CCommentConfig -- help config class for holding a set of settings for 
/// a comment glyph.
class CCommentConfig : public CObject
{
public:
    enum EPosition {
        ePos_Above,
        ePos_Below,
        ePos_Inside,
        ePos_Left,
        ePos_Right,
        ePos_Any
    };

    CCommentConfig()
        : m_RelativePos(ePos_Above)
        , m_LabelColor(0.0f, 0.0f, 0.0f)
        , m_LineColor(0.0f, 0.0f, 1.0f)
        , m_Font(CGlTextureFont::eFontFace_Helvetica, 10)
        , m_ShowBoundary(false)
        , m_ShowConnection(true)
        , m_Centered(true)
    {}

    /// relative position between the comment and target.
    EPosition       m_RelativePos;
    CRgbaColor      m_LabelColor;
    CRgbaColor      m_LineColor;
    CGlTextureFont  m_Font;
    bool            m_ShowBoundary;

    /// display a connection between the comment and target.
    bool            m_ShowConnection;
    /// label is centered against the target.
    bool            m_Centered;
};


///////////////////////////////////////////////////////////////////////////////
///   CCommentGlyph inline methods
///
inline
const TModelPoint& CCommentGlyph::GetTargetPos() const
{
    return m_TargetPos;
}

inline
void CCommentGlyph::SetTargetPos(const TModelPoint& pos)
{
    m_TargetPos = pos;
}

inline
void CCommentGlyph::SetConfig(CConstRef<CCommentConfig> config)
{
    m_Config = config;
}


inline
CRef<CSeqGlyph> CCommentGlyph::GetLinkedGlyph()
{
    return m_LinkedGlyph;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___COMMENT_GLYPH__HPP
