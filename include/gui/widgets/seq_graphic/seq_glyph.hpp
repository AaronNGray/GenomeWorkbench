#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQ_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQ_GLYPH__HPP

/* $Id: seq_glyph.hpp 44204 2019-11-14 20:16:52Z filippov $
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
 * Author:  Mike Dicuccio, Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistr.hpp>
#include <util/range.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/tool_tip_info.hpp>
#include <gui/opengl/gltypes.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/widgets/gl/html_active_area.hpp>
#include <objects/seqloc/Seq_loc.hpp>

#include <set>

class wxContextMenuEvent;

BEGIN_NCBI_SCOPE

class CRenderingContext;
class IGlyphVisitor;
class CGlTextureFont;
class ITooltipFormatter;

static const size_t kFadeCount = 10;

///////////////////////////////////////////////////////////////////////////////
/// class CSeqGlyph defines an interface that wraps a rectilinear abstract
/// object.
///
/// CSeqGlyph defines how an axis-aligned rectilinear object appears to a
/// layout algorithm.  The basic concepts supported include:
///  -- querying rectilnear position or interval as a total range
///  -- functors for sorting
///  -- Draw() for rendering the object
///
/// Comments on the coordinate system for a layout object:
///  1. For most of the layout objects, the x coordinate (in horizontal mode) is
///     predetermined and it is in the top sequence coordinate.  So for a given
///     layout object, we define the x origin of the local coord. system the same
///     as the x origin of the 'world' coordinate system.
///  2. And y origin of the local coordinate will always be at the upper left
///     corner (in horizontal mode) of the layout object.
///  3. From a developer's perspective, one can assume the sequence is in horizontal
///     mode with no flipping strand.


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqGlyph : public CObjectEx
{
public:
    typedef vector<CHTMLActiveArea> TAreaVector;
    typedef list< CRef<CSeqGlyph> > TObjects;
    typedef list< CConstRef<CSeqGlyph> > TConstObjects;

    /// Layout renderer interface.
    class IGlyphRenderer {
    public:
        virtual ~IGlyphRenderer() {}
        virtual void Draw(const CSeqGlyph*) const = 0;
    };

    enum ENeighbours {
        eNghbrs_None    = 0x0,
        eNghbrs_Left    = 0x1,
        eNghbrs_Right   = 0x2,
        eNghbrs_Both    = 0x3
    };

    CSeqGlyph();

    virtual ~CSeqGlyph() {
        // for debugging
        // m_isDead = true;
    }

    /// @name Mouse event handlers
    /// Each layout object may have customized handlers on the following mouse events.
    /// The x and y are in 'world' coord system which is the coord. system for the
    /// uppermost layout object.
    /// Return true if event is consumed and false if not
    /// @{
    virtual bool OnLeftDown(const TModelPoint& /*p*/);
    virtual bool OnLeftUp(const TModelPoint& /*p*/);
    virtual bool OnLeftDblClick(const TModelPoint& /*p*/);
    virtual bool OnContextMenu(wxContextMenuEvent& event);
    /// @}

    /// Hit testing.
    /// Test whether a given point in PARENT's COORD. SYSTEM falls inside this
    /// layout object. Recursive hit testing may happen if this layout object
    /// is a container of other layout objects.
    /// @return the object that gets hit.
    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);
    virtual bool HitTestHor(TSeqPos x, const CObject *obj);

    /// Intersect testing.
    /// Test weather a given rectangular area intersects with this layout
    /// object. Recursive intersect testing is necessary if the layout
    /// object is a container of other containers.  All the intersected leaf
    /// objects will be added into objs. The rect is in parent's coord system.
    virtual bool Intersects(const TModelRect& rect, TConstObjects& objs) const;

    /// Check if need to show tooltip.
    /// If the tooltip retrieval is trivial, the derived glyph
    /// may decide to fill out the tooltip (info object) in this call.
    virtual bool NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const;

    /// Get the tooltip if available.
    /// The x and y are in 'world' coord system which is the coord. system for the
    /// uppermost layout object.
    virtual void GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& t_title) const;

    /// Get html active areas.
    /// This function is for CGI mode only.  By default, it does nothing.
    virtual void GetHTMLActiveAreas(TAreaVector* /*p_areas*/) const {};

    /// Update content and layout including the bounding box.
    /// @param layout_only if true only update the layout not the content.
    /// May need to update the layout only under certain situation, e.g. selection.
    /// The update is recursive if this is a container layout.
    /// By default, it updates the bounding box only.
    virtual void Update(bool /*layout_only*/);

    /// get the total range of this object.
    virtual TSeqRange GetRange(void) const;

    /// Interface for accepting an IGlyphVisitor.
    /// The visitor will be passed to its child glyphs for composite glyph.
    /// @return true if continue the traversal and false, otherwise.
    virtual bool Accept(IGlyphVisitor* visitor);

    /// Select or deselect this glyph.
    /// By default, it only set/unset m_Selected flag.
    /// Some concrete glyphs may need to override this method to change other
    /// glyph properties and layout when selected or deselected, e.g. show
    /// additional features.
    /// returns true if a layout update (ePUA_CallLayoutChanged) is required after all selections are processed
    virtual bool SetSelected(bool flag);

    /// Force to hide label.
    /// This method may not be applicable to all different types of glyphs.
    /// It is up to the concrete glyph to implement.  By default, it does
    /// nothing.  Putting this in the base class is just for the sake of
    /// inline layout policy to make sure not overlaping between glyph's label
    /// and other glyphs or lables.
    virtual void SetHideLabel(bool /*flag*/) {}

    /// Query if there is label and label is on the side.
    /// This method may not be applicable to all different types of glyphs.
    /// It is up to the concrete glyph to implement.  By default, it returns
    /// false.  Putting this in this base class is just for the sake of
    /// inline layout policy to determine if special actions need to take
    /// if there is sided label.
    virtual bool HasSideLabel() const { return false; };

    /// Query if this glyph is clickable.
    /// Mainly for chaging mouse cursor for clickable glyph. By default,
    /// it returns false.  If a glyph is clickable, but there is no
    /// need to change mouse cursor due to any other reason, such as
    /// there is already other visual hint, then don't need to override
    /// this function.
    virtual bool IsClickable() const { return false; };

    /// compare this object to another based on glyph sequence position.
    virtual bool LessBySeqPos(const CSeqGlyph& obj) const;

    /// compare this object to another, based on glyph geometry position.
    /// Notice that glyph geometry position may be the same as sequence
    /// position if there is not additional visual features showing up
    /// at the start and the end of the glyph.  E.g. the possible visual
    /// featue can be the side label.
    bool LessByPos(const CSeqGlyph& obj) const;

    /// compare this object to another, based on sequence size.
    bool LessBySeqSize(const CSeqGlyph& obj) const;

    /// compare this object to another, based on geometry size.
    /// Notice that sequence size and glyph geometry size may be the same
    /// for some cases.
    bool LessBySize(const CSeqGlyph& obj) const;

    /// render the layout.
    void Draw() const;

    /// get the bounding box.
    TModelRect GetModelRect() const;

    /// Hit test for points in PARENT COORD.
    bool IsIn(const TModelPoint& p) const;
    bool IsIn(TModelUnit x, TModelUnit y) const;
    bool IsInHor(TModelUnit x) const;

    /// @name Accessors
    /// @{
    bool IsSelected() const;
    bool IsHighlighted() const;
    CRgbaColor* GetHighlightsColor() const;
    virtual TModelUnit GetHeight() const;
    virtual TModelUnit GetWidth() const;
    virtual TModelUnit GetLeft() const;
    virtual TModelUnit GetTop() const;
    virtual TModelUnit GetRight() const;
    virtual TModelUnit GetBottom() const;
    const TModelPoint& GetPos() const;
    /// Get top left position in world coordinate.
    void GetPosInWorld(TModelPoint& pos) const;
    const CSeqGlyph* GetParent(void) const;
    CSeqGlyph* GetParent(void);
    ENeighbours GetNeighbours() const;
    /// @}

    /// @name Mutators
    /// @{
    void SetHighlighted(bool flag);
    void SetHighlightsColor(const CRgbaColor &hl_color);
    void SetHighlightsColor(const string &hl_color_str);
    virtual void SetWidth(TModelUnit w);
    virtual void SetHeight(TModelUnit h);
    virtual void SetLeft(TModelUnit l);
    virtual void SetTop(TModelUnit b);
    virtual void SetPos(const TModelPoint& upper_left);
    void SetParent(CSeqGlyph* p);
    CSeqGlyph* SetParent(void);
    void SetNeighbours(ENeighbours neighbours);
    /// @}

    /// set customized renderer for this layout objects.
    void SetRender(IGlyphRenderer* renderer);

    /// Set the rendering context.
    void SetRenderingContext(CRenderingContext* context);

    /// Get the rendering context.
    const CRenderingContext* GetRenderingContext() const;

    static bool PointInRect(const TModelPoint& p, const TModelRect& rc);
    static bool PointInRect(TModelUnit x, TModelUnit y, const TModelRect& rc);

    /// tearline factor -- used to limit a list of glyphs
    void SetTearline(size_t Tearline);
    size_t GetTearline() const;

    /// tearline text -- if set, is appended to the glyph name (does not naffect processing otherwise)
    void SetTearlineText(const string& sTearlineText) { m_sTearlineText = sTearlineText; }
    string GetTearlineText() const { return m_sTearlineText; }

    void SetRowNum(size_t cur_row) { m_RowNumber = cur_row; }
    size_t GetRowNum() const { return m_RowNumber; }

    void SetVisible(size_t v) { m_Visible = v; }
    size_t GetVisible() const { return m_Visible; }

    int GetLevel() const { return m_Level; }
    void SetLevel(int Level) { m_Level = Level; }

    /// persistent name of the glyph
    /// should not depend on the instance of the glyph, but should uniquely reflect its contents so two glyphs
    /// having the same contents will have the same name
    /// currently used to track isDrawn() status for expandable glyphs
    /// default implementation returns an empty string
    /// implementations exist for CFeatGlyph (name of the contained feature) and
    /// CLayoutGroup (concatenation of all sorted children names)
    virtual string GetPName() const { return ""; }

    float GetFadeFactor() const;

protected:
    /// The default renderer for this layout object.
    virtual void x_Draw() const = 0;

    /// Update the bounding box assuming children's sizes are fixed if any.
    virtual void x_UpdateBoundingBox() = 0;

    virtual void x_DrawHighlight() const;

    void x_DrawInnerLabelColumns(TModelUnit base, const string &label, const CRgbaColor &color, const CGlTextureFont& font, bool side_label_visible) const;
    void x_DrawInnerLabels(TModelUnit base, const string &label, const CRgbaColor &color, const CGlTextureFont& font, bool side_label_visible, bool inside_only, vector<TModelRange> *labels_ranges = nullptr, TSeqRange* interval = nullptr, bool XOR_mode = false) const;
    void x_DrawTruncatedLabel(const string &label, const CGlTextureFont& font, TModelUnit width, TModelUnit x, TModelUnit y, vector<TModelRange> *labels_ranges = nullptr) const;

    /// update the layout.
    virtual void x_OnLayoutChanged();

    /// Transform the coordiante from parent coord. to local coord.
    void x_Parent2Local(TModelPoint& pnt) const;

    /// Transform the coordiante from parent coord. to local coord.
    void x_Parent2Local(TModelUnit& /*x*/, TModelUnit& y) const;

    /// Transform the coordiante from parent coord. to local coord.
    void x_Parent2Local(TModelRect& rect) const;

    /// Transform the coordiante from world coord. to local coord.
    void x_World2Local(TModelPoint& p) const;
    void x_World2Local(TModelUnit& x, TModelUnit& y) const;

    /// Transform the coordiantes from local coord. to world coord.
    void x_Local2World(TModelPoint& p) const;
    void x_Local2World(TModelUnit& x, TModelUnit& y) const;

    /// initialize the basic information for a given active area.
    void x_InitHTMLActiveArea(CHTMLActiveArea& area) const;

    /// is this glyph visible in vertical direction?
    bool x_Visible_V() const;

protected:
    CSeqGlyph*              m_Parent;           ///< parent/child relationships for this feature
    CRenderingContext*      m_Context;          ///< the rendering context
    unique_ptr<CRgbaColor>  m_HighlightsColor;   ///< Highlight color
    ENeighbours             m_Neighbours;       ///< Indicates whether the glyph has neighbours
    // for debugging, to easier track  glyph names
    // mutable  string m_sPName;

private:
    TModelUnit      m_Height;   ///< object height
    TModelUnit      m_Width;    ///< object width
    TModelPoint     m_Pos;      ///< upper left corner position in parent coord
    CIRef<IGlyphRenderer> m_Renderer;   ///< user-defined renderer
    bool            m_Selected;
    bool            m_Highlighted;

    // numerical  limit for glyphs
    size_t          m_Tearline;

    // Current row number of glyph within its CLayoutGroup. This lets us trim a group
    // to a size (m_TearLine)
    size_t          m_RowNumber;

    // Set false to hide a glyph (used when m_RowNumber exceeds m_TearLine)
    bool            m_Visible;

    /// shows how deep is the glyph in an hierarchy (0 being closer to root, -1 undefined)
    int m_Level;

    // for debugging
    // bool m_isDead;
    
    // text that is used if the list of glyphs was truncated
    string m_sTearlineText;
};

///////////////////////////////////////////////////////////////////////////////
/// Visitor interface for applying any potential actions or algorithms.
///
class IGlyphVisitor
{
public:
    virtual ~IGlyphVisitor() {};

    /// @return true if continue to next visiting.
    virtual bool Visit(CSeqGlyph* glyph) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IGlyphDialogHost
/// An interface used for handling issues related to any dialog pops up that
/// requires some special cares, such as disallow tooltip pop up from the
/// underlying objects.
class IGlyphDialogHost
{
public:
    virtual ~IGlyphDialogHost() {};

    /// Prepare for showing a dialog.
    virtual void PreDialogShow() = 0;

    /// Post-processing after showing a dialog.
    virtual void PostDialogShow() = 0;
};


inline
bool operator< (const CSeqGlyph& o1, const CSeqGlyph& o2)
{
    return o1.LessByPos(o2);
}


/// basic sort by sequence position function.
struct SGlyphBySeqPos
{
    static bool s_CompareCRefs(const CRef<CSeqGlyph>& ref_obj1,
                               const CRef<CSeqGlyph>& ref_obj2)
    {
        return (ref_obj1->IsHighlighted() == ref_obj2->IsHighlighted()  &&
            ref_obj1->LessBySeqPos(*ref_obj2))  ||  ref_obj1->IsHighlighted();
    }

    bool operator() (const CRef<CSeqGlyph>& ref_obj1,
                     const CRef<CSeqGlyph>& ref_obj2) const
    {
        return s_CompareCRefs(ref_obj1, ref_obj2);
    }

    bool operator() (const CSeqGlyph& obj1,
                     const CSeqGlyph& obj2) const
    {
        return (obj1.IsHighlighted() == obj2.IsHighlighted()  &&
            obj1.LessBySeqPos(obj2))  ||  obj1.IsHighlighted();
    }

};

/// basic sort by geometry position function.
struct SGlyphByPos
{
    bool operator() (const CRef<CSeqGlyph>& ref_obj1,
                     const CRef<CSeqGlyph>& ref_obj2) const
    {
        return (ref_obj1->IsHighlighted() == ref_obj2->IsHighlighted()  &&
            ref_obj1->LessByPos(*ref_obj2))  ||  ref_obj1->IsHighlighted();
    }

    bool operator() (const CSeqGlyph& obj1,
                     const CSeqGlyph& obj2) const
    {
        return (obj1.IsHighlighted() == obj2.IsHighlighted()  &&
            obj1.LessByPos(obj2))  ||  obj1.IsHighlighted();
    }

};


/// basic sort by sequence size function
/// Larger size objects will be put in front.
struct SGlyphBySeqSize
{
    static bool s_CompareCRefs(const CRef<CSeqGlyph>& ref_obj1,
                               const CRef<CSeqGlyph>& ref_obj2)
    {
        return (ref_obj1->IsHighlighted() == ref_obj2->IsHighlighted()  &&
            ref_obj2->LessBySeqSize(*ref_obj1))  ||  ref_obj1->IsHighlighted();
    }

    bool operator() (const CRef<CSeqGlyph>& ref_obj1,
                     const CRef<CSeqGlyph>& ref_obj2) const
    {
        return s_CompareCRefs(ref_obj1, ref_obj2);
    }

    bool operator() (const CSeqGlyph& obj1,
                     const CSeqGlyph& obj2) const
    {
        return (obj1.IsHighlighted() == obj2.IsHighlighted()  &&
            obj2.LessBySeqSize(obj1))  ||  obj1.IsHighlighted();
    }

};


/// basic sort by geometry size function
struct SGlyphBySize
{
    bool operator() (const CRef<CSeqGlyph>& ref_obj1,
                     const CRef<CSeqGlyph>& ref_obj2) const
    {
        return (ref_obj1->IsHighlighted() == ref_obj2->IsHighlighted()  &&
            ref_obj2->LessBySize(*ref_obj1))  ||  ref_obj1->IsHighlighted();
    }

    bool operator() (const CSeqGlyph& obj1,
                     const CSeqGlyph& obj2) const
    {
        return (obj1.IsHighlighted() == obj2.IsHighlighted()  &&
            obj2.LessBySize(obj1))  ||  obj1.IsHighlighted();
    }
};


///////////////////////////////////////////////////////////////////////////////
/// CSeqGlyph inline methods
///
inline /*virtual*/
TSeqRange CSeqGlyph::GetRange(void) const
{
    return TSeqRange((TSeqPos)m_Pos.X(), (TSeqPos)(m_Pos.X() + m_Width));
}

inline /*virtual*/
bool CSeqGlyph::Accept(IGlyphVisitor* visitor)
{
    return visitor->Visit(this);
}

inline /*virtual*/
bool CSeqGlyph::SetSelected(bool flag)
{
//    LOG_POST(Trace << "==== CSeqGlyph::SetSelected() " << this << " to " << flag);
//    LOG_POST(Trace << "Call stack for " << this << ": " << CStackTrace());
    m_Selected = flag;
    return false;
}

inline /*virtual*/
bool CSeqGlyph::LessBySeqPos(const CSeqGlyph& obj) const
{
    TSeqRange r0 = GetRange();
    TSeqRange r1 = obj.GetRange();
    return r0.GetFrom() < r1.GetFrom()  ||
        (r0.GetFrom() == r1.GetFrom()  &&  r0.GetTo() < r1.GetTo());
}

inline
bool CSeqGlyph::LessByPos(const CSeqGlyph& obj) const
{
    return GetLeft() < obj.GetLeft()  ||
        (GetLeft() == obj.GetLeft()  &&  GetRight() < obj.GetRight());
}

inline
bool CSeqGlyph::LessBySeqSize(const CSeqGlyph& obj) const
{
    return (GetRange().GetLength() < obj.GetRange().GetLength());
}

inline
bool CSeqGlyph::LessBySize(const CSeqGlyph& obj) const
{
    return m_Width < obj.GetWidth();
}

inline
TModelRect CSeqGlyph::GetModelRect() const
{
    return TModelRect(m_Pos.X(), m_Pos.Y() + m_Height,
        m_Pos.X() + m_Width, m_Pos.Y());
}

inline
bool CSeqGlyph::IsIn(const TModelPoint& p) const
{ return IsIn(p.X(), p.Y()); }

inline
bool CSeqGlyph::IsSelected() const
{ return m_Selected; }

inline
bool CSeqGlyph::IsHighlighted() const
{ return m_Highlighted; }

inline
CRgbaColor* CSeqGlyph::GetHighlightsColor() const
{
    return m_HighlightsColor.get();
}

inline
TModelUnit CSeqGlyph::GetHeight() const
{ return m_Height;  }

inline
TModelUnit CSeqGlyph::GetWidth() const
{ return m_Width;   }

inline
TModelUnit CSeqGlyph::GetLeft() const
{ return m_Pos.X(); }

inline
TModelUnit CSeqGlyph::GetTop() const
{ return m_Pos.Y(); }

inline
TModelUnit CSeqGlyph::GetRight() const
{ return m_Pos.X() + m_Width; }

inline
TModelUnit CSeqGlyph::GetBottom() const
{ return m_Pos.Y() + m_Height; }

inline
const TModelPoint& CSeqGlyph::GetPos() const
{ return m_Pos; }

inline
void CSeqGlyph::GetPosInWorld(TModelPoint& pos) const
{
    pos.Init(m_Pos.X(), 0.0);
    x_Local2World(pos);
}

inline
const CSeqGlyph* CSeqGlyph::GetParent(void) const
{ return m_Parent; }

inline
CSeqGlyph* CSeqGlyph::GetParent(void)
{ return m_Parent; }

inline
CSeqGlyph::ENeighbours CSeqGlyph::GetNeighbours() const
{ return m_Neighbours; }

inline
void CSeqGlyph::SetHighlighted(bool flag)
{
    m_Highlighted = flag;
}

inline 
void CSeqGlyph::SetHighlightsColor(const CRgbaColor &hl_color)
{
    m_HighlightsColor.reset(new CRgbaColor(hl_color));
}

inline
void CSeqGlyph::SetWidth(TModelUnit w)
{ m_Width = w;      }

inline
void CSeqGlyph::SetHeight(TModelUnit h)
{ m_Height = h;     }

inline
void CSeqGlyph::SetLeft(TModelUnit l)
{ m_Pos.m_X = l;    }

inline
void CSeqGlyph::SetTop(TModelUnit t)
{ m_Pos.m_Y = t;    }

inline
void CSeqGlyph::SetPos(const TModelPoint& upper_left)
{ m_Pos = upper_left; }

inline
CSeqGlyph* CSeqGlyph::SetParent(void)
{ return m_Parent; }

inline
void CSeqGlyph::SetParent(CSeqGlyph* p)
{ m_Parent = p; }

inline
void CSeqGlyph::SetNeighbours(CSeqGlyph::ENeighbours neighbours)
{
    m_Neighbours = neighbours;
}

inline
void CSeqGlyph::SetRender(IGlyphRenderer* renderer)
{ m_Renderer.Reset(renderer); }

inline
void CSeqGlyph::SetRenderingContext(CRenderingContext* context)
{ m_Context = context;
//  cerr << "Context set: " << (void *) this << endl;
}

inline
const CRenderingContext* CSeqGlyph::GetRenderingContext() const
{ return m_Context; }

inline
bool CSeqGlyph::PointInRect(const TModelPoint& p, const TModelRect& rc)
{ return PointInRect(p.X(), p.Y(), rc); }

inline
bool CSeqGlyph::PointInRect(TModelUnit x, TModelUnit y, const TModelRect& rc)
{
    TModelUnit l = rc.Left();
    TModelUnit r = rc.Right();
    TModelUnit t = rc.Top();
    TModelUnit b = rc.Bottom();
    if (l > r) swap(l, r);
    if (t > b) swap(t, b);
    return l < x  &&  r > x  &&  t < y  &&  b > y;
}

inline
void CSeqGlyph::x_Parent2Local(TModelPoint& pnt) const
{ x_Parent2Local(pnt.m_X, pnt.m_Y); }

inline
void CSeqGlyph::x_Parent2Local(TModelUnit& /*x*/, TModelUnit& y) const
{ y -= m_Pos.Y(); }

inline
void CSeqGlyph::x_Parent2Local(TModelRect& rect) const
{ rect.Offset(0, -m_Pos.Y()); }

inline
void CSeqGlyph::x_World2Local(TModelPoint& p) const
{ x_World2Local(p.m_X, p.m_Y); }

inline
void CSeqGlyph::x_Local2World(TModelPoint& p) const
{ x_Local2World(p.m_X, p.m_Y); }

inline
void CSeqGlyph::SetTearline(size_t Tearline)
{
    m_Tearline = Tearline;
}

inline
size_t CSeqGlyph::GetTearline() const
{
    return m_Tearline;
}

inline 
float CSeqGlyph::GetFadeFactor() const
{
    float fade_factor = 1.0;
    if (GetTearline() > 0) {
        if (GetRowNum() > (GetTearline() - size_t(kFadeCount - 1.0f))) {
            fade_factor = 1.1f - (float(GetRowNum() - (GetTearline() - kFadeCount)) / kFadeCount);
        }
        fade_factor = std::max(0.0f, fade_factor);
    }
    return fade_factor;
}



END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___SEQ_GLYPH__HPP
