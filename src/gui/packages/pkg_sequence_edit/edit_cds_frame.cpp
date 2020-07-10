/* $Id: edit_cds_frame.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/retranslate_cds.hpp>
#include <gui/packages/pkg_sequence_edit/edit_cds_frame.hpp>

#include <wx/button.h>

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CEditCdsFrame, wxDialog )



BEGIN_EVENT_TABLE( CEditCdsFrame, wxDialog )

////@begin CEditCdsFrame event table entries
////@end CEditCdsFrame event table entries

END_EVENT_TABLE()


/*!
 * CEditCdsFrame constructors
 */

CEditCdsFrame::CEditCdsFrame()
{
    Init();
}

CEditCdsFrame::CEditCdsFrame( wxWindow* parent, CSeq_entry_Handle seh,
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CEditCdsFrame creator
 */

bool CEditCdsFrame::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditCdsFrame creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditCdsFrame creation

    return true;
}


/*!
 * CEditCdsFrame destructor
 */

CEditCdsFrame::~CEditCdsFrame()
{
////@begin CEditCdsFrame destruction
////@end CEditCdsFrame destruction
}


/*!
 * Member initialisation
 */

void CEditCdsFrame::Init()
{
////@begin CEditCdsFrame member initialisation
    m_From = NULL;
    m_To = NULL;
    m_Retranslate = NULL;
////@end CEditCdsFrame member initialisation
}


/*!
 * Control creation for CEditCdsFrame
 */

void CEditCdsFrame::CreateControls()
{    
////@begin CEditCdsFrame content construction
    CEditCdsFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Set Coding Region frame to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText1, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Where Coding Region frame is:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString to_strs;
    to_strs.Add(_("1"));
    to_strs.Add(_("2"));
    to_strs.Add(_("3"));
    to_strs.Add(_("Best"));
    m_To = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, to_strs, 0 );
    itemBoxSizer6->Add(m_To, 0, wxALIGN_LEFT|wxALL, 5);
    m_To->SetSelection(3);

    wxArrayString from_strs;
    from_strs.Add(_("1"));
    from_strs.Add(_("2"));
    from_strs.Add(_("3"));
    from_strs.Add(_("Any"));
    m_From = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, from_strs, 0 );
    itemBoxSizer6->Add(m_From, 0, wxALIGN_LEFT|wxALL, 5);
    m_From->SetSelection(3);

    m_Retranslate = new wxCheckBox( itemDialog1, wxID_ANY, _("Retranslate adjusted coding regions"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Retranslate->SetValue(true);
    itemBoxSizer2->Add(m_Retranslate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Constraint = new CConstraintPanel( itemDialog1, m_TopSeqEntry);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_LEFT|wxALL, 5);
    m_Constraint->SetSelection(CAECRFrame::eFieldType_Feature);
    m_Constraint->SetFieldName("CDS");


    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CEditCdsFrame content construction
    
}


/*!
 * Should we show tooltips?
 */

bool CEditCdsFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditCdsFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditCdsFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditCdsFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditCdsFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditCdsFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditCdsFrame icon retrieval
}

CRef<CCmdComposite> CEditCdsFrame::GetCommand()
{   
    bool retranslate = m_Retranslate->GetValue();
    string from = m_From->GetString(m_From->GetSelection()).ToStdString();
    string to = m_To->GetString(m_To->GetSelection()).ToStdString();

    CRef<CCmdComposite> cmd(new CCmdComposite("Edit CDS Frame"));
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    CIRef<IEditingAction> action1 =  CreateAction(m_TopSeqEntry, "codon_start", CFieldNamePanel::eFieldType_Feature, CSeqFeatData::eSubtype_cdregion);   
    CRef<CEditingActionConstraint> constraint = m_Constraint->GetConstraint("codon_start", CFieldNamePanel::eFieldType_Feature, CSeqFeatData::eSubtype_cdregion);
                                                                              

    action1->SetConstraint(constraint);
    action1->ResetChangedFeatures();
    action1->NOOP();
    const map<CSeq_feat_Handle, CRef<CSeq_feat> >& feat_map = action1->GetChangedFeatures();

    // offset to use when creating new protein IDs (if needed)
    int offset = 1;
    for (auto& fh_feat : feat_map)
    {
        CSeq_feat_Handle fh = fh_feat.first;
        CRef<CSeq_feat> feat = fh_feat.second;
        if  (!feat->IsSetData() || !feat->GetData().IsCdregion())
            continue;

        if (feat->IsSetExcept_text() && NStr::Find(feat->GetExcept_text(), "RNA editing") != string::npos) 
        {
            continue;
        }

        bool go(false);
        if (from == "Any")
            go = true;

        if (!go && feat->GetData().GetCdregion().IsSetFrame())
        {
            CCdregion::EFrame frame = feat->GetData().GetCdregion().GetFrame();
            if (from == "1" && (frame == CCdregion::eFrame_one || frame == CCdregion::eFrame_not_set))
                go = true;
            if (from == "2" && frame == CCdregion::eFrame_two)
                go = true;
            if (from == "3" && frame == CCdregion::eFrame_three)
                go = true;
        }
        if (!go)
            continue;

        if (to == "1")
            feat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_one);
        if (to == "2")
            feat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_two);
        if (to == "3")
            feat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_three);
        if (to == "Best")
            feat->SetData().SetCdregion().SetFrame(CSeqTranslator::FindBestFrame(*feat, fh.GetScope()));

        CIRef<IEditCommand>  chgFeat(new CCmdChangeSeq_feat(fh, *feat));
        cmd->AddCommand(*chgFeat);

        if (retranslate)
        {
            bool cds_change(false);
            CRef<CCmdComposite> retranslate_cds_cmd =  CRetranslateCDS::RetranslateCDSCommand(fh.GetScope(), *fh.GetOriginalSeq_feat(), *feat, cds_change, offset, create_general_only);
            cmd->AddCommand(*retranslate_cds_cmd);
        }
    }


    return cmd;
}

END_NCBI_SCOPE
