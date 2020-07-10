/*  $Id: rawseqtodeltabyn_panel.cpp 43364 2019-06-20 14:46:17Z asztalos $
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
#include <objects/seq/Seq_gap.hpp>
#include <serial/enumvalues.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/rawseqtodeltabyn_panel.hpp>

#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CRawSeqToDeltaByNPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRawSeqToDeltaByNPanel, wxPanel )


/*!
 * CRawSeqToDeltaByNPanel event table definition
 */

BEGIN_EVENT_TABLE( CRawSeqToDeltaByNPanel, wxPanel )

////@begin CRawSeqToDeltaByNPanel event table entries
    EVT_RADIOBUTTON( ID_RAW_SEQ_TO_DELTA_BY_N_RB1, CRawSeqToDeltaByNPanel::GetExplanation )
    EVT_RADIOBUTTON( ID_RAW_SEQ_TO_DELTA_BY_N_RB2, CRawSeqToDeltaByNPanel::GetExplanation )
    EVT_TEXT( ID_RAW_SEQ_TO_DELTA_BY_N_TC1, CRawSeqToDeltaByNPanel::GetExplanation )
    EVT_RADIOBUTTON( ID_RAW_SEQ_TO_DELTA_BY_N_RB3, CRawSeqToDeltaByNPanel::GetExplanation )
    EVT_RADIOBUTTON( ID_RAW_SEQ_TO_DELTA_BY_N_RB4, CRawSeqToDeltaByNPanel::GetExplanation )
    EVT_TEXT( ID_RAW_SEQ_TO_DELTA_BY_N_TC2, CRawSeqToDeltaByNPanel::GetExplanation )
    EVT_CHECKBOX( ID_RAW_SEQ_TO_DELTA_BY_N_ADD_LINKAGE, CRawSeqToDeltaByNPanel::OnAddLinkage )
    EVT_CHOICE( ID_RAW_SEQ_TO_DELTA_BY_N_GAP_TYPE, CRawSeqToDeltaByNPanel::OnGapType )
    EVT_CHOICE( ID_RAW_SEQ_TO_DELTA_BY_N_LINKAGE, CRawSeqToDeltaByNPanel::OnLinkage )
////@end CRawSeqToDeltaByNPanel event table entries

END_EVENT_TABLE()


/*!
 * CRawSeqToDeltaByNPanel constructors
 */

CRawSeqToDeltaByNPanel::CRawSeqToDeltaByNPanel()
{
    Init();
}

CRawSeqToDeltaByNPanel::CRawSeqToDeltaByNPanel( wxWindow* parent, bool adjust_cds, bool add_linkage, bool keep_gap_length,  
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_AdjustCDSValue(adjust_cds), m_AddLinkageValue(add_linkage), m_KeepGapLengthValue(keep_gap_length)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRawSeqToDeltaByNPanel creator
 */

bool CRawSeqToDeltaByNPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRawSeqToDeltaByNPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRawSeqToDeltaByNPanel creation
    return true;
}


/*!
 * CRawSeqToDeltaByNPanel destructor
 */

CRawSeqToDeltaByNPanel::~CRawSeqToDeltaByNPanel()
{
////@begin CRawSeqToDeltaByNPanel destruction
////@end CRawSeqToDeltaByNPanel destruction
}


/*!
 * Member initialisation
 */

void CRawSeqToDeltaByNPanel::Init()
{
////@begin CRawSeqToDeltaByNPanel member initialisation
    m_RadioButton_unknown = NULL;
    m_TextCtrl_unknown = NULL;
    m_RadioButton_known = NULL;
    m_TextCtrl_known = NULL;
    m_Description = NULL;
    m_AdjustCDS = NULL;
    m_AddLinkage = NULL;
    m_GapType = NULL;
    m_Linkage = NULL;
    m_LinkageEvidence = NULL;
    m_KeepGapLength = NULL;
////@end CRawSeqToDeltaByNPanel member initialisation
}


/*!
 * Control creation for CRawSeqToDeltaByNPanel
 */

void CRawSeqToDeltaByNPanel::CreateControls()
{    
////@begin CRawSeqToDeltaByNPanel content construction
    CRawSeqToDeltaByNPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Choose length of Ns to convert to gaps of unknown length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_RadioButton_unknown = new wxRadioButton( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_RB1, _("="), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton_unknown->SetValue(true);
    itemBoxSizer5->Add(m_RadioButton_unknown, 0, wxALIGN_LEFT|wxALL, 5);

    if (RunningInsideNCBI())
    {
        wxRadioButton* itemRadioButton7 = new wxRadioButton( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_RB2, _(">="), wxDefaultPosition, wxDefaultSize, 0 );
        itemRadioButton7->SetValue(false);
        itemBoxSizer5->Add(itemRadioButton7, 0, wxALIGN_LEFT|wxALL, 5);
    }

    m_TextCtrl_unknown = new wxTextCtrl( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_TC1, _("100"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_TextCtrl_unknown, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_KeepGapLength = new wxCheckBox( itemPanel1, ID_CHECKBOX, _("Do not adjust gap length in sequence to 100 N's"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KeepGapLength->SetValue(m_KeepGapLengthValue);
    itemBoxSizer2->Add(m_KeepGapLength, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Choose length of Ns to convert to gaps of known length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer10->Add(itemBoxSizer11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_RadioButton_known = new wxRadioButton( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_RB3, _("="), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RadioButton_known->SetValue(false);
    itemBoxSizer11->Add(m_RadioButton_known, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton13 = new wxRadioButton( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_RB4, _(">="), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton13->SetValue(true);
    itemBoxSizer11->Add(itemRadioButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_TextCtrl_known = new wxTextCtrl( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_TC2, _("101"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_TextCtrl_known, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer15Static = new wxStaticBox(itemPanel1, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer15 = new wxStaticBoxSizer(itemStaticBoxSizer15Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer15, 1, wxGROW|wxALL, 2);

    m_Description = new wxStaticText( itemPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(-1, 50), 0 );
    itemStaticBoxSizer15->Add(m_Description, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    x_SetDescription();

    m_AdjustCDS = new wxCheckBox( itemPanel1, ID_CHECKBOX, _("Adjust CDS locations for gaps"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AdjustCDS->SetValue(m_AdjustCDSValue);
    itemBoxSizer2->Add(m_AdjustCDS, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AddLinkage = new wxCheckBox( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_ADD_LINKAGE, _("Set gap-type and linkage_evidence for assembly_gap features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddLinkage->SetValue(m_AddLinkageValue);
    itemBoxSizer2->Add(m_AddLinkage, 0, wxALIGN_LEFT|wxALL, 5);
    if (m_AddLinkageValue) {
        m_AddLinkage->Hide();
    }

    CEnumeratedTypeValues::TValues type_values = CSeq_gap::ENUM_METHOD_NAME(EType)()->GetValues();
    CEnumeratedTypeValues::TValues linkage_values = CSeq_gap::ENUM_METHOD_NAME(ELinkage)()->GetValues();
    CEnumeratedTypeValues::TValues linkage_evidence_values = CLinkage_evidence::ENUM_METHOD_NAME(EType)()->GetValues();

    wxArrayString type_str, linkage_str, linkage_evidence_str;
    type_str.Add("within");
    type_str.Add("between");
    for (CEnumeratedTypeValues::TValues::const_iterator i = type_values.begin(); i != type_values.end(); ++i)
    {
        type_str.Add(wxString(i->first));
    }

    linkage_str.Add(_("Within Scaffolds"));
    linkage_str.Add(_("Between Scaffolds"));

    for (CEnumeratedTypeValues::TValues::const_iterator i = linkage_evidence_values.begin(); i != linkage_evidence_values.end(); ++i)
    {
        linkage_evidence_str.Add(wxString(i->first));
    }

    wxFlexGridSizer* itemFlexGridSizer19 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Gap Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GapType = new wxChoice( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_GAP_TYPE, wxDefaultPosition, wxDefaultSize, type_str, 0 );
    itemFlexGridSizer19->Add(m_GapType, 0, wxALIGN_LEFT|wxGROW| wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_GapType->SetSelection(0);
    m_GapType->Disable();

    wxStaticText* itemStaticText22 = new wxStaticText( itemPanel1, wxID_STATIC, _("Linkage"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText22, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Linkage = new wxChoice( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_LINKAGE, wxDefaultPosition, wxDefaultSize, linkage_str, 0 );
    itemFlexGridSizer19->Add(m_Linkage, 0, wxALIGN_LEFT| wxGROW |wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Linkage->SetSelection(0);
    m_Linkage->Disable();

    wxStaticText* itemStaticText24 = new wxStaticText( itemPanel1, wxID_STATIC, _("Linkage Evidence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText24, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LinkageEvidence = new wxChoice( itemPanel1, ID_RAW_SEQ_TO_DELTA_BY_N_LINKEVIDENCE, wxDefaultPosition, wxDefaultSize, linkage_evidence_str, 0 );
    itemFlexGridSizer19->Add(m_LinkageEvidence, 0, wxALIGN_LEFT| wxGROW |wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_LinkageEvidence->SetSelection(0);
    m_LinkageEvidence->Disable();

    x_OnAddLinkage();

////@end CRawSeqToDeltaByNPanel content construction
}

/*!
 * Should we show tooltips?
 */

bool CRawSeqToDeltaByNPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRawSeqToDeltaByNPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRawSeqToDeltaByNPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRawSeqToDeltaByNPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRawSeqToDeltaByNPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRawSeqToDeltaByNPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRawSeqToDeltaByNPanel icon retrieval
}

void CRawSeqToDeltaByNPanel::GetMinMax(long &min_known, int &max_known, long &min_unknown, int &max_unknown)

{
    if (!m_TextCtrl_known || !m_TextCtrl_unknown || !m_RadioButton_known || !m_RadioButton_unknown)
        return;
    if (!m_TextCtrl_known->GetValue().ToLong(&min_known))
    {
        min_known = INT_MAX;
    }
    if (!m_TextCtrl_unknown->GetValue().ToLong(&min_unknown))
    {
        min_unknown = INT_MAX;
    }

    max_known = -1;
    if (m_RadioButton_known->GetValue())
    {
        max_known = min_known;
    }
    max_unknown = -1;
    if (m_RadioButton_unknown->GetValue())
    {
        max_unknown = min_unknown;
    }
}

bool CRawSeqToDeltaByNPanel::GetAdjustCDS() const
{ 
    return m_AdjustCDS->GetValue();
}

bool CRawSeqToDeltaByNPanel::GetKeepGapLength() const
{
    return m_KeepGapLength->GetValue();
}

void CRawSeqToDeltaByNPanel::GetAssemblyParams(bool& is_assembly_gap, int& gap_type, int& linkage, int& linkage_evidence)
{
    is_assembly_gap = false;
    gap_type = CSeq_gap::eType_unknown;
    linkage = -1;
    linkage_evidence = -1;
    if (!m_AddLinkage || !m_GapType || !m_Linkage || !m_LinkageEvidence)
        return;

    is_assembly_gap = m_AddLinkage->GetValue();
    if (is_assembly_gap)
    {
        int gap_type_sel = m_GapType->GetSelection();
        int linkage_sel = m_Linkage->GetSelection();
        int linkage_evidence_sel = m_LinkageEvidence->GetSelection();

        if (gap_type_sel != wxNOT_FOUND)
        {
            string gap_type_str = m_GapType->GetString(gap_type_sel).ToStdString();
            if (gap_type_str == "within")
                gap_type_str = "scaffold";
            if (gap_type_str == "between")
                gap_type_str = "contig";
            gap_type = CSeq_gap::ENUM_METHOD_NAME(EType)()->FindValue(gap_type_str);
            linkage = CSeq_gap::eLinkage_unlinked;
            if (gap_type_str == "repeat")
            {
                if (linkage_sel != wxNOT_FOUND)
                {
                    if (m_Linkage->GetString(linkage_sel) == _("Within Scaffolds"))
                    {
                        linkage = CSeq_gap::eLinkage_linked;
                        linkage_evidence = CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue(m_LinkageEvidence->GetString(linkage_evidence_sel).ToStdString());
                    }
                }
            }
            if (gap_type_str == "scaffold")
            {
                linkage = CSeq_gap::eLinkage_linked;
                linkage_evidence = CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue(m_LinkageEvidence->GetString(linkage_evidence_sel).ToStdString());
            }
            if (gap_type_str == "contamination")
            {
                linkage = CSeq_gap::eLinkage_linked;
                linkage_evidence = CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue(m_LinkageEvidence->GetString(linkage_evidence_sel).ToStdString());
            }
        }
    }
}

void CRawSeqToDeltaByNPanel::GetAssemblyParams(bool& is_assembly_gap, string& gap_type, string& linkage, string& linkage_evidence)
{
    int gap_type_val = CSeq_gap::eType_unknown;
    int linkage_val = -1;
    int linkage_evidence_val = -1;
    GetAssemblyParams(is_assembly_gap, gap_type_val, linkage_val, linkage_evidence_val);

    gap_type = linkage = linkage_evidence = kEmptyStr;

    if (!m_AddLinkage || !m_GapType || !m_Linkage || !m_LinkageEvidence)
        return;
    if (is_assembly_gap) {
        int gap_type_sel = m_GapType->GetSelection();
        gap_type = m_GapType->GetString(gap_type_sel).ToStdString();

        if (linkage_val > -1) {
            int linkage_sel = m_Linkage->GetSelection();
            linkage = m_Linkage->GetString(linkage_sel).ToStdString();
        }

        if (linkage_evidence_val > -1) {
            int linkage_evidence_sel = m_LinkageEvidence->GetSelection();
            linkage_evidence = m_LinkageEvidence->GetString(linkage_evidence_sel).ToStdString();
        }
    }
}

/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_RB1
 */

void CRawSeqToDeltaByNPanel::GetExplanation(wxCommandEvent& event)
{
    x_SetDescription();
    event.Skip();
}

void CRawSeqToDeltaByNPanel::x_SetDescription()
{
    long min_known = INT_MAX;
    int max_known;
    long min_unknown = INT_MAX;
    int max_unknown;
    GetMinMax(min_known, max_known, min_unknown, max_unknown);
    wxString text;
    if (min_unknown < INT_MAX)
    {
        if (min_unknown == max_unknown)
            text << _(" All sequences of exactly ") << min_unknown << _(" Ns will be converted to gaps of unknown length.");
        else
            text << _(" All sequences of Ns with lengths >= ") << min_unknown << _(" will be converted to gaps of unknown length.");
    }
    if (min_known < INT_MAX)
    {
        if (min_known == max_known)
            text << _(" All sequences of exactly ") << min_known << _(" Ns will be converted to gaps of known length.");
        else
            text << _(" All sequences of Ns with lengths >= ") << min_known << _(" will be converted to gaps of known length.");
    }

    if (m_Description) {
        m_Description->SetLabelText(text);
        m_Description->Wrap(GetSize().GetWidth() - 30); // TODO ?
    }
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_ADD_LINKAGE
 */

void CRawSeqToDeltaByNPanel::OnAddLinkage(wxCommandEvent& event)
{
    x_OnAddLinkage();
    event.Skip();
}

void CRawSeqToDeltaByNPanel::x_OnAddLinkage()
{
    if (m_AddLinkage->GetValue())
    {
        m_GapType->Enable();
        int gap_type = m_GapType->GetSelection();
        if (gap_type != wxNOT_FOUND)
        {
            auto gap_str = m_GapType->GetString(gap_type);

            if (gap_str == _("repeat"))
            {
                m_Linkage->Enable();
                m_LinkageEvidence->Enable();
            }
            if (gap_str == _("scaffold") || gap_str == _("within"))
            {
                m_LinkageEvidence->Enable();
            }
        }
    }
    else
    {
        m_GapType->Disable();
        m_Linkage->Disable();
        m_LinkageEvidence->Disable();
    }
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_GAP_TYPE
 */

void CRawSeqToDeltaByNPanel::OnGapType(wxCommandEvent& event)
{
    m_Linkage->Disable();
    m_LinkageEvidence->Disable();
    int gap_type = m_GapType->GetSelection();
    int linkage = m_Linkage->GetSelection();
    if (gap_type != wxNOT_FOUND)
    {
        auto gap_str = m_GapType->GetString(gap_type);
        if (gap_str == _("repeat"))
        {
            m_Linkage->Enable();
            if (linkage != wxNOT_FOUND  && m_Linkage->GetString(linkage) == _("Within Scaffolds"))
            {
                m_LinkageEvidence->Enable();
            }
        }
        if (gap_str == _("scaffold") || gap_str == _("within"))
        {
            m_LinkageEvidence->Enable();
        }
        if (gap_str == _("contamination")) {
            m_LinkageEvidence->Enable();
            m_LinkageEvidence->SetSelection(8);
            m_Linkage->SetSelection(0);
        }

    }
    event.Skip();
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RAW_SEQ_TO_DELTA_BY_N_LINKAGE
 */

void CRawSeqToDeltaByNPanel::OnLinkage(wxCommandEvent& event)
{
    int gap_type = m_GapType->GetSelection();
    int linkage = m_Linkage->GetSelection();
    if (gap_type != wxNOT_FOUND  && m_GapType->GetString(gap_type) == _("repeat") &&
        linkage != wxNOT_FOUND  && m_Linkage->GetString(linkage) == _("Within Scaffolds"))
    {
        m_LinkageEvidence->Enable();
    }
    else
    {
        m_LinkageEvidence->Disable();
    }
    event.Skip();
}

END_NCBI_SCOPE

