#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GLYPH_CONTAINER_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GLYPH_CONTAINER_TRACK__HPP

/* $Id: glyph_container_track.hpp 44204 2019-11-14 20:16:52Z filippov $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <gui/widgets/seq_graphic/layout_track_impl.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CGlyphContainer - a glyph container in the form of layout track.
/// An object of CGlyphContainer, itself, is a layout track.  It contains a
/// set of CSeqGlyphs. The track container is not resposible of creating
/// any contained glyphs. The purpose of CGlyphContainer to form a
/// group of glyphs to make them behave like a single track: movable,
/// collapsible, and closible.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGlyphContainer : public CLayoutTrack
{
public:
    typedef vector< CRef<CCommentGlyph> >   TComments;
    typedef set<string>         THighlights;
    typedef map<string, THighlights> TKeyHighlights;

    CGlyphContainer(CRenderingContext* r_cntx);
    virtual ~CGlyphContainer();

    /// @name CLayoutTrack interface implementation.
    /// @{
    //virtual void UpdateConfig();
    /// @}

    /// @name CSeqGlyph interface implementation
    /// @{
    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);
    virtual bool HitTestHor(TSeqPos x, const CObject *obj);
    virtual bool Intersects(const TModelRect& rect, TConstObjects& objs) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual void Update(bool layout_only);
    virtual bool Accept(IGlyphVisitor* visitor);
    /// @}




    /// Append a layout object to the end.
    void Add(CSeqGlyph* obj);
    void SetObjects(const CLayoutGroup::TObjectList& objs);
    
    /// set comments.
    /// @param comment_str is a string consisting of one or more
    /// comments in the form of: comment1|pos1|comment2|pos2...
    /// Each comment has to come with a position, and the comment
    /// and position alternate in the comment_str separated by '|'.
    virtual void SetComments(const string& comment_str);
    /// set a list of 'object' need to be highlighted.
    /// @param hl_str is a string consisting of one or more
    ///  labels (track-specific) representing objects
    virtual void SetHighlights(const string& hl_str);

    void SetCommentGlyphs(const CSeqGlyph::TObjects& objs, CRef<CCommentConfig> config);
    void ResetCommentGlyph() { m_Comments.clear(); }

    void SetLabelGlyphs(const CSeqGlyph::TObjects& objs, CRef<CCommentConfig> config);
    void ResetLabelGlyph() { m_Labels.clear(); }

    const THighlights& GetHighlights() const;
    const TKeyHighlights&  GetHighlightsByKey() const;

    CLayoutGroup& SetGroup();
    const CLayoutGroup& GetGroup() const;
    CLayoutGroup::TObjectList& SetChildren();
    const CLayoutGroup::TObjectList& GetChildren() const;

    bool  Empty() const;

    /// Set policy on how to deploy the layout of its children.
    void SetLayoutPolicy(ILayoutPolicy* policy);

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

protected:
    virtual void x_UpdateBoundingBox();

    // Get message associated with contained histogram glyph (if any). This is a message
    // that describes current configuration (linear, log etc) and does not overlap with
    // m_Msg so we can't use that.
    virtual string x_GetHistMsg() const;

    /// @name CLayoutTrack pure virtual method implementation.
    /// @{
    virtual void x_RenderContent() const;
    virtual bool x_Empty() const;
    virtual void x_ClearContent();
    /// @}

private:
    /// prohibited copy constructor and assignment operator.
    CGlyphContainer(const CGlyphContainer&);
    CGlyphContainer& operator=(const CGlyphContainer&);

    void x_UpdateComments(TComments& comments);
    void x_UpdateCommentTargetPos(TComments& comments);

protected:
    CLayoutGroup          m_Group;
    CRef<CSimpleLayout>   m_Simple;
    CRef<CInlineLayout>   m_Inline;
    CRef<CLayeredLayout>  m_Layered;

private:
    TComments       m_Comments;
    /// object labels shown at any arbitrary position.
    /// Currently only used in Gene Model track.
    TComments       m_Labels;
    THighlights     m_Highlights;
    TKeyHighlights  m_KeyHighlights;
    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
// CLayoutTrack inline method implmentation
///////////////////////////////////////////////////////////////////////////////
inline
void CGlyphContainer::Add(CSeqGlyph* obj)
{ m_Group.PushBack(obj); }

inline
void CGlyphContainer::SetObjects(const CLayoutGroup::TObjectList& objs)
{ m_Group.Set(objs); }

inline
const CGlyphContainer::THighlights& CGlyphContainer::GetHighlights() const
{ return m_Highlights; }

inline
const CGlyphContainer::TKeyHighlights& CGlyphContainer::GetHighlightsByKey() const
{
    return m_KeyHighlights;
}

inline
CLayoutGroup& CGlyphContainer::SetGroup()
{ return m_Group; }

inline
const CLayoutGroup& CGlyphContainer::GetGroup() const
{ return m_Group; }

inline
CLayoutGroup::TObjectList& CGlyphContainer::SetChildren()
{ return m_Group.SetChildren(); }

inline
const CLayoutGroup::TObjectList& CGlyphContainer::GetChildren() const
{ return m_Group.GetChildren(); }

inline
bool  CGlyphContainer::Empty() const
{ return x_Empty(); }

inline
void CGlyphContainer::SetLayoutPolicy(ILayoutPolicy* policy)
{
    m_Group.SetLayoutPolicy(policy);
}

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___GLYPH_CONTAINER_TRACK__HPP
