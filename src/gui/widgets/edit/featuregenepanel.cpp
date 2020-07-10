/*  $Id: featuregenepanel.cpp 42681 2019-04-02 15:42:12Z filippov $
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


#include <ncbi_pch.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>

////@begin includes
////@end includes
#include "generic_props_panel.hpp"
#include "featuregenepanel.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CFeatureGenePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFeatureGenePanel, wxPanel )


/*!
 * CFeatureGenePanel event table definition
 */

BEGIN_EVENT_TABLE( CFeatureGenePanel, wxPanel )

////@begin CFeatureGenePanel event table entries
    EVT_LISTBOX( ID_CHOICE16, CFeatureGenePanel::OnSelectGene )
    EVT_CHECKBOX( ID_CHECKBOX13, CFeatureGenePanel::OnClickSuppress )
    EVT_BUTTON( ID_BUTTON_EDIT_GENE, CFeatureGenePanel::OnEditGene )
////@end CFeatureGenePanel event table entries

END_EVENT_TABLE()


static CBioseq_Handle BioseqHandleFromLocation (CScope* m_Scope, const CSeq_loc& loc)

{
    CBioseq_Handle bsh;
    for ( CSeq_loc_CI citer (loc); citer; ++citer) {
        const CSeq_id& id = citer.GetSeq_id();
        CSeq_id_Handle sih = CSeq_id_Handle::GetHandle(id);
        bsh = m_Scope->GetBioseqHandle (sih, CScope::eGetBioseq_All);
        if (bsh) {
            return bsh;
        }
    }
    return bsh;
}


/*!
 * CFeatureGenePanel constructors
 */

CFeatureGenePanel::CFeatureGenePanel()
{
    Init();
}

CFeatureGenePanel::CFeatureGenePanel( wxWindow* parent, CSerialObject& object, CScope& scope, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object), m_Scope(&scope)
{
    Init();
    m_GeneRef = new CGene_ref();

    CSeq_feat * feat = dynamic_cast<CSeq_feat*>(m_Object);
    const CGene_ref* gref = feat->GetGeneXref();
    if (gref) {
        m_GeneRef->Assign (*gref);
    } else {
        // find gene currently associated with feature
    }
    
    Create(parent, id, pos, size, style);
}


/*!
 * CFeatureGenePanel creator
 */

bool CFeatureGenePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatureGenePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFeatureGenePanel creation
    return true;
}


/*!
 * CFeatureGenePanel destructor
 */

CFeatureGenePanel::~CFeatureGenePanel()
{
////@begin CFeatureGenePanel destruction
////@end CFeatureGenePanel destruction
}


/*!
 * Member initialisation
 */

void CFeatureGenePanel::Init()
{
////@begin CFeatureGenePanel member initialisation
    m_GeneChoice = NULL;
    m_MapType = NULL;
    m_Suppress = NULL;
    m_ButtonEditGene = NULL;
    m_modify_pseudo = false;
////@end CFeatureGenePanel member initialisation
}


/*!
 * Control creation for CFeatureGenePanel
 */

void CFeatureGenePanel::CreateControls()
{    
////@begin CFeatureGenePanel content construction
    CFeatureGenePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Gene:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_GeneChoiceStrings;
    m_GeneChoice = new wxListBox( itemPanel1, ID_CHOICE16, wxDefaultPosition, wxDefaultSize, m_GeneChoiceStrings, wxLB_SINGLE );
    itemBoxSizer3->Add(m_GeneChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_MapTypeStrings;
    m_MapTypeStrings.Add(_("&Overlap"));
    m_MapTypeStrings.Add(_("&Cross-reference"));
    m_MapType = new wxRadioBox( itemPanel1, ID_RADIOBOX4, _("Map by"), wxDefaultPosition, wxDefaultSize, m_MapTypeStrings, 1, wxRA_SPECIFY_ROWS );
    m_MapType->SetSelection(0);
    itemBoxSizer2->Add(m_MapType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Suppress = new wxCheckBox( itemPanel1, ID_CHECKBOX13, _("Suppress Gene"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Suppress->SetValue(false);
    itemBoxSizer2->Add(m_Suppress, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CFeatureGenePanel content construction

    m_GeneChoice->Append(wxEmptyString);
    m_GeneChoice->Append(_("New"));


    CSeq_feat * feat = dynamic_cast<CSeq_feat*>(m_Object);
    string match_label;
    const CGene_ref* gs = feat->GetGeneXref();
    if (gs) {        
        gs->GetLabel (&match_label);
        if (NStr::IsBlank (match_label)) {
            m_Suppress->SetValue(true);
            m_GeneChoice->Enable (false);
            m_MapType->Enable (false);
        } else {
            m_MapType->SetSelection(1);
        }
    } 

    if (NStr::IsBlank (match_label)) {
        m_OverlappingGene = sequence::GetOverlappingGene(feat->GetLocation(), *m_Scope);
        // pick out gene by overlap, in case user turns off suppression
        if ( m_OverlappingGene ) {
            m_OverlappingGene->GetData().GetGene().GetLabel(&match_label);
        }
    }

    int index = -1;
    m_GeneList.clear();
    m_GeneFeatList.clear();
    CBioseq_Handle bsh = BioseqHandleFromLocation(m_Scope, feat->GetLocation());
    if (bsh) 
    {
        for (CFeat_CI gene_it(bsh, SAnnotSelector(CSeqFeatData::e_Gene)); gene_it; ++gene_it) 
        {
            const CGene_ref& gr = gene_it->GetSeq_feat()->GetData().GetGene();
            string label;
            gr.GetLabel(&label);
            if (label.empty()) {
                continue;
            }
                       
            m_GeneList.push_back(ConstRef(&gr));
            m_GeneFeatList.push_back(gene_it->GetSeq_feat());
            m_GeneChoice->Append(ToWxString(label));
        }      
        if (m_OverlappingGene)
        {
            int n = 2;
            for (CFeat_CI gene_it(bsh, SAnnotSelector(CSeqFeatData::e_Gene)); gene_it; ++gene_it, ++n) 
            {
                const CSeq_feat& feat = gene_it->GetOriginalFeature();
                if (m_OverlappingGene->Equals(feat)) 
                {
                    index = n;
                    break;
                }
            }
        }
        if (index == -1 && !NStr::IsBlank(match_label))
        {
            int n = 2;
            for (CFeat_CI gene_it(bsh, SAnnotSelector(CSeqFeatData::e_Gene)); gene_it; ++gene_it, ++n) 
            {
                const CGene_ref& gr = gene_it->GetSeq_feat()->GetData().GetGene();
                string label;
                gr.GetLabel(&label);
                if (!label.empty() && NStr::Equal(match_label, label)) 
                {
                    index = n;
                    break;
                }
            }
        }
    }
  

    m_ButtonEditGene = new wxButton(itemPanel1, ID_BUTTON_EDIT_GENE, _("Edit Gene"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_ButtonEditGene, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5); 
    m_ButtonEditGene->Disable();

    if (index > -1) {
        m_GeneChoice->SetSelection(index);
        m_GeneChoice->EnsureVisible(index);
        m_ButtonEditGene->Enable();
    } else {
        // label not found
        if (NStr::IsBlank (match_label)) {
            m_GeneChoice->SetSelection (0);
            m_GeneChoice->EnsureVisible(0);
        } else {
            // create new
            m_GeneChoice->SetSelection(1);
            m_GeneChoice->EnsureVisible(1);
        }
    }

    m_GeneRefPanel = new CGenePanel( itemPanel1, *m_GeneRef, *m_Scope);
    itemBoxSizer2->Add(m_GeneRefPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    if (m_GeneChoice->GetSelection() == 1) {
        m_GeneRefPanel->Enable (true);
    } else {
        m_GeneRefPanel->Enable (false);
    }
}


static void ClearGeneXref (CSeq_feat * feat)
{
    if (!feat->IsSetXref()) {
        return;
    }

    CSeq_feat::TXref::iterator it = feat->SetXref().begin();
    while (it != feat->SetXref().end()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsGene ()) {
            it = feat->SetXref().erase(it);
        } else {
            ++it;
        }
    }
    if (feat->SetXref().size() == 0) {
        feat->ResetXref();
    }
}


bool CFeatureGenePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;
    m_GeneRefPanel->TransferDataFromWindow();

    CSeq_feat * feat = dynamic_cast<CSeq_feat*>(m_Object);
    if (m_Suppress->GetValue()) {
        CGene_ref& s = feat->SetGeneXref();
        s.Reset();
    } else if (m_MapType->GetSelection() == 1) {        
        int which_gene = m_GeneChoice->GetSelection();
        if (which_gene == 0) {
            // empty, same as suppress
            CGene_ref& s = feat->SetGeneXref();
            s.Reset();
        } else if (which_gene == 1) {
            // new gene            
            string label = "";
            m_GeneRef->GetLabel(&label);
            if (NStr::IsBlank (label)) {
                ClearGeneXref (feat);
            } else {
                // create new gene xref
                CGene_ref& s = feat->SetGeneXref();
                s.Assign(*m_GeneRef);
            }
        } else {
            // create gene xref to existing gene
            CGene_ref& s = feat->SetGeneXref();
            s.Assign(*m_GeneList[which_gene - 2]);            
            if (m_GeneFeatList[which_gene - 2]->IsSetId())
            {
                NON_CONST_ITERATE(CSeq_feat::TXref, xit, feat->SetXref()) 
                {
                    if ((*xit)->IsSetData() && (*xit)->GetData().IsGene() && s.Equals((*xit)->GetData().GetGene())) 
                    {
                        (*xit)->SetId().Assign(m_GeneFeatList[which_gene - 2]->GetId());
                    }
                }
            }                           
        }
    } else {
        // clear gene xref if there is one, build gene
        ClearGeneXref (feat);
    }

    return true;
}


CRef<CSeq_loc> CFeatureGenePanel::GetLocationForGene()
{
    CSeq_feat * feat = dynamic_cast<CSeq_feat*>(m_Object);
    if (!feat || !feat->IsSetLocation()) {
        return CRef<CSeq_loc>(NULL);
    }
    bool trans_splicing = false;
    if (feat->IsSetExcept_text() && NStr::Find(feat->GetExcept_text(), "trans-splicing") != string::npos) {
        trans_splicing = true;
    }

    CRef<CSeq_loc> gene_loc(NULL);

    
    CSeq_loc_CI loc_iter(feat->GetLocation());
    CSeq_loc_CI::TRange range = loc_iter.GetRange();
    TSeqPos from = range.GetFrom();
    TSeqPos to = range.GetTo();
    bool minus = (loc_iter.GetStrand() == eNa_strand_minus);
    CConstRef<CSeq_id> last_id(&(loc_iter.GetSeq_id()));
    CBioseq_Handle bsh = BioseqHandleFromLocation(m_Scope, loc_iter.GetEmbeddingSeq_loc());
    ++loc_iter;
    while (loc_iter) {
        bool need_new_interval = false;
        bool new_minus = (loc_iter.GetStrand() == eNa_strand_minus);
        if ((minus && !new_minus) || (!minus && new_minus)) {
            need_new_interval = true;
        }
        CBioseq_Handle new_bsh = BioseqHandleFromLocation(m_Scope, loc_iter.GetEmbeddingSeq_loc());
        if (bsh != new_bsh) {
            need_new_interval = true;
        }
        range = loc_iter.GetRange();
        TSeqPos new_from = range.GetFrom();;
        TSeqPos new_to = range.GetTo();

        if (!need_new_interval && trans_splicing) {
            // check for distance
            TSeqPos distance = 0;
            bool out_of_order = false;
            // check for out of order or distance
            if (minus) {
                if (from < new_from || to < new_to) {
                    out_of_order = true;
                } else if (from > new_to) {
                    distance = from - new_to;
                }
            } else {
                if (from > new_from || to > new_to) {
                    out_of_order = true;
                } else if (new_from > to) {
                    distance = new_from - to;
                }                    
            }
            if (out_of_order || distance > 10000) {
                need_new_interval = true;
            }
        }

        if (need_new_interval) {
            if (!gene_loc) {
                gene_loc.Reset(new CSeq_loc());
            }
            CRef<CSeq_id> new_id(new CSeq_id());
            new_id->Assign(*last_id);
            CRef<CSeq_loc> sub(new CSeq_loc(*new_id, from, to, minus ? eNa_strand_minus : eNa_strand_unknown));
            gene_loc->SetMix().Set().push_back(sub);
            from = new_from;
            to = new_to;
            minus = new_minus;
            last_id = &(loc_iter.GetSeq_id());
        } else {
            // extend interval
            if (minus) {
                from = new_from;
            } else {
                to = new_to;
            }
        }
        ++loc_iter;
    }
    if (gene_loc) {
        CRef<CSeq_id> new_id(new CSeq_id());
        new_id->Assign(*last_id);
        CRef<CSeq_loc> sub(new CSeq_loc(*new_id, from, to, minus ? eNa_strand_minus : eNa_strand_unknown));
        gene_loc->SetMix().Set().push_back(sub);
    } else {
        CRef<CSeq_id> new_id(new CSeq_id());
        new_id->Assign(*last_id);
        gene_loc.Reset(new CSeq_loc(*new_id, from, to, minus ? eNa_strand_minus : eNa_strand_unknown));
    }
        
    bool partial_start = feat->GetLocation().IsPartialStart(eExtreme_Positional);
    bool partial_stop = feat->GetLocation().IsPartialStop(eExtreme_Positional);
    gene_loc->SetPartialStart(partial_start, eExtreme_Positional);
    gene_loc->SetPartialStop(partial_stop, eExtreme_Positional);
    return gene_loc;
}

void CFeatureGenePanel::AddReciprocicalXref(CRef<CSeq_feat> gene_feat)
{
    if (!gene_feat || m_MapType->GetSelection() != 1)
        return;

    CSeq_feat * src = dynamic_cast<CSeq_feat*>(m_Object);
    if (!src ||!src->IsSetId() || !src->GetId().IsLocal())
        return;

    if (!CSeqFeatData::AllowXref(src->GetData().GetSubtype(), gene_feat->GetData().GetSubtype())) {
        // only create reciprocal xrefs if permitted
        return;
    }
    CBioseq_Handle bsh = BioseqHandleFromLocation(m_Scope, gene_feat->GetLocation());
    if (!bsh)
        return;
    const CTSE_Handle& tse = bsh.GetSeq_entry_Handle().GetTSE_Handle();
    // don't create xref if already have xref to feature of same type as src
    bool has_xref = false;
    if (gene_feat->IsSetXref()) {
        NON_CONST_ITERATE(CSeq_feat::TXref, xit, gene_feat->SetXref()) {
            if ((*xit)->IsSetId() && (*xit)->GetId().IsLocal()) {
                if ((*xit)->GetId().Equals(src->GetId())) {
                    // already have xref
                    has_xref = true;
                    break;
                } else {  
                    const CTSE_Handle::TFeatureId& feat_id = (*xit)->GetId().GetLocal();
                    CTSE_Handle::TSeq_feat_Handles far_feats = tse.GetFeaturesWithId(CSeqFeatData::e_not_set, feat_id);
                    ITERATE(CTSE_Handle::TSeq_feat_Handles, fit, far_feats) {
                        if (fit->GetData().GetSubtype() == src->GetData().GetSubtype()) {
                            (*xit)->SetId().Assign(src->GetId());
                            has_xref = true;
                            break;
                        }
                    }
                    if (has_xref) {
                        break;
                    }
                }
            }
        }
    }

    if (!has_xref) {  
        gene_feat->AddSeqFeatXref(src->GetId());       
    }
}

CRef<CSeq_feat> CFeatureGenePanel::GetNewGene()
{
    TransferDataFromWindow();
    CRef<CSeq_feat> gene_feat;
    if (!m_Suppress->GetValue() 
        //&& m_MapType->GetSelection() == 0
        && (m_GeneRef->IsSetLocus() || m_GeneRef->IsSetLocus_tag())
        && m_GeneChoice->GetSelection() == 1) 
    {        
        gene_feat.Reset(new CSeq_feat());
        gene_feat->SetData().SetGene().Assign (*m_GeneRef);
        // pick location to match
        CRef<CSeq_loc> gene_loc = GetLocationForGene();
        gene_feat->SetLocation(*gene_loc);
        bool partial_start = gene_feat->GetLocation().IsPartialStart(eExtreme_Positional);
        bool partial_stop = gene_feat->GetLocation().IsPartialStop(eExtreme_Positional);
        if (partial_start || partial_stop) 
        {
            gene_feat->SetPartial (true);
        }
        AddReciprocicalXref(gene_feat);
        if (m_modify_pseudo)
        {
            CGenericPropsPanel::ModifyPseudo(*gene_feat, m_pseudo_choice);
        }
    }
    return gene_feat;
}


CConstRef<CSeq_feat> CFeatureGenePanel::GetGeneToEdit()
{
    TransferDataFromWindow();
    if (!m_Suppress->GetValue() 
        //&& m_MapType->GetSelection() == 0 
        && m_GeneChoice->GetSelection() > 1) {
        int which_gene = m_GeneChoice->GetSelection() - 2;
       
        return m_GeneFeatList[which_gene];
    } else {
        CConstRef<CSeq_feat> gene_feat;
        gene_feat.Reset();
        return gene_feat;
    }        
}

CRef<CSeq_feat> CFeatureGenePanel::GetUpdatedGene()
{
    TransferDataFromWindow();
    if (!m_Suppress->GetValue() 
        //&& m_MapType->GetSelection() == 0 
        && m_GeneChoice->GetSelection() > 1) {
        int which_gene = m_GeneChoice->GetSelection() - 2;
        CRef<CSeq_feat> gene_feat(new CSeq_feat);
        gene_feat->Assign(*m_GeneFeatList[which_gene]);
        AddReciprocicalXref(gene_feat);
        if (m_modify_pseudo)
        {
            CGenericPropsPanel::ModifyPseudo(*gene_feat, m_pseudo_choice);
        }
        return gene_feat;
    } else {
        CRef<CSeq_feat> gene_feat;
        return gene_feat;
    }        
}

/*!
 * Should we show tooltips?
 */

bool CFeatureGenePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeatureGenePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatureGenePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatureGenePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeatureGenePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatureGenePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatureGenePanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX13
 */

void CFeatureGenePanel::OnClickSuppress( wxCommandEvent& event )
{
    if (m_Suppress->GetValue()) {
        m_GeneChoice->Enable (false);
        m_MapType->Enable(false);
        m_GeneRefPanel->Enable(false);
    } else {
        m_GeneChoice->Enable (true);
        m_MapType->Enable(true);
        if (m_GeneChoice->GetSelection() == 1) {
            m_GeneRefPanel->Enable (true);
        } else{
            m_GeneRefPanel->Enable (false);
        }        
    }
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE16
 */

void CFeatureGenePanel::OnSelectGene( wxCommandEvent& event )
{
    if (m_GeneChoice->GetSelection() == 1) {
        m_GeneRefPanel->Enable (true);
    } else{
        m_GeneRefPanel->Enable (false);
    }
    if (m_GeneChoice->GetSelection() > 1)
        m_ButtonEditGene->Enable();
    else
        m_ButtonEditGene->Disable();
}

void CFeatureGenePanel::OnEditGene(wxCommandEvent& event)
{
    CConstRef<CSeq_feat> gene = GetGeneToEdit();
    if (!gene)
        return;
    CBioseq_Handle bsh = BioseqHandleFromLocation(m_Scope, gene->GetLocation());
    if (!bsh)
        return;

    CSeq_entry_Handle feat_seh = bsh.GetSeq_entry_Handle();
    CIRef<IEditObject> edit(new CEditObjectSeq_feat(*gene, feat_seh, *m_Scope, false));

    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    ICommandProccessor *cmd_processor = NULL;
    wxWindow *win = GetParent();
    while(win)
    {
        CEditObjViewDlg *parent = dynamic_cast<CEditObjViewDlg*>(win);
        if (parent)
        {
            cmd_processor = parent->GetUndoManager();
            break;
        }
        CEditObjViewDlgModal *parent2 = dynamic_cast<CEditObjViewDlgModal*>(win);
        if (parent2)
        {
            cmd_processor = parent2->GetUndoManager();
            break;
        }
        win = win->GetParent();
    }
    if (!cmd_processor)
        return;
    edit_dlg->SetUndoManager(cmd_processor);
    wxWindow* editorWindow = edit->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(edit);
    edit_dlg->SetTitle("Gene");
    edit_dlg->Show(true);
}

void CFeatureGenePanel::SetModifyPseudo(bool modify_pseudo, const string &pseudo_choice)
{
    m_modify_pseudo = modify_pseudo;
    m_pseudo_choice = pseudo_choice;
}

END_NCBI_SCOPE
