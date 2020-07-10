/*  $Id: relaxed_lookup_dlg.cpp 41483 2018-08-03 15:33:33Z evgeniev $
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

////@begin includes
////@end includes

#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/general/Date_std.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/Person_id.hpp>

#include <objects/mla/Mla_back.hpp>
#include <objects/mla/mla_client.hpp>
#include <gui/objutils/gui_eutils_client.hpp>
#include <misc/xmlwrapp/document.hpp>
#include <misc/hydra_client/hydra_client.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/relaxed_lookup_dlg.hpp>
#include <gui/utils/app_popup.hpp>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/statbox.h>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CRelaxedLookupDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRelaxedLookupDlg, wxDialog )


/*
 * CRelaxedLookupDlg event table definition
 */

BEGIN_EVENT_TABLE( CRelaxedLookupDlg, wxDialog )

////@begin CRelaxedLookupDlg event table entries
    EVT_BUTTON( ID_SEND_QUERY, CRelaxedLookupDlg::OnSendQueryClick )

////@end CRelaxedLookupDlg event table entries

END_EVENT_TABLE()


/*
 * CRelaxedLookupDlg constructors
 */

CRelaxedLookupDlg::CRelaxedLookupDlg()
{
    Init();
}

CRelaxedLookupDlg::CRelaxedLookupDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CRelaxedLookupDlg creator
 */

bool CRelaxedLookupDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRelaxedLookupDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRelaxedLookupDlg creation
    return true;
}


/*
 * CRelaxedLookupDlg destructor
 */

CRelaxedLookupDlg::~CRelaxedLookupDlg()
{
////@begin CRelaxedLookupDlg destruction
////@end CRelaxedLookupDlg destruction
}


/*
 * Member initialisation
 */

void CRelaxedLookupDlg::Init()
{
////@begin CRelaxedLookupDlg member initialisation
    m_MaxMatchTxt = NULL;
    m_UseFirstAuthorBtn = NULL;
    m_FirstAuthorTxt = NULL;
    m_UseLastAuthorBtn = NULL;
    m_LastAuthorTxt = NULL;
    m_UseJournalBtn = NULL;
    m_JournalTxt = NULL;
    m_UseYearBtn = NULL;
    m_YearTxt = NULL;
    m_YearFuzzBtn = NULL;
    m_UseVolumeBtn = NULL;
    m_UseVolumeTxt = NULL;
    m_UsePageBtn = NULL;
    m_UsePageTxt = NULL;
    m_ArticleTitleTxt = NULL;
    m_UseExtraTermsBtn = NULL;
    m_ExtraTermsTxt = NULL;
    m_NewQueryChoice = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CRelaxedLookupDlg member initialisation
}


/*
 * Control creation for CRelaxedLookupDlg
 */

void CRelaxedLookupDlg::CreateControls()
{    
////@begin CRelaxedLookupDlg content construction
    CRelaxedLookupDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Number of potential matches (Max 20):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MaxMatchTxt = new wxTextCtrl( itemDialog1, ID_MAX_MATCH_TXT, _("20"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_MaxMatchTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Available information on this in-press citation:"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_UseFirstAuthorBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_USE_FIRST_AUTHOR_BTN, _("First Author"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseFirstAuthorBtn->SetValue(false);
    itemBoxSizer7->Add(m_UseFirstAuthorBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FirstAuthorTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_FIRST_AUTHOR_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_FirstAuthorTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UseLastAuthorBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_USE_LAST_AUTHOR_BTN, _("Last Author"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseLastAuthorBtn->SetValue(false);
    itemBoxSizer7->Add(m_UseLastAuthorBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LastAuthorTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_LAST_AUTHOR_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_LastAuthorTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer6->Add(itemBoxSizer12, 0, wxGROW|wxALL, 5);

    m_UseJournalBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_USE_JOURNAL_BTN, _("Journal"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseJournalBtn->SetValue(false);
    itemBoxSizer12->Add(m_UseJournalBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_JournalTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_JOURNAL_TXT, wxEmptyString, wxDefaultPosition, wxSize(500, -1), 0 );
    itemBoxSizer12->Add(m_JournalTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer6->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_UseYearBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_USE_YEAR_BTN, _("Year"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseYearBtn->SetValue(false);
    itemBoxSizer15->Add(m_UseYearBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_YearTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_YEAR_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_YearTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_YearFuzzBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_YEAR_FUZZ_BTN, _("+/- 1 Year"), wxDefaultPosition, wxDefaultSize, 0 );
    m_YearFuzzBtn->SetValue(true);
    itemBoxSizer15->Add(m_YearFuzzBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UseVolumeBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_USE_VOLUME_BTN, _("Volume"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseVolumeBtn->SetValue(false);
    itemBoxSizer15->Add(m_UseVolumeBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UseVolumeTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_USE_VOLUME_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_UseVolumeTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UsePageBtn = new wxCheckBox( itemStaticBoxSizer6->GetStaticBox(), ID_USE_PAGE_BTN, _("Page"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UsePageBtn->SetValue(false);
    itemBoxSizer15->Add(m_UsePageBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UsePageTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_PAGE_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_UsePageTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer6->Add(itemBoxSizer23, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText24 = new wxStaticText( itemStaticBoxSizer6->GetStaticBox(), wxID_STATIC, _("Article Title:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ArticleTitleTxt = new wxTextCtrl( itemStaticBoxSizer6->GetStaticBox(), ID_ARTICLE_TITLE_TXT, wxEmptyString, wxDefaultPosition, wxSize(500, -1), wxTE_MULTILINE );
    itemBoxSizer23->Add(m_ArticleTitleTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer26, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_UseExtraTermsBtn = new wxCheckBox( itemDialog1, ID_USE_EXTRA_TERMS_BTN, _("Add these terms to new query"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseExtraTermsBtn->SetValue(false);
    itemBoxSizer26->Add(m_UseExtraTermsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ExtraTermsTxt = new wxTextCtrl( itemDialog1, ID_EXTRA_TERMS_TXT, wxEmptyString, wxDefaultPosition, wxSize(400, -1), 0 );
    itemBoxSizer26->Add(m_ExtraTermsTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer29 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer29, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText30 = new wxStaticText( itemDialog1, wxID_STATIC, _("New query option:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer29->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_NewQueryChoiceStrings;
    m_NewQueryChoiceStrings.Add(_("default: use all fields"));
    m_NewQueryChoiceStrings.Add(_("customize: use checked fields"));
    m_NewQueryChoice = new wxChoice( itemDialog1, ID_NEW_QUERY_CHOICE, wxDefaultPosition, wxDefaultSize, m_NewQueryChoiceStrings, 0 );
    m_NewQueryChoice->SetStringSelection(_("default: use all fields"));
    itemBoxSizer29->Add(m_NewQueryChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton32 = new wxButton( itemDialog1, ID_SEND_QUERY, _("Send Modified Query"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer29->Add(itemButton32, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText33 = new wxStaticText( itemDialog1, wxID_STATIC, _("Summary of potential matches, highlight one to use in update"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText33, 0, wxALIGN_LEFT|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemDialog1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 0, wxGROW|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->SetMinSize(wxSize(100, 200));

    wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer36, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton37 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer36->Add(itemButton37, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton38 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer36->Add(itemButton38, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRelaxedLookupDlg content construction
}


/*
 * Should we show tooltips?
 */

bool CRelaxedLookupDlg::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CRelaxedLookupDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRelaxedLookupDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRelaxedLookupDlg bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CRelaxedLookupDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRelaxedLookupDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRelaxedLookupDlg icon retrieval
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEND_QUERY
 */

void CRelaxedLookupDlg::OnSendQueryClick( wxCommandEvent& event )
{
    SendQuery();
}


void CRelaxedLookupDlg::OnPubTextClick( wxMouseEvent& event )
{
    // Find radio button next to text, select
    wxStaticText* item = (wxStaticText*)event.GetEventObject();
    wxWindow* prev = item->GetPrevSibling();
    if (prev) {
        wxRadioButton* btn = dynamic_cast<wxRadioButton*>(prev);
        if (btn) {
            btn->SetValue(true);
        }
    }
}


void CRelaxedLookupDlg::OnPubTextDClick( wxMouseEvent& event )
{
    if (!m_Sizer) {
        return;
    }
    // Find pub for clicked text, launch URL for pub
    wxStaticText* pubitem = (wxStaticText*)event.GetEventObject();
    if (!pubitem) {
        return;
    }

    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    vector<CRef<objects::CPubdesc> >::iterator pub_it = m_Pubs.begin();
    while (pub_it != m_Pubs.end() && !(*pub_it)) {
        ++pub_it;
    }

    objects::CPub::TPmid pmid(0);

    for (; node != m_Sizer->GetChildren().end() && pub_it != m_Pubs.end() && pmid == 0; ++node) {
        if ((*node)->IsSizer()) {
            wxStaticText *w = dynamic_cast<wxStaticText *>((*node)->GetSizer()->GetItem((size_t)1)->GetWindow());
            if (w && w == pubitem) {
                ITERATE(objects::CPubdesc::TPub::Tdata, pit, (*pub_it)->GetPub().Get()) {
                    if ((*pit)->IsPmid()) {
                        pmid.Set((*pit)->GetPmid().Get());
                        break;
                    }
                }
            }
            ++pub_it;
            while (pub_it != m_Pubs.end() && !(*pub_it)) {
                ++pub_it;
            }      
        }
    }
    if (pmid > 0) {
        string url = "https://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=Retrieve&db=PubMed&list_uids=" +
                     NStr::NumericToString(pmid.Get()) + "&dopt=Abstract";
        CAppPopup::PopupURL(url);
    }
}


void CRelaxedLookupDlg::Clear()
{
    m_MaxMatchTxt->SetValue(wxT("20"));
    m_UseFirstAuthorBtn->SetValue(false);
    m_FirstAuthorTxt->SetValue(wxEmptyString);
    m_UseLastAuthorBtn->SetValue(false);
    m_LastAuthorTxt->SetValue(wxEmptyString);
    m_UseJournalBtn->SetValue(false);
    m_JournalTxt->SetValue(wxEmptyString);
    m_UseYearBtn->SetValue(false);
    m_YearTxt->SetValue(wxEmptyString);
    m_UseVolumeBtn->SetValue(false);
    m_UseVolumeTxt->SetValue(wxEmptyString);
    m_UsePageBtn->SetValue(false);
    m_UsePageTxt->SetValue(wxEmptyString);
    m_UseExtraTermsBtn->SetValue(false);
    m_ExtraTermsTxt->SetValue(wxEmptyString);
}


string s_GetStringFromAuthor(const objects::CAuthor& auth)
{
    string val = "";

    if (auth.IsSetName() && auth.GetName().IsName()
        && auth.GetName().GetName().IsSetLast()) {
        val = auth.GetName().GetName().GetLast();
    }
    return val;
}


void CRelaxedLookupDlg::SetPubdesc( const objects::CPubdesc& pubdesc )
{
    Clear();
    m_Authors.clear();
    m_AuthorsExist.clear();
    if (pubdesc.IsSetPub()) {
        ITERATE(objects::CPubdesc::TPub::Tdata, pub_it, pubdesc.GetPub().Get()) {
            if ((*pub_it)->IsArticle()) {
                m_CitMatch.Reset(new objects::CPub);
                m_CitMatch->Assign(**pub_it);
                const objects::CCit_art& article = (*pub_it)->GetArticle();
                if (article.IsSetTitle()) {
                    try {
                        string title = article.GetTitle().GetTitle();
                        m_ArticleTitleTxt->SetValue(ToWxString(title));
                    } catch(CException&) {} catch (exception&) {}
                }
                if ((*pub_it)->IsSetAuthors() 
                    && (*pub_it)->GetAuthors().IsSetNames()
                    && (*pub_it)->GetAuthors().GetNames().IsStd()
                    && (*pub_it)->GetAuthors().GetNames().GetStd().size() > 0) {
                    const objects::CAuthor& first_auth = *((*pub_it)->GetAuthors().GetNames().GetStd().front());
                    string first = s_GetStringFromAuthor(first_auth);
                    if (!NStr::IsBlank(first)) {
                        m_UseFirstAuthorBtn->SetValue(true);
                        m_FirstAuthorTxt->SetValue(ToWxString(first));
                    }
                    const objects::CAuthor& last_auth = *((*pub_it)->GetAuthors().GetNames().GetStd().back());
                    string last = s_GetStringFromAuthor(last_auth);
                    if (!NStr::IsBlank(last)) {
                        m_UseLastAuthorBtn->SetValue(true);
                        m_LastAuthorTxt->SetValue(ToWxString(last));
                    }
                    for (const auto &author : (*pub_it)->GetAuthors().GetNames().GetStd())
                    {
                        string name = s_GetStringFromAuthor(*author);
                        if  (!NStr::IsBlank(name) && m_AuthorsExist.find(name) == m_AuthorsExist.end())
                        {
                            m_AuthorsExist.insert(name);
                            m_Authors.push_back(name);
                        }
                    }
                }
                if (article.IsSetFrom() && article.GetFrom().IsJournal()) {
                    const objects::CCit_jour& journal = article.GetFrom().GetJournal();
                    if (journal.IsSetTitle()) {
                        try {
                            string title = journal.GetTitle().GetTitle();
                            if (!NStr::IsBlank(title)) {
                                m_UseJournalBtn->SetValue(true);
                                m_JournalTxt->SetValue(ToWxString(title));
                            }
                        } catch(CException&) {} catch (exception&) {}
                    }
                    if (journal.IsSetImp()) {
                        const objects::CImprint& imp = journal.GetImp();
                        if (imp.IsSetDate() && imp.GetDate().IsStd() && imp.GetDate().GetStd().IsSetYear()) {
                            m_UseYearBtn->SetValue(true);
                            m_YearTxt->SetValue(ToWxString(NStr::NumericToString(imp.GetDate().GetStd().GetYear())));
                        }
                        if (imp.IsSetVolume() && !NStr::IsBlank(imp.GetVolume())) {
                            m_UseVolumeBtn->SetValue(true);
                            m_UseVolumeTxt->SetValue(ToWxString(imp.GetVolume()));
                        }
                        if (imp.IsSetPages() && !NStr::IsBlank(imp.GetPages())) {
                            m_UsePageBtn->SetValue(true);
                            m_UsePageTxt->SetValue(ToWxString(imp.GetPages()));
                        }    
                        // TODO issue!
                    }
                }
            }
        }
    }
}


void s_AddToTerms(string& terms, const string& key, const string& val)
{
    if (NStr::IsBlank(val)) {
        return;
    }
    if (!NStr::IsBlank(terms)) {
        terms += " AND ";
    }
    terms += val;

    if (!NStr::IsBlank(key)) {
        terms += "[" + key + "]";
    }
}


string CRelaxedLookupDlg::x_GetSearchTerms()
{
    string terms;
    //s_AddToTerms(terms, "TITLE", ToStdString(m_ArticleTitleTxt->GetValue()));

    bool use_all = false;
    if (m_NewQueryChoice->GetSelection() == 0) {
        use_all = true;
    }

    if (use_all || m_UseExtraTermsBtn->GetValue()) {
        string val = ToStdString(m_ExtraTermsTxt->GetValue());
        s_AddToTerms(terms, "", val);
    }

    string other_author = "";
   
    if (use_all || m_UseFirstAuthorBtn->GetValue()) {
            string val = ToStdString(m_FirstAuthorTxt->GetValue());
            s_AddToTerms(terms, "AUTH", val);
            other_author = val;
    }
    if (use_all || m_UseLastAuthorBtn->GetValue()) {
        string val = ToStdString(m_LastAuthorTxt->GetValue());
        if (!NStr::Equal(other_author, val)) {
            s_AddToTerms(terms, "AUTH", val);
        }        
    }

    if (use_all || m_UseJournalBtn->GetValue()) {
        string val = ToStdString(m_JournalTxt->GetValue());
        s_AddToTerms(terms, "JOUR", val);
    }

    if (use_all || m_UseVolumeBtn->GetValue()) {
        string val = ToStdString(m_UseVolumeTxt->GetValue());
        s_AddToTerms(terms, "VOLUME", val);
    }

    if (use_all || m_UsePageBtn->GetValue()) {
        string val = ToStdString(m_UsePageTxt->GetValue());
        s_AddToTerms(terms, "PAGE", val);
    }

    if (use_all || m_UseYearBtn->GetValue()) {
        string val = ToStdString(m_YearTxt->GetValue());
        if (!NStr::IsBlank(val)) {
            int year = 0;
            try {
                year = NStr::StringToInt(val);
                string year_text = "";
                if (m_YearFuzzBtn->GetValue()) {
                    year_text = "(" + NStr::NumericToString(year - 1) + "[EDAT] OR "
                        + NStr::NumericToString(year) + "[EDAT] OR "
                        + NStr::NumericToString(year + 1) + "[EDAT])";
                } else {
                    year_text = NStr::NumericToString(year) + "[EDAT]";
                }
                if (NStr::IsBlank(terms)) {
                    terms = year_text;
                } else {
                    terms += " AND " + year_text;
                }
            } catch(CException&) {} catch (exception&) {}
        }
    }

    return terms;
}

string CRelaxedLookupDlg::x_GetSearchTermsHydra()
{
    vector<string> terms;
    
    for (const auto &name : m_Authors)
        terms.push_back(name);

    if (!m_ArticleTitleTxt->GetValue().IsEmpty())
        terms.push_back(ToStdString(m_ArticleTitleTxt->GetValue()));
    if (!m_JournalTxt->GetValue().IsEmpty())
        terms.push_back(ToStdString(m_JournalTxt->GetValue()));
    if (!m_YearTxt->GetValue().IsEmpty())
        terms.push_back(ToStdString(m_YearTxt->GetValue()));
    if (!m_UseVolumeTxt->GetValue().IsEmpty())
        terms.push_back(ToStdString(m_UseVolumeTxt->GetValue()));
// issue?
    if (!m_UsePageTxt->GetValue().IsEmpty())
        terms.push_back(ToStdString(m_UsePageTxt->GetValue()));
    
    string str = NStr::Join(terms, " ");
    NStr::ReplaceInPlace(str, "&", " ");

    return str;
}


void CRelaxedLookupDlg::x_GetPubIdsHydra(const string& terms)
{
    // get Pub IDs
    m_Ids.clear();

    CHydraSearch parser;
    if (! parser.DoHydraSearch (terms, m_Ids) )
    {
        LOG_POST(Error << "CRelaxedLookupDlg::x_GetPubIdsHydra(): error executing search Entrez query: " << terms);
    }
    if (m_Ids.empty())
        x_GetPubIdsMla();
}

void CRelaxedLookupDlg::x_GetPubIdsMla()
{
    if (!m_CitMatch)
        return;
    
    CRef<objects::CMLAClient> mla(new objects::CMLAClient);
    objects::CMla_back reply;
    try
    {
        mla->AskCitmatch(*m_CitMatch, &reply);
        if (reply.IsCitmatch() && reply.GetCitmatch() > 0)
            m_Ids.push_back(reply.GetCitmatch());
    }
    catch(CException&) {} catch (exception&) {}   
}

void CRelaxedLookupDlg::x_GetPubIds(const string& terms)
{
    // get Pub IDs
    m_Ids.clear();

    int max = 20;
    try {
        max = NStr::StringToInt(ToStdString(m_MaxMatchTxt->GetValue()));
        if (max < 1 || max > 20) {
            max = 20;
        }
    } 
    catch(CException&) {} catch (exception&) {}


    string db = "PubMed";

    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(max);
    
    try {
        ecli.Search(db, terms, m_Ids);
    }
    catch (CException& e) {
        LOG_POST(Error << "CRelaxedLookupDlg::x_GetPubIds(): error executing search Entrez query: " << terms << ": " << e.GetMsg());
    }
}



void CRelaxedLookupDlg::SendQuery(bool use_hydra)
{
    Freeze();
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }

    int one_row_height = 0;
    int total_height = 0;
    int total_width = 0;
    int num_rows = 0;
    int max_rows_displayed = 6;

    if (use_hydra)
    {
        string terms = x_GetSearchTermsHydra();
        x_GetPubIdsHydra(terms);
    }
    else
    {
        string terms = x_GetSearchTerms();
        x_GetPubIds(terms);
    }
    m_Labels.clear();
    m_Pubs.clear();
    ITERATE(vector<int>, it, m_Ids) {
        CRef<objects::CPubdesc> pubdesc = GetPubdescFromEntrezById(*it);
        if (pubdesc) {
#if 1
            m_Labels.push_back(GetDocSumLabel(*pubdesc));
#else
            m_Labels.push_back(GetPubdescLabel(*pubdesc));            
#endif
        } else {
            m_Labels.push_back("");
        }
        m_Pubs.push_back(pubdesc);
    }
    bool first = true;
    if (m_Labels.size() == 0) {
        wxStaticText* label = new wxStaticText( m_ScrolledWindow, wxID_STATIC,
                                wxT("No match was found, hints for a modified query:\n Leave out one author\n Leave out year field (or increase it by 1) \n Leave out Volume field \n Modify the fields as you deem sensible"),
                        wxDefaultPosition, wxDefaultSize, 0 );
       label->Wrap(550);
       m_Sizer->Add(label, 0, wxALIGN_LEFT|wxALL, 0);
    } else {
        ITERATE(list<string>, it, m_Labels) {
            if (!NStr::IsBlank(*it)) {

                wxSizer* row = new wxBoxSizer(wxHORIZONTAL);
                m_Sizer->Add(row, 0, wxALIGN_LEFT|wxALL, 0);

                wxRadioButton* btn = new wxRadioButton( m_ScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
                row->Add(btn, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0); 
                if (first)
                {
                    btn->SetValue(true);
                    first = false;
                }
                wxStaticText* label = new wxStaticText( m_ScrolledWindow, wxID_STATIC, ToWxString(*it), wxDefaultPosition, wxDefaultSize, 0 );
                label->Wrap(550);
                row->Add(label, 0, wxALIGN_LEFT|wxALL, 0);
                label->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)(wxEventFunction)&CRelaxedLookupDlg::OnPubTextClick, NULL, this); 
                label->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)(wxEventFunction)&CRelaxedLookupDlg::OnPubTextDClick, NULL, this);

                int row_height, row_width;
                int btn_height, btn_width;
                int label_height, label_width;
                btn->GetClientSize(&btn_width, &btn_height);
                label->GetClientSize(&label_width, &label_height);
                row_height = btn_height;
                if (label_height > row_height) {
                    row_height = label_height;
                }
                row_width = btn_width + label_width;

                if (row_width > total_width) {
                    total_width = row_width;
                }
                if (num_rows < max_rows_displayed) {
                    total_height += row_height;
                }
                num_rows++;
                if (one_row_height == 0 || one_row_height > row_height) {
                    one_row_height = row_height;
                }
            }
        }        
    }


    m_ScrolledWindow->SetVirtualSize(total_width + 10, total_height);
    m_ScrolledWindow->SetScrollRate(0, one_row_height);

    m_ScrolledWindow->FitInside();
    Layout();
    Thaw();
}


CRef<objects::CPubdesc> CRelaxedLookupDlg::GetPubdesc()
{
    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    vector<CRef<objects::CPubdesc> >::iterator pub_it = m_Pubs.begin();
    while (pub_it != m_Pubs.end() && !(*pub_it)) {
        ++pub_it;
    }

    for (; node != m_Sizer->GetChildren().end() && pub_it != m_Pubs.end(); ++node) {
        if ((*node)->IsSizer()) {
            wxRadioButton *w = dynamic_cast<wxRadioButton *>((*node)->GetSizer()->GetItem((size_t)0)->GetWindow());
            if (w) {
                if (w->GetValue()) {
                    return *pub_it;
                } 
                ++pub_it;
                while (pub_it != m_Pubs.end() && !(*pub_it)) {
                    ++pub_it;
                }      
            }
        }
    }
    return CRef<objects::CPubdesc>(NULL);
}

void CRelaxedLookupDlg::SetStrict()
{
    m_MaxMatchTxt->SetValue(_("1"));
    m_YearFuzzBtn->SetValue(false);
}

END_NCBI_SCOPE

