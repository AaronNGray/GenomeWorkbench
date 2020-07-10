/*  $Id: publishedref_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/edit/publishedref_panel.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/ArticleIdSet.hpp>
#include <objects/biblio/ArticleId.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/objutils/doi_lookup.hpp>
#include <gui/widgets/edit/pubstatus_panel.hpp>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h> 
BEGIN_NCBI_SCOPE

/*
 * CPublishedRefPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPublishedRefPanel, wxPanel )


/*
 * CPublishedRefPanel event table definition
 */

BEGIN_EVENT_TABLE( CPublishedRefPanel, wxPanel )

////@begin CPublishedRefPanel event table entries
    EVT_BUTTON( ID_PUBPMIDLOOKBTN, CPublishedRefPanel::OnLookupClick )
////@end CPublishedRefPanel event table entries

END_EVENT_TABLE()


/*
 * CPublishedRefPanel constructors
 */

CPublishedRefPanel::CPublishedRefPanel()
    : m_is_inpress(false)
{
    Init();
}

CPublishedRefPanel::CPublishedRefPanel( wxWindow* parent, bool is_inpress, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_is_inpress(is_inpress)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CPublishedRefPanel creator
 */

bool CPublishedRefPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPublishedRefPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPublishedRefPanel creation
    return true;
}


/*
 * CPublishedRefPanel destructor
 */

CPublishedRefPanel::~CPublishedRefPanel()
{
////@begin CPublishedRefPanel destruction
////@end CPublishedRefPanel destruction
}


/*
 * Member initialisation
 */

void CPublishedRefPanel::Init()
{
////@begin CPublishedRefPanel member initialisation
    m_ChoiceCtrl = NULL;
    m_PubMedID = NULL;
    m_ReferenceTitle = NULL;
    m_JournalTitle = NULL;
    m_YearCtrl = NULL;
    m_VolCtrl = NULL;
    m_IssueCtrl = NULL;
    m_PagesFromCtrl = NULL;
    m_PagesToCtrl = NULL;
////@end CPublishedRefPanel member initialisation
    m_Pmid = 0;
}


/*
 * Control creation for CPublishedRefPanel
 */

void CPublishedRefPanel::CreateControls()
{    
////@begin CPublishedRefPanel content construction
    CPublishedRefPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer1, 0, wxALIGN_LEFT|wxALL, 0);

    wxArrayString m_ChoiceCtrlStrings;
    m_ChoiceCtrlStrings.Add(_("PubMed ID"));
    m_ChoiceCtrlStrings.Add(_("DOI"));
    m_ChoiceCtrl = new wxChoice( itemPanel1, ID_LOOKUPCHOICE, wxDefaultPosition, wxDefaultSize, m_ChoiceCtrlStrings, 0 );
    itemBoxSizer1->Add(m_ChoiceCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);
    m_ChoiceCtrl->SetSelection(0);
    if (m_is_inpress)
    {
        m_ChoiceCtrl->Disable();
    }

    m_PubMedID = new wxTextCtrl( itemPanel1, ID_PUBPMIDTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer1->Add(m_PubMedID, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxButton* itemButton4 = new wxButton( itemPanel1, ID_PUBPMIDLOOKBTN, _("Lookup"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemButton4, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Reference title*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Journal title*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_ReferenceTitle = new wxTextCtrl( itemPanel1, ID_PUBREFTITLE, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer1->Add(m_ReferenceTitle, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_JournalTitle = new wxTextCtrl( itemPanel1, ID_PUBJOURNAL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer1->Add(m_JournalTitle, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer2, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Year*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Volume*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Issue"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages from*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages to*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_YearCtrl = new wxTextCtrl( itemPanel1, ID_PUBYEAR, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_YearCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_YearCtrl->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    m_VolCtrl = new wxTextCtrl( itemPanel1, ID_PUBVOL, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_VolCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IssueCtrl = new wxTextCtrl( itemPanel1, ID_PUBISSUE, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_IssueCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PagesFromCtrl = new wxTextCtrl( itemPanel1, ID_PUBPAGESFROM, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_PagesFromCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PagesToCtrl = new wxTextCtrl( itemPanel1, ID_PUBPAGESTO, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_PagesToCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CPublishedRefPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CPublishedRefPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPublishedRefPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPublishedRefPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPublishedRefPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CPublishedRefPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPublishedRefPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPublishedRefPanel icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PUBPMIDLOOKBTN
 */

void CPublishedRefPanel::OnLookupClick( wxCommandEvent& event )
{
    if (m_PubMedID->GetValue().IsEmpty())
        return;
    CRef<objects::CPubdesc> pubdesc;
    string error;
    switch (m_ChoiceCtrl->GetSelection())
    {
    case wxNOT_FOUND : break;
    case 0 :
    {
        int pmid = wxAtoi(m_PubMedID->GetValue());
        if (pmid < 1)
            return;
        pubdesc = GetPubdescFromEntrezById(pmid);
    }
    break;
    case 1 : 
    {
        pair<CRef<objects::CPubdesc>, string> pubdesc_str = CDoiLookup::GetPubFromCrossRef(m_PubMedID->GetValue().ToStdString());
        pubdesc = pubdesc_str.first;
        error = pubdesc_str.second;
    }
    break;
    default : break;
    }
    
    if (!pubdesc) 
    {
	wxMessageBox(wxT("No article found \n") + wxString(error), wxT("Error"), wxOK | wxICON_ERROR, NULL);
	return;
    }
    else
    {
        CPubStatusPanel* status_panel = dynamic_cast<CPubStatusPanel*> (GetGrandParent());
        if (status_panel)
            status_panel->ApplyPub(*pubdesc);
    }
}

void CPublishedRefPanel::SetCitArt(objects::CCit_art& art)
{
    m_Art.Reset(&art);
    TransferDataToWindow();
}


void CPublishedRefPanel::SetPmid(int pmid)
{
    m_Pmid = pmid;
    x_UpdateDOIPmidControls();
}


void CPublishedRefPanel::x_UpdateDOIPmidControls()
{
    if (m_Pmid > 0) {
        m_ChoiceCtrl->SetSelection(0);
        m_ChoiceCtrl->Disable();
        m_PubMedID->SetValue(ToWxString(NStr::NumericToString(m_Pmid)));
    } else if (!m_is_inpress) {
        m_ChoiceCtrl->Enable();
        bool found_doi = false;
        if (m_Art->IsSetIds()) {
            for (auto id : m_Art->GetIds().Get()) {
                if (id->IsDoi()) {
                    m_PubMedID->SetValue(ToWxString(id->GetDoi()));
                    m_ChoiceCtrl->SetSelection(1);
                    found_doi = true;
                    break;
                }
            }
        }
        if (!found_doi) {
            m_PubMedID->SetValue(wxT(""));
        }
    }
}


void CPublishedRefPanel::x_Reset()
{
    m_ChoiceCtrl->SetSelection(0);
    m_PubMedID->SetValue(wxT(""));
    m_ReferenceTitle->SetValue(wxT(""));
    m_JournalTitle->SetValue(wxT(""));
    m_YearCtrl->SetValue(wxT(""));
    m_VolCtrl->SetValue(wxT(""));
    m_IssueCtrl->SetValue(wxT(""));
    m_PagesFromCtrl->SetValue(wxT(""));
    m_PagesToCtrl->SetValue(wxT(""));
}


bool CPublishedRefPanel::TransferDataToWindow()
{
    x_Reset();
    if (!m_Art) {
        return false;
    }
    if (m_Art->IsSetTitle() && m_Art->GetTitle().IsSet() && !m_Art->GetTitle().Get().empty()) {
        m_ReferenceTitle->SetValue(ToWxString(GetStringFromTitle(*(m_Art->GetTitle().Get().front()))));
    }
    if (m_Art->IsSetFrom() && m_Art->GetFrom().IsJournal()) {
        const objects::CCit_jour& jour = m_Art->GetFrom().GetJournal();
        if (jour.IsSetTitle() && jour.GetTitle().IsSet() && !jour.GetTitle().Get().empty()) {
            m_JournalTitle->SetValue(ToWxString(GetStringFromTitle(*(jour.GetTitle().Get().front()))));
        }
        if (jour.IsSetImp()) {
            const objects::CImprint& imp = jour.GetImp();
            if (imp.IsSetDate() && imp.GetDate().IsStd() &&
                imp.GetDate().GetStd().IsSetYear()) {
                m_YearCtrl->SetValue(ToWxString(NStr::NumericToString(imp.GetDate().GetStd().GetYear())));
            }
            if (imp.IsSetVolume()) {
                m_VolCtrl->SetValue(ToWxString(imp.GetVolume()));
            }
            if (imp.IsSetIssue()) {
                m_IssueCtrl->SetValue(ToWxString(imp.GetIssue()));
            }
            if (imp.IsSetPages()) {
                const string& pages = imp.GetPages();
                size_t pos = NStr::Find(pages, "-");
                if (pos == NPOS) {
                    m_PagesFromCtrl->SetValue(ToWxString(pages));
                }
                else {
                    m_PagesFromCtrl->SetValue(ToWxString(pages.substr(0, pos)));
                    m_PagesToCtrl->SetValue(ToWxString(pages.substr(pos + 1)));
                }
            }
        }
    }
    x_UpdateDOIPmidControls();
    return true;
}


bool CPublishedRefPanel::TransferDataFromWindow()
{
    if (!m_Art) {
        return false;
    }

    // article title
    if (m_ReferenceTitle->IsEmpty()) {
        m_Art->ResetTitle();
    } else {
        if (m_Art->SetTitle().Set().empty()) {
            m_Art->SetTitle().Set().push_back(CRef<objects::CTitle::C_E>(new objects::CTitle::C_E()));            
        }
        UpdateTitle(*(m_Art->SetTitle().Set().front()), ToStdString(m_ReferenceTitle->GetValue()));
    }

    objects::CCit_jour& journal = m_Art->SetFrom().SetJournal();
    // journal title is required, so use ? if blank
    if (journal.SetTitle().Set().empty()) {
        journal.SetTitle().Set().push_back(CRef<objects::CTitle::C_E>(new objects::CTitle::C_E()));
    }
    UpdateTitle(*(journal.SetTitle().Set().front()),  m_JournalTitle->IsEmpty() ? "?": ToStdString(m_JournalTitle->GetValue()));

    objects::CImprint& imp = journal.SetImp();
    if (m_YearCtrl->IsEmpty()) {
        imp.SetDate().SetStr("?");
    } else {
        int year = NStr::StringToInt(ToStdString(m_YearCtrl->GetValue()));
        imp.SetDate().SetStd().SetYear(year);
    }
    if (m_VolCtrl->IsEmpty()) {
        imp.ResetVolume();
    } else {
        imp.SetVolume(ToStdString(m_VolCtrl->GetValue()));
    }
    if (m_IssueCtrl->IsEmpty()) {
        imp.ResetIssue();
    } else {
        imp.SetIssue(ToStdString(m_IssueCtrl->GetValue()));
    }
    if (m_PagesFromCtrl->IsEmpty() && m_PagesToCtrl->IsEmpty()) {
        imp.ResetPages();
    } else if (m_PagesFromCtrl->IsEmpty()) {
        imp.SetPages(ToStdString(m_PagesFromCtrl->GetValue()));
    } else if (m_PagesToCtrl->IsEmpty()) {
        imp.SetPages(ToStdString(m_PagesToCtrl->GetValue()));
    } else {
        imp.SetPages(ToStdString(m_PagesFromCtrl->GetValue()) + "-" + ToStdString(m_PagesToCtrl->GetValue()));
    }

    // TODO: PMID/DOI

    return true;
}

void CPublishedRefPanel::ReportMissingFields(string &text)
{
    if (m_ReferenceTitle->GetValue().IsEmpty())
        text += "Reference Title\n";
    if (m_JournalTitle->GetValue().IsEmpty() || m_JournalTitle->GetValue() == _("?"))
        text += "Journal Title\n";
    if (m_YearCtrl->GetValue().IsEmpty())
        text += "Publication Year\n";
    if (m_VolCtrl->GetValue().IsEmpty())
        text += "Publication Volume\n";
    if (m_PagesFromCtrl->GetValue().IsEmpty())
        text += "Pages From\n";
    if (m_PagesToCtrl->GetValue().IsEmpty())
        text += "Pages To\n";
}

END_NCBI_SCOPE

