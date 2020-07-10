/*  $Id: layout_group.cpp 44431 2019-12-17 22:16:34Z evgeniev $
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

#include <ncbi_pch.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>
#include <gui/opengl/irender.hpp>
#include <corelib/ncbi_limits.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kPadding = 2;

// #define DEBUG_INFO_ON_SCREEN 1


CLayoutGroup::CLayoutGroup(ELinkedFeatDisplay LinkedFeat) :
    m_HideMaster(false)
{
    m_LinkedFeat = LinkedFeat;
}

int CLayoutGroup::GetIndex(const CSeqGlyph* obj) const
{
    int i = 0;
    CConstRef<CSeqGlyph> child;
    TObjectList::const_iterator iter = m_Children.begin();
    while (iter != m_Children.end()) {
        if (obj == iter->GetPointer()) {
            break;
        }
        ++iter;
        ++i;
    }
    if (iter == m_Children.end()) {
        i = -1;
    }
    return i;
}


/// Get the layout object at index 'idx'.
CConstRef<CSeqGlyph> CLayoutGroup::GetChild(int idx) const
{
    int i = 0;
    CConstRef<CSeqGlyph> child;
    TObjectList::const_iterator iter = m_Children.begin();
    while (iter != m_Children.end()) {
        if (i == idx) {
            // got what we want
            child.Reset(iter->GetPointer());
            break;
        }
        ++iter;
        ++i;
    }
    return child;
}


CRef<CSeqGlyph> CLayoutGroup::GetChild(int idx)
{
    int i = 0;
    CRef<CSeqGlyph> child;
    TObjectList::iterator iter = m_Children.begin();
    while (iter != m_Children.end()) {
        if (i == idx) {
            // got what we want
            child.Reset(iter->GetPointer());
            break;
        }
        ++iter;
        ++i;
    }
    return child;
}



/// Remove a layout object.
bool CLayoutGroup::Remove(CSeqGlyph* obj)
{
    NON_CONST_ITERATE (TObjectList, iter, m_Children) {
        if (obj == iter->GetPointer()) {
            m_Children.erase(iter);
            return true;
        }
    }
    return false;
}


bool CLayoutGroup::Replace(CSeqGlyph* obj, int idx)
{
    int i = 0;
    TObjectList::iterator iter = m_Children.begin();
    while (iter != m_Children.end()) {
        if (i == idx) {
            // got what we want
            (*iter).Reset(obj);
            obj->SetParent(this);
            if (m_Context) obj->SetRenderingContext(m_Context);
            return true;
        }
        ++iter;
        ++i;
    }
    return false;
}


/// Insert a layout object before the given index.
void CLayoutGroup::Insert(int at, CSeqGlyph* obj)
{
    int i = 0;
    CConstRef<CSeqGlyph> child;
    TObjectList::iterator iter = m_Children.begin();
    while (iter != m_Children.end()  &&  i != at) {
        ++iter;
        ++i;
    }

    m_Children.insert(iter, CRef<CSeqGlyph>(obj));
    obj->SetParent(this);
    if (m_Context) obj->SetRenderingContext(m_Context);
}

/// Insert a layout object before the given layout object.
void CLayoutGroup::Insert(const CSeqGlyph* at_obj, CSeqGlyph* obj)
{
    TObjectList::iterator iter = m_Children.begin();
    while (iter != m_Children.end()  &&  at_obj != iter->GetPointer()) {
        ++iter;
    }

    m_Children.insert(iter, CRef<CSeqGlyph>(obj));
    obj->SetParent(this);
    if (m_Context) obj->SetRenderingContext(m_Context);
}

/// Insert a layout object after the given layout object.
void CLayoutGroup::InsertAft(const CSeqGlyph* at_obj, CSeqGlyph* obj)
{
    obj->SetParent(this);
    if (m_Context) obj->SetRenderingContext(m_Context);

    NON_CONST_ITERATE (TObjectList, iter, m_Children) {
        if (at_obj == iter->GetPointer()) {
            m_Children.insert(++iter, CRef<CSeqGlyph>(obj));
            return;
        }
    }
    PushBack(obj);
}

void CLayoutGroup::Update(bool layout_only)
{
    NON_CONST_ITERATE (TObjectList, iter, m_Children) {
        (*iter)->Update(layout_only);
    }
    x_UpdateBoundingBox();
}


CRef<CSeqGlyph> CLayoutGroup::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> glyph;
    if (IsIn(p)) {
        TModelPoint pp(p);
        x_Parent2Local(pp);
        NON_CONST_ITERATE (TObjectList, iter, m_Children) {
            glyph = (*iter)->HitTest(pp);
            if (glyph) {
                break;
            }
        }
    }
    return glyph;
}

bool CLayoutGroup::HitTestHor(TSeqPos x, const CObject *obj)
{
    if (IsInHor(x)) {
        NON_CONST_ITERATE (TObjectList, iter, m_Children) {
            if ((*iter)->HitTestHor(x, obj))
                return true;
        }
    }
    return false;
}

bool CLayoutGroup::Intersects(const TModelRect& rect,
                              TConstObjects& objs) const
{
    if (rect.Intersects(GetModelRect())) {
        TModelRect rc(rect);
        x_Parent2Local(rc);
        ITERATE (TObjectList, iter, m_Children) {
            (*iter)->Intersects(rc, objs);
        }
        return true;
    }
    return false;
}


void CLayoutGroup::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    if(x_isDrawn()) {
        try {
            ITERATE (TObjectList, iter, m_Children) {
                if (m_HideMaster && IsMaster(*iter))
                    continue;
                (*iter)->GetHTMLActiveAreas(p_areas);
            }
        } catch (CException& e) {
            LOG_POST(Error
                << "CLayoutGroup::GetHTMLActiveAreas() failure on getting HTML active areas, error: "
                << e.GetMsg());
        }
    }
}


bool CLayoutGroup::Accept(IGlyphVisitor* visitor)
{
    bool cont = visitor->Visit(this);
    TObjectList::iterator iter = m_Children.begin();
    while (cont  &&  iter != m_Children.end()) {
        cont = (*iter)->Accept(visitor);
        ++iter;
    }
    return cont;
}

bool CLayoutGroup::x_HasExpandedChildren() const
{
    const CLayoutGroup::TObjectList& AllChildren(GetChildren());
    ITERATE(CLayoutGroup::TObjectList, iChildren, AllChildren)
    {
        if ((*iChildren)->GetLevel() == 0) {
            //            LOG_POST(Trace << ">>>> child at level 0, true");
            return true;
        }
        const CFeatGlyph* pFeatGlyph = dynamic_cast<const CFeatGlyph*>(iChildren->GetPointerOrNull());
        if (pFeatGlyph && pFeatGlyph->isDrawn()) {
            //            LOG_POST(Trace << ">>>> feature child is drawn, true");
            return true;
        }
        const CLayoutGroup* pGroup = dynamic_cast<const CLayoutGroup*>(iChildren->GetPointerOrNull());
        if (pGroup && pGroup->x_isDrawn()) {
            //            LOG_POST(Trace << ">>>> group child is drawn, true");
            return true;
        }
    }
    //    LOG_POST(Trace << ">>>> no children drawn, false");
    return false;
}

bool CLayoutGroup::x_isDrawn() const
{
//    LOG_POST(Trace << "<<<< CLayoutGroup::x_isDrawn() " << this << " \"" << " <" << GetLevel() <<  "> \"" << GetPName() << "\"");
    // layout groups containing a glyph with level 0 (topmost) are unconditionally shown
    if(m_LinkedFeat == ELinkedFeatDisplay::eLFD_Expandable) 
        return x_HasExpandedChildren();

    return true;
}




void CLayoutGroup::x_Draw() const
{
    if ((GetLeft() >= 0) && m_Context->IntersectVisible(GetRange()).Empty())
        return;

    if (x_Visible_V() && x_isDrawn()) {
        IRender& gl = GetGl();

        gl.PushMatrix();
        gl.Translated(0.0, GetTop(), 0.0f);
        x_DrawBoundary();
        ITERATE (TObjectList, iter, m_Children) {
            if (m_HideMaster && IsMaster(*iter))
                continue;
            (*iter)->Draw();
        }
        gl.PopMatrix();
    }
        // debug: print the level and drawn status of the group
#ifdef DEBUG_INFO_ON_SCREEN
        CGlTextureFont font;
        font.SetFontFace(CGlTextureFont::FaceFromString("Helvetica"));
        font.SetFontSize(10);
        string sDebugInfo("CLayoutGroup<" + NStr::NumericToString(GetLevel()) + ">/" + NStr::IntToString(x_isDrawn()) + "/[" +  NStr::IntToString(GetChildrenNum()) + "]");
        m_Context->TextOut(&font, sDebugInfo.c_str(), GetLeft() - 30 > 0 ? GetLeft() - 30 : 10, GetTop(), true, true);
#endif

}

void CLayoutGroup::x_UpdateBoundingBox()
{
    SetHeight(0.0);
    SetWidth(0.0);

    if(x_isDrawn()) {
        ILayoutPolicy::SBoundingBox bound;
        // just in case any children became "selected" since the last update and therefore became visible, force update their
        // bounding boxes as well
        if (m_LinkedFeat == ELinkedFeatDisplay::eLFD_Expandable && x_HasExpandedChildren()) {
            NON_CONST_ITERATE(TObjectList, iChildren, m_Children) {
                if (m_HideMaster && IsMaster(*iChildren))
                    continue;
                if (iChildren->GetPointerOrNull() != this) {
                    (*iChildren)->Update(true);
                }
            }
        } 
        if (m_Policy) {
            size_t size = m_Children.size();
            m_Policy->BuildLayout(*this, bound);

            if (size != m_Children.size()) {
                // HACK! HACK! HACK!
                // The object hierachy is changed.  We need to do something
                // create a group to hold all the sub-groups
                TObjectList sub_groups = GetChildren();
                Clear();
                CLayoutGroup* group  = new CLayoutGroup(GetLinkedFeat());
                if(GetLevel() != -1) {
                    group->SetLevel(GetLevel()+1);
                }
                PushBack(group);
                group->Set(sub_groups);
                CSimpleLayout* simple = new CSimpleLayout;
                group->SetLayoutPolicy(simple);
                simple->SetVertSpace(0);
                simple->SetTopMargin(0);
                simple->BuildLayout(*group, bound);
                group->SetLeft(bound.m_X);
                group->SetHeight(bound.m_Height);
                group->SetWidth(bound.m_Width);
                group->SetTop(0.0);
            }

            SetHeight(bound.m_Height);
            SetWidth(bound.m_Width);
            SetLeft(bound.m_X);
        }

        int padding_y = kPadding;
        if (m_Config) {
            padding_y = m_Config->GetPadding();
        }
        TModelUnit padding_x = m_Context->ScreenToSeq(padding_y);;
        SetLeft(GetLeft() - padding_x);
        SetWidth(GetWidth() + padding_x * 2);
        SetHeight(GetHeight() + padding_y);
    }
}

bool CLayoutGroup::AllChildrenSameType() const
{
    // If 0 or 1 element, same type by definition
    if (m_Children.size() <= 1)
        return true;

    // we are currently only concerned about features so
    // check if all children are CFeatGlyph and if their feature subtype
    // is the same
    bool first_type = true;
    CSeqFeatData::ESubtype t;

    ITERATE (TObjectList, iter, m_Children) {
        const CFeatGlyph* glyph = dynamic_cast<const CFeatGlyph*>(iter->GetPointerOrNull());
        if (glyph == NULL)
            return false;
        if (glyph->GetFeature().IsSetData()) {
            if (!first_type) {
                if (t != glyph->GetFeature().GetData().GetSubtype())
                    return false;
            }
            else {
                t = glyph->GetFeature().GetData().GetSubtype();
                first_type = false;
            }
        }
        else return false;
    }

    return true;
}

void CLayoutGroup::x_DrawBoundary() const
{
    IRender& gl = GetGl();

    if (m_Config) {
        TModelRect rect(GetLeft(), 0.0, GetRight(), GetHeight());
        if (m_Config->ShowBackground()) {
            gl.ColorC(m_Config->GetBgColor());
            m_Context->DrawQuad(rect);
        }
        if (m_Config->ShowBoundary()) {
            gl.ColorC(m_Config->GetFgColor());
            gl.LineWidth(m_Config->GetLineWidth());
            m_Context->DrawRect(rect);
            gl.LineWidth(1.0f);
        }
        if (IsSelected()) {
            m_Context->DrawSelection(rect);
        }
    }
}


// pGlyph is the glyph we are changing
// pGlyphFrom is the child from which we came
void CLayoutGroup::PropagateRelatedSelection(bool isSelected, CSeqGlyph* pGlyph, CSeqGlyph* pGlyphFrom)
{
//    LOG_POST(Trace << "<<<< CLayoutGroup::PropagateRelatedSelection(" << isSelected << ") " << pGlyph << ", from:" << pGlyphFrom);
//    LOG_POST(Trace << "CLayoutGroup::PropagateRelatedSelection(): " << pGlyph->GetPName());
    if(pGlyph) {
        CFeatGlyph* pFeatGlyph(dynamic_cast<CFeatGlyph*>(pGlyph));

        if(pFeatGlyph) {
            pFeatGlyph->SetRelatedGlyphSelected(isSelected);
        }
        CLayoutGroup* pLayoutGroup(dynamic_cast<CLayoutGroup*>(pGlyph));
        int GroupLevel(-1);

        bool isChildLevel0Reached(false);

        if(pLayoutGroup) {
            GroupLevel = pLayoutGroup->GetLevel();
            if(GroupLevel < 1) {
                return;
            }
            CLayoutGroup::TObjectList& AllChildren(pLayoutGroup->GetChildren());
            NON_CONST_ITERATE(CLayoutGroup::TObjectList, iChildren, AllChildren) {
                if((*iChildren)->GetLevel() == 0) {
                    isChildLevel0Reached = true;
                }
                if(iChildren->GetPointerOrNull() != pGlyph && iChildren->GetPointerOrNull() != pGlyphFrom) {
                    PropagateRelatedSelection(isSelected, iChildren->GetPointerOrNull(), pGlyph);
                }
            }
        }
        // if any of the members of the group is level 0, don't go up
        if(isChildLevel0Reached) {
            return;
        }
        if(pGlyph->GetParent() != pGlyphFrom) {
            PropagateRelatedSelection(isSelected, pGlyph->GetParent(), pGlyph);
        }
    }
//    LOG_POST(Trace << ">>>> CLayoutGroup::PropagateRelatedSelection(" << isSelected << ") " << pGlyph << ", from:" << pGlyphFrom);
}




END_NCBI_SCOPE

