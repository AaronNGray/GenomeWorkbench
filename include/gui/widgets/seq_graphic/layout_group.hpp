#ifndef GUI_WIDGET_SEQ_GRAPHICS___CLAYOUTGROUP__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___CLAYOUTGROUP__HPP

/*  $Id: layout_group.hpp 44204 2019-11-14 20:16:52Z filippov $
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

#include <corelib/ncbistr.hpp>
#include <gui/gui_export.h>
#include <gui/opengl/gltypes.hpp>
#include <gui/widgets/seq_graphic/layout_policy.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_enums.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/gui.hpp>
#include <gui/utils/rgba_color.hpp>

BEGIN_NCBI_SCOPE

class CBoundaryParams;

///////////////////////////////////////////////////////////////////////////////
/// CLayoutGroup is a container of CSeqGlyphs (layout objects).  Itself is
/// one type of many layout objects.  It simply takes a set of layout objects,
/// and generates layout based on the given layout policy.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CLayoutGroup: public CSeqGlyph
{
public:
    typedef ILayoutPolicy::TObjectList TObjectList;

    CLayoutGroup(ELinkedFeatDisplay LinkedFeat = ELinkedFeatDisplay::eLFD_Default);
    virtual ~CLayoutGroup(void) {}

    /// @name ICompostion interface implementation.
    /// @{
    /// Return the object list.
    const TObjectList& GetChildren() const;
    TObjectList& GetChildren();

    /// Get total number of children.
    size_t GetChildrenNum() const;

    /// Get the layout object at index 'idx'.
    CConstRef<CSeqGlyph> GetChild(int idx) const;

    /// Get the layout object at index 'idx'.
    CRef<CSeqGlyph> GetChild(int idx);

    /// Get the child index.
    /// @return -1 if there is no such object.
    int GetIndex(const CSeqGlyph* obj) const;

    /// Remove a layout object.
    bool Remove(CSeqGlyph* obj);

    /// Replace a child at index 'idx'.
    bool Replace(CSeqGlyph* obj, int idx);

    /// Insert a layout object before the given index.
    /// If index is out of bound, the object is added to the end.
    void Insert(int at, CSeqGlyph* obj);

    /// Insert a layout object before the given layout object.
    /// If no matched at_obj found, the object will be added to the end.
    void Insert(const CSeqGlyph* at_obj, CSeqGlyph* obj);

    /// Insert a layout object after the given layout object.
    /// If no matched at_obj found, the object will be added to the end.
    void InsertAft(const CSeqGlyph* at_obj, CSeqGlyph* obj);

    /// Append a layout object to the end.
    void PushBack(CSeqGlyph* obj);

    void PushFront(CSeqGlyph* obj);

    void Set(const TObjectList& objs);

    void Append(TObjectList& objs);

    TObjectList& SetChildren();

    void Clear();
    /// @}

    /// @name CSeqGlyph interface implementation
    /// @{
    virtual void Update(bool layout_only);
    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);
    virtual bool HitTestHor(TSeqPos x, const CObject *obj);
    virtual bool Intersects(const TModelRect& rect, TConstObjects& objs) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool Accept(IGlyphVisitor* visitor);
    /// @}

    

    /// Set composition boundary parameters.
    /// The settings is not required. It will only be used when there is a
    /// need to show the bounding box for this composition.
    void SetConfig(const CBoundaryParams* conf);

    /// Set policy on how to deploy the layout of its children.
    void SetLayoutPolicy(ILayoutPolicy* policy);

    /// update group's bounding box only.
    void UpdateBoundingBox();

    /// update group's bounding box only, and its
    /// parent layout.
    void UpdateLayout();

    /// Return true if there are 0 or 1 children, or all children are features
    /// of the same type (e.g. all introns)
    bool AllChildrenSameType() const;

    /// master glyph has some special meaning (e.g. represents feature that is a parent of all other features
    /// represented by glyphs of the layout)
    /// there is no requirement for a layout to have a master object but if there is one then
    /// it is supposed to be among children (calling SetAsMaster() does _not_ insert it to the child list)
    void SetAsMaster(CRef<CSeqGlyph> master) { m_MasterGlyph = master; }
    bool IsMaster(const CSeqGlyph* glyph) const { return m_MasterGlyph.GetPointerOrNull() == glyph; }
    void HideMaster(bool hidden = true) { m_HideMaster = hidden; }
    bool IsMasterHidden() const { return m_HideMaster; }

    // enable these methods to put a breakpoint into
//    virtual void SetHeight(TModelUnit h) {
//        CSeqGlyph::SetHeight(h);
//    };
//    virtual TModelUnit GetHeight() const {
//        return CSeqGlyph::GetHeight();
//    };

    /// set "RelatedGlyphSelected" in CLayoutGroup-based glyph hierarchies
    /// everywhere in the hierarchy containing the given glyph
    static void PropagateRelatedSelection(bool isSelected, CSeqGlyph* pGlyph, CSeqGlyph* pGlyphFrom = NULL);

    void SetLinkedFeat(ELinkedFeatDisplay LinkedFeat) { m_LinkedFeat = LinkedFeat; }
    ELinkedFeatDisplay GetLinkedFeat() const { return m_LinkedFeat; }


protected:
    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    /// override x_DrawHighlight() to do nothing.
    virtual void x_DrawHighlight() const {}
    /// @}

    void x_DrawBoundary() const;

    /// determines whether the glyph should be drawn or not at all (currently children of unselected parents when m_LinkedFeat
    /// is set to "Expandable")
    bool x_isDrawn() const;
    bool x_HasExpandedChildren() const;
private:
    CIRef<ILayoutPolicy>    m_Policy;
    TObjectList             m_Children;
    CConstRef<CBoundaryParams>   m_Config;   ///< boundary settings
    CRef<CSeqGlyph> m_MasterGlyph;
    bool            m_HideMaster;

    /// linkage mode for the track where this glyph is residing
    ELinkedFeatDisplay m_LinkedFeat;
};


///////////////////////////////////////////////////////////////////////////////
/// IBoundaryParams
///
class CBoundaryParams : public CObject
{
public:
    CBoundaryParams()
        : m_ShowBoundary(true)
        , m_ShowBackground(false)
        , m_FgColor(0.0f, 0.0f, 0.0f, 0.0f)
        , m_BgColor(1.0f, 1.0f, 1.0f, 1.0f)
        , m_LineWidth(1.0)
        , m_Padding(2)
    {}

    CBoundaryParams(bool show_boundary, bool show_bg, const CRgbaColor& fg,
        const CRgbaColor& bg, TModelUnit w, int padding = 2)
        : m_ShowBoundary(show_boundary)
        , m_ShowBackground(show_bg)
        , m_FgColor(fg)
        , m_BgColor(bg)
        , m_LineWidth(w)
        , m_Padding(padding)
    {}

    /// @name accessors
    /// @{
    bool ShowBoundary() const               { return m_ShowBoundary; }
    bool ShowBackground() const             { return m_ShowBackground; }
    const CRgbaColor& GetBgColor() const    { return m_BgColor; }
    const CRgbaColor& GetFgColor() const    { return m_FgColor; }
    TModelUnit  GetLineWidth() const        { return m_LineWidth; }
    int GetPadding() const                  {return m_Padding; }
    /// @}

    /// @name mutators
    /// @{
    void SetShowBoundary(bool flag)     { m_ShowBoundary = flag; }
    void SetShowBackground(bool flag)   { m_ShowBackground = flag; }
    void SetBgColor(const CRgbaColor& flag)   { m_BgColor = flag; }
    void SetFgColor(const CRgbaColor& flag)   { m_FgColor = flag; }
    void SetLineWidth(TModelUnit flag)  { m_LineWidth = flag; }
    void SetPadding(int p) { m_Padding = p; }
    /// @}

private:
    bool        m_ShowBoundary;
    bool        m_ShowBackground;
    CRgbaColor  m_FgColor;
    CRgbaColor  m_BgColor;
    TModelUnit  m_LineWidth;
    int         m_Padding;
};

///////////////////////////////////////////////////////////////////////////////
/// CLayoutGroup inline methods
///
inline
const CLayoutGroup::TObjectList& CLayoutGroup::GetChildren() const
{
    return m_Children;
}
inline
CLayoutGroup::TObjectList& CLayoutGroup::GetChildren()
{
    return m_Children;
}

inline
size_t CLayoutGroup::GetChildrenNum() const
{
    return m_Children.size();
}

inline
void CLayoutGroup::SetConfig(const CBoundaryParams* conf)
{
    m_Config.Reset(conf);
}

inline
void CLayoutGroup::SetLayoutPolicy(ILayoutPolicy* policy)
{
    m_Policy.Reset(policy);
}

inline
void CLayoutGroup::PushBack(CSeqGlyph* obj)
{
    m_Children.push_back(CRef<CSeqGlyph>(obj));
    obj->SetParent(this);
    if (m_Context) obj->SetRenderingContext(m_Context);
}

inline
void CLayoutGroup::PushFront(CSeqGlyph* obj)
{
    m_Children.push_front(CRef<CSeqGlyph>(obj));
    obj->SetParent(this);
    if (m_Context) obj->SetRenderingContext(m_Context);
}

inline
void CLayoutGroup::Set(const TObjectList& objs)
{
    m_Children = objs;
    NON_CONST_ITERATE(TObjectList, iter, m_Children) {
        (*iter)->SetParent(this);
        if (m_Context) (*iter)->SetRenderingContext(m_Context);
    }
}

inline
void CLayoutGroup::Append(TObjectList& objs)
{
    m_Children.insert(m_Children.end(), objs.begin(), objs.end());
    NON_CONST_ITERATE(TObjectList, iter, objs) {
        (*iter)->SetParent(this);
        if (m_Context) (*iter)->SetRenderingContext(m_Context);
    }
}

inline
CLayoutGroup::TObjectList& CLayoutGroup::SetChildren()
{
    return m_Children;
}

inline
void CLayoutGroup::Clear()
{
    m_Children.clear();
}

inline
void CLayoutGroup::UpdateBoundingBox()
{
    x_UpdateBoundingBox();
}

inline
void CLayoutGroup::UpdateLayout()
{
    x_OnLayoutChanged();
}

END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___CLAYOUTGROUP__HPP
