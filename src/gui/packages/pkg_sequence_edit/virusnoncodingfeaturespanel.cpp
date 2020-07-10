/*  $Id: virusnoncodingfeaturespanel.cpp 28329 2013-06-19 11:58:38Z bollin $
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
#include <objmgr/bioseq_ci.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <gui/objutils/cmd_create_feat.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/virusnoncodingfeaturespanel.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CVirusNonCodingFeaturesPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CVirusNonCodingFeaturesPanel, CBulkCmdPanel )


/*!
 * CVirusNonCodingFeaturesPanel event table definition
 */

BEGIN_EVENT_TABLE( CVirusNonCodingFeaturesPanel, CBulkCmdPanel )

////@begin CVirusNonCodingFeaturesPanel event table entries
    EVT_RADIOBOX( ID_RADIOBOX, CVirusNonCodingFeaturesPanel::OnFeatureTypeSelected )

////@end CVirusNonCodingFeaturesPanel event table entries

END_EVENT_TABLE()


/*!
 * CVirusNonCodingFeaturesPanel constructors
 */

CVirusNonCodingFeaturesPanel::CVirusNonCodingFeaturesPanel()
{
    Init();
}

CVirusNonCodingFeaturesPanel::CVirusNonCodingFeaturesPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CVirusNonCodingFeaturesPanel creator
 */

bool CVirusNonCodingFeaturesPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CVirusNonCodingFeaturesPanel creation
    CBulkCmdPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CVirusNonCodingFeaturesPanel creation
    return true;
}


/*!
 * CVirusNonCodingFeaturesPanel destructor
 */

CVirusNonCodingFeaturesPanel::~CVirusNonCodingFeaturesPanel()
{
////@begin CVirusNonCodingFeaturesPanel destruction
////@end CVirusNonCodingFeaturesPanel destruction
}


/*!
 * Member initialisation
 */

void CVirusNonCodingFeaturesPanel::Init()
{
////@begin CVirusNonCodingFeaturesPanel member initialisation
    m_FeatureType = NULL;
    m_ExtraText = NULL;
    m_PartialBox = NULL;
    m_Partial5 = NULL;
    m_Partial3 = NULL;
////@end CVirusNonCodingFeaturesPanel member initialisation
}


/*!
 * Control creation for CVirusNonCodingFeaturesPanel
 */

void CVirusNonCodingFeaturesPanel::CreateControls()
{    
////@begin CVirusNonCodingFeaturesPanel content construction
    // Generated by DialogBlocks, 10/06/2013 17:56:05 (unregistered)

    CVirusNonCodingFeaturesPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_FeatureTypeStrings;
    m_FeatureTypeStrings.Add(_("&Long Terminal Repeat (LTR)"));
    m_FeatureTypeStrings.Add(_("&5' Untranslated Region (UTR)"));
    m_FeatureTypeStrings.Add(_("&3' Untranslated Region (UTR)"));
    m_FeatureTypeStrings.Add(_("Viroid &complete genome"));
    m_FeatureTypeStrings.Add(_("Viroid &partial genome"));
    m_FeatureTypeStrings.Add(_("Something else"));
    m_FeatureType = new wxRadioBox( itemCBulkCmdPanel1, ID_RADIOBOX, _("Feature Type"), wxDefaultPosition, wxDefaultSize, m_FeatureTypeStrings, 1, wxRA_SPECIFY_COLS );
    m_FeatureType->SetSelection(0);
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ExtraText = new wxTextCtrl( itemCBulkCmdPanel1, ID_NONCODINGVIRUSFEATURETYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ExtraText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_PartialBox = new wxStaticBox(itemCBulkCmdPanel1, wxID_ANY, _("Feature is:"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(m_PartialBox, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Partial5 = new wxCheckBox( itemCBulkCmdPanel1, ID_PARTIAL5, _("Incomplete at 5' end"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial5->SetValue(false);
    itemStaticBoxSizer5->Add(m_Partial5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Partial3 = new wxCheckBox( itemCBulkCmdPanel1, ID_PARTIAL3, _("Incomplete at 3' end"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial3->SetValue(false);
    itemStaticBoxSizer5->Add(m_Partial3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CVirusNonCodingFeaturesPanel content construction
    m_ExtraText->Show (false);
}


/*!
 * Should we show tooltips?
 */

bool CVirusNonCodingFeaturesPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CVirusNonCodingFeaturesPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CVirusNonCodingFeaturesPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CVirusNonCodingFeaturesPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CVirusNonCodingFeaturesPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVirusNonCodingFeaturesPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVirusNonCodingFeaturesPanel icon retrieval
}


CRef<CCmdComposite> CVirusNonCodingFeaturesPanel::GetCommand()
{
    CRef<CCmdComposite> cmd (new CCmdComposite("Bulk Add Virus Noncoding Features"));

    string comment = "";
    string key = "";
    
    int sel = m_FeatureType->GetSelection();
    switch (sel) {
        case 0:
            key = "LTR";
            break;
        case 1:
            key = "5'UTR";
            break;
        case 2:
            key = "3'UTR";
            break;
        case 3:
        case 4:
            key = "misc_feature";
            comment = m_FeatureType->GetStringSelection();
            break;
        case 5:
            key = "misc_feature";
            comment = m_ExtraText->GetValue();
            break;
    }
    if (NStr::IsBlank(key)
        || (NStr::Equal(key, "misc_feature") && NStr::IsBlank(comment))) {
        CRef<CCmdComposite> empty;
        return empty;
    }
    
    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        if (AlreadyHasFeature(*b_iter, key, comment)) {
            continue;
        }
        CRef<objects::CSeq_feat> new_feat(new objects::CSeq_feat());
        new_feat->SetData().SetImp().SetKey(key);
        if (!NStr::IsBlank(comment)) {
            new_feat->SetComment(comment);
        }
        CRef<objects::CSeq_id> new_feat_id(new objects::CSeq_id());
        new_feat_id->Assign(*(b_iter->GetCompleteBioseq()->GetId().front()));
        new_feat->SetLocation().SetInt().SetId(*new_feat_id);
        new_feat->SetLocation().SetInt().SetFrom(0);
        new_feat->SetLocation().SetInt().SetTo(b_iter->GetBioseqLength() - 1);
        if (sel < 3) {
            if (m_Partial5->GetValue()) {
                new_feat->SetLocation().SetPartialStart(true, objects::eExtreme_Biological);
            }
            if (m_Partial3->GetValue()) {
                new_feat->SetLocation().SetPartialStop(true, objects::eExtreme_Biological);
            }
        }
        objects::CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    }
    return cmd;
}


string CVirusNonCodingFeaturesPanel::GetErrorMessage()
{
    return "You must specify the type of feature to be created.";
}



/*!
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_RADIOBOX
 */

void CVirusNonCodingFeaturesPanel::OnFeatureTypeSelected( wxCommandEvent& event )
{
    int sel = m_FeatureType->GetSelection();
    if (sel == 5) {
        m_ExtraText->Show (true);
    } else {
        m_ExtraText->Show (false);
    }

    // show/hide partials
    if (sel < 3) {
        x_ShowPartial ();
    } else {
        x_HidePartial ();
    }
    Layout();
    Fit();
}


void CVirusNonCodingFeaturesPanel::x_HidePartial ()
{
    m_PartialBox->Show (false);
    m_Partial5->Show (false);
    m_Partial3->Show (false);
}


void CVirusNonCodingFeaturesPanel::x_ShowPartial ()
{
    m_PartialBox->Show (true);
    m_Partial5->Show (true);
    m_Partial3->Show (true);
}


END_NCBI_SCOPE
