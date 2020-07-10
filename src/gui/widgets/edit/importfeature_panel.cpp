/*  $Id: importfeature_panel.cpp 38194 2017-04-07 18:54:32Z bollin $
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
#include <objects/seqfeat/Imp_feat.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
////@begin includes
////@end includes

#include "importfeature_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CImportFeaturePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CImportFeaturePanel, wxPanel )


/*!
 * CImportFeaturePanel event table definition
 */

BEGIN_EVENT_TABLE( CImportFeaturePanel, wxPanel )

////@begin CImportFeaturePanel event table entries
    EVT_CHOICE( ID_CHOICE3, CImportFeaturePanel::OnChoice3Selected )

////@end CImportFeaturePanel event table entries

END_EVENT_TABLE()


/*!
 * CImportFeaturePanel constructors
 */

CImportFeaturePanel::CImportFeaturePanel()
{
    Init();
}

CImportFeaturePanel::CImportFeaturePanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    m_Object = dynamic_cast<CSeq_feat*>(&object);
    const CSeq_feat& seqfeat = dynamic_cast<const CSeq_feat&>(*m_Object);
    m_EditedFeat.Reset((CSerialObject*)CSeq_feat::GetTypeInfo()->Create());
    m_EditedFeat->Assign(seqfeat);
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CImportFeatureSubPanel creator
 */

bool CImportFeaturePanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CImportFeaturePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CImportFeaturePanel creation
    return true;
}


/*!
 * CImportFeaturePanel destructor
 */

CImportFeaturePanel::~CImportFeaturePanel()
{
////@begin CImportFeaturePanel destruction
////@end CImportFeaturePanel destruction
}


/*!
 * Member initialisation
 */

void CImportFeaturePanel::Init()
{
////@begin CImportFeaturePanel member initialisation
    m_KeyCtrl = NULL;
////@end CImportFeaturePanel member initialisation
}


/*!
 * Control creation for CImportFeatureSubPanel
 */

void CImportFeaturePanel::CreateControls()
{    
////@begin CImportFeaturePanel content construction
    CImportFeaturePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Key"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_KeyCtrlStrings;
    m_KeyCtrlStrings.Add(_("C_region"));
    m_KeyCtrlStrings.Add(_("centromere"));
    m_KeyCtrlStrings.Add(_("D-loop"));
    m_KeyCtrlStrings.Add(_("D_segment"));
    m_KeyCtrlStrings.Add(_("exon"));
    m_KeyCtrlStrings.Add(_("iDNA"));
    m_KeyCtrlStrings.Add(_("intron"));
    m_KeyCtrlStrings.Add(_("J_segment"));
    m_KeyCtrlStrings.Add(_("misc_binding"));
    m_KeyCtrlStrings.Add(_("misc_difference"));
    m_KeyCtrlStrings.Add(_("misc_feature"));
    m_KeyCtrlStrings.Add(_("misc_recomb"));
    m_KeyCtrlStrings.Add(_("misc_structure"));
    m_KeyCtrlStrings.Add(_("mobile_element"));
    m_KeyCtrlStrings.Add(_("modified_base"));
    m_KeyCtrlStrings.Add(_("N_region"));
    m_KeyCtrlStrings.Add(_("operon"));
    m_KeyCtrlStrings.Add(_("oriT"));
    m_KeyCtrlStrings.Add(_("polyA_site"));
    m_KeyCtrlStrings.Add(_("prim_transcript"));
    m_KeyCtrlStrings.Add(_("primer_bind"));
    m_KeyCtrlStrings.Add(_("protein_bind"));
    m_KeyCtrlStrings.Add(_("regulatory"));
    m_KeyCtrlStrings.Add(_("repeat_region"));
    m_KeyCtrlStrings.Add(_("rep_origin"));
    m_KeyCtrlStrings.Add(_("S_region"));
    m_KeyCtrlStrings.Add(_("stem_loop"));
    m_KeyCtrlStrings.Add(_("telomere"));
    m_KeyCtrlStrings.Add(_("unsure"));
    m_KeyCtrlStrings.Add(_("V_region"));
    m_KeyCtrlStrings.Add(_("V_segment"));
    m_KeyCtrlStrings.Add(_("variation"));
    m_KeyCtrlStrings.Add(_("3'clip"));
    m_KeyCtrlStrings.Add(_("3'UTR"));
    m_KeyCtrlStrings.Add(_("5'clip"));
    m_KeyCtrlStrings.Add(_("5'UTR"));
    m_KeyCtrl = new wxChoice( itemPanel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, m_KeyCtrlStrings, 0 );
    m_KeyCtrl->SetStringSelection(_("misc_feature"));
    itemBoxSizer3->Add(m_KeyCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 1, wxGROW|wxALL, 0);

////@end CImportFeaturePanel content construction
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);

    if (edited_feat.GetData().GetImp().IsSetKey()) {
        string key = edited_feat.GetData().GetImp().GetKey();
        if (!NStr::IsBlank(key)) {
            m_KeyCtrl->SetStringSelection(ToWxString(key));
        }
    }

    m_GBQualSizer = itemBoxSizer6;
    m_GBQualPanel = new CGBQualPanel (itemPanel1, *m_Object);
    m_GBQualSizer->Add(m_GBQualPanel, 1, wxGROW|wxALL, 0);
}


bool CImportFeaturePanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    return m_GBQualPanel->TransferDataToWindow();
}


bool CImportFeaturePanel::TransferDataFromWindow()
{
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    if (!wxPanel::TransferDataFromWindow())
        return false;
    wxString key = m_KeyCtrl->GetStringSelection ();
    edited_feat.SetData().SetImp().SetKey(ToStdString(key));
    m_GBQualPanel->PopulateGBQuals(edited_feat);
    return true;
}


void CImportFeaturePanel::PopulateImpFeat(objects::CSeq_feat& seq_feat)
{
    TransferDataFromWindow();
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    seq_feat.SetData().SetImp().SetKey(edited_feat.GetData().GetImp().GetKey());
    m_GBQualPanel->PopulateGBQuals(seq_feat);
}


/*!
 * Should we show tooltips?
 */

bool CImportFeaturePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CImportFeaturePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CImportFeaturePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CImportFeaturePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CImportFeaturePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CImportFeaturePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CImportFeaturePanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE3
 */

void CImportFeaturePanel::OnChoice3Selected( wxCommandEvent& event )
{
    TransferDataFromWindow();

    m_GBQualSizer->DeleteWindows();
    m_GBQualPanel = new CGBQualPanel (this, *m_EditedFeat);
    m_GBQualSizer->Add(m_GBQualPanel, 1, wxGROW|wxALL, 0);
    m_GBQualSizer->Layout();
    TransferDataToWindow();
}


END_NCBI_SCOPE
