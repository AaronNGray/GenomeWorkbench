#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SELECTION_MANAGER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SELECTION_MANAGER__HPP

/*  $Id: selection_manager.hpp 39118 2017-08-01 20:05:22Z evgeniev $
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
 *
 */

#include <gui/gui.hpp>
#include <gui/opengl.h>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/objutils/object_index.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// class CSelectionVisitor is a concrete glyph visitor for selection
/// traversal and deselection traversal on a layout tree.  The selected
/// objects can be objects broadcasted from other views, signatures
/// from cgi, or user-selected objects in current view.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSelectionVisitor : public IGlyphVisitor
{
public:
    enum EActionMode {
        eAct_Select,
        eAct_Deselect,
        eAct_CollectSel
    };

    /// action that needs to be executed on selected glyphs
    /// after the visitor finishes visiting _all_ glyphs
    enum EPostUpdateAction {
        ePUA_None,
        /// call LayoutChanged for all glyphs that are CFeatGlyph
        ePUA_CallLayoutChanged
    };

    typedef EActionMode TActionMode;
    typedef list< CWeakRef<CSeqGlyph> > TSelectedGlyphs;

    CSelectionVisitor()
        : m_ActionMode(eAct_Select)
        , m_TraversedObjs(0)
        , m_TraversedSigs(0)
        , m_DeselectedObjs(0)
        , m_PostUpdateAction(ePUA_None)
    {}

    virtual ~CSelectionVisitor() {}

    /// @name IGlyphVisitor interface implementation
    /// @{
    virtual bool Visit(CSeqGlyph* glyph);
    /// @}

    void SetScope(objects::CScope* scope);
    objects::CScope* GetScope();
    void SetActionMode(TActionMode mode);
    void SetSelectedObjectSig(const string& obj_sig);

    void Clear();

    bool HasSelectedObjects() const;
    bool IsObjectSelected(const CObject* obj);

    const CSeqGlyph::TConstObjects&
        GetSelectedLayoutObjects(CSeqGlyph* top_glyph);

    void GetObjectSelection(TConstObjects& objs) const;

    void ResetObjectSelection(CSeqGlyph* glyph);

    void DeSelectObject(const CObject* obj);

    void SelectObject(const CObject* obj, bool verified);

    void SelectSelection(const CSeqGlyph::TConstObjects& glyphs);

    void UpdateSelection(CSeqGlyph* glyph);

    const TSelectedGlyphs& GetSelectedFeats() const;

    const TSelectedGlyphs& GetSelectedCDSFeats() const;

    void ClearSelectedFeats();

private:
    bool x_DoSelection(CSeqGlyph* glyph);
    bool x_NeedUpdate() const;

private:
    typedef set< CConstRef<CObject> > TSelectedObjects;
    // Object Selection
    CObjectIndex        m_SelectedIndexes;
    TSelectedObjects    m_SelectedObjects;

    // struct for storing unverified objects with type seq-id
    CObjectIndex        m_UnknownSelectedIndexes;

    list<string>        m_SelectedSignatures;
    CSeqGlyph::TConstObjects    m_SelGlyphs;

    TSelectedGlyphs     m_SelFeatures;
    TSelectedGlyphs     m_SelCDSFeatures;

    TModelRect m_SelRect;

    CRef<objects::CScope>   m_Scope;
    TActionMode             m_ActionMode;

    size_t                  m_TraversedObjs;
    size_t                  m_TraversedSigs;
    size_t                  m_DeselectedObjs;

    EPostUpdateAction m_PostUpdateAction;
};

///////////////////////////////////////////////////////////////////////////////
/// CSelectionVisitor inline methods
///

inline
void CSelectionVisitor::SetActionMode(TActionMode mode)
{ m_ActionMode = mode; }

inline
bool CSelectionVisitor::HasSelectedObjects() const
{ return !m_SelectedObjects.empty(); }

inline
const CSelectionVisitor::TSelectedGlyphs&
CSelectionVisitor::GetSelectedFeats() const
{ return m_SelFeatures; }

inline
const CSelectionVisitor::TSelectedGlyphs&
CSelectionVisitor::GetSelectedCDSFeats() const
{ return m_SelCDSFeatures; }


inline
void CSelectionVisitor::ClearSelectedFeats()
{
    m_SelFeatures.clear();
    m_SelCDSFeatures.clear();
    m_PostUpdateAction = ePUA_None;
}

inline
bool CSelectionVisitor::x_NeedUpdate() const
{
    return !m_SelectedIndexes.Empty()  ||
        !m_SelectedObjects.empty()  ||
        !m_SelectedSignatures.empty()  ||
        !m_UnknownSelectedIndexes.Empty()  ||
        m_DeselectedObjs > 0;
}

END_NCBI_SCOPE


#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SELECTION_MANAGER__HPP */
