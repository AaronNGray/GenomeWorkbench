#ifndef GUI_WIDGETS_EDIT___EDIT_OBJECT_SEQ_FEAT__HPP
#define GUI_WIDGETS_EDIT___EDIT_OBJECT_SEQ_FEAT__HPP

/*  $Id: edit_object_seq_feat.hpp 41740 2018-09-18 20:04:07Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/scope.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <wx/treebook.h>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CEditObjectSeq_feat : public CObject, public IEditObject
{
public:
    CEditObjectSeq_feat(const CObject& object,
                        const objects::CSeq_entry_Handle& seh,
                        objects::CScope& scope, bool createMode)
        : m_Object(&object), m_Scope(&scope), m_SEH(seh), m_CreateMode(createMode), m_Window() {}

    virtual wxWindow* CreateWindow(wxWindow* parent);
    virtual IEditCommand* GetEditCommand();
    virtual bool CanClose();
    CRef<CSerialObject> GetEditObject() {return m_EditedFeature;}
    wxWindow* CreateWindowImpl(wxWindow* parent, objects::CSeq_feat& edited_seq_feat);
    void SetWindowToNull() {m_Window = NULL;}
    wxWindow* GetSubPage() {if (m_Window) return m_Window->GetCurrentPage(); return NULL;}
    void OnPageChange(wxBookCtrlEvent& event);

private:

    enum {
        ID_NOTEBOOK = 10015
    };

    void x_AddCodingRegionProteinNamePanel(objects::CProt_ref& prot);
    void x_AddCodingRegionProteinPanels(objects::CProt_ref& prot, objects::CSeq_feat* edited_protein);
    CIRef<IEditCommand> x_AdjustGene(const objects::CSeq_feat& old_gene,
                                     const objects::CSeq_loc& old_loc,  
                                     const objects::CSeq_loc& new_loc, 
                                     CRef<objects::CSeq_feat> updated_gene);
    bool x_IsCircular(const objects::CSeq_loc& loc);
    void x_AdjustCircularGene( CRef<objects::CSeq_feat> adjusted_gene, const objects::CSeq_loc& new_loc);

    void x_AdjustLocation(objects::CSeq_feat& edited_feat);
    void x_SetGbQuals(objects::CSeq_feat& edited_feat);
    void x_SetCrossRefs(objects::CSeq_feat& edited_feat);
    void x_SetIdentifiers(objects::CSeq_feat& edited_feat);
    void x_SetInferences(objects::CSeq_feat& edited_feat);
    void x_SetExperiments(objects::CSeq_feat& edited_feat);
    void x_SetCitations(objects::CSeq_feat& edited_feat);
    void x_SetTranslExcept(objects::CSeq_feat& cds);
    CIRef<IEditCommand> x_UpdatemRNA(const objects::CSeq_feat& cds);
    CRef<CCmdComposite> x_HandleCdsProduct(const objects::CSeq_feat &old_cds, objects::CSeq_feat& cds);
    objects::CSeq_feat& x_GetSeqFeatCopy();
    void x_ConvertCommentToMiscFeat(objects::CSeq_feat& edited_seq_feat);

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;

    objects::CSeq_entry_Handle m_SEH;
    bool m_CreateMode;

    CRef<CSerialObject> m_EditedFeature;
    CRef<CSerialObject> m_EditedProtein;
    CIRef<IEditCommand> m_EditAction;

    wxTreebook* m_Window;
    objects::CSeq_feat_Handle m_fh;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___EDIT_OBJECT_SEQ_FEAT__HPP
