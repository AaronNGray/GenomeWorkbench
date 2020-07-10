/*  $Id: journal_panel.cpp 44555 2020-01-21 19:24:06Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>

#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/CitRetract.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/pubmed/Pubmed_entry.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <gui/widgets/edit/isojta_lookup_dlg.hpp>
#include <gui/widgets/edit/relaxed_lookup_dlg.hpp>
#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/edit/pmid_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/doi_lookup.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include "journal_panel.hpp"
#include <gui/widgets/edit/publicationtype_panel.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/msgdlg.h>

#include <misc/pmcidconv_client/pmcidconv_client.hpp>

#include <wx/textdlg.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CJournalPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CJournalPanel, wxPanel )


/*
 * CJournalPanel event table definition
 */

BEGIN_EVENT_TABLE( CJournalPanel, wxPanel )

////@begin CJournalPanel event table entries
    EVT_TEXT( ID_JOURNAL_PMID, CJournalPanel::OnJournalPmidTextUpdated )
    EVT_CHOICE( ID_JOURNAL_ERRATUM, CJournalPanel::OnJournalErratumSelected )
    EVT_BUTTON( ID_JOURNAL_LOOKUP_RELAXED, CJournalPanel::OnJournalLookupRelaxedClick )
    EVT_BUTTON( ID_JOURNAL_LOOKUP_ISOJTA, CJournalPanel::OnJournalLookupIsojtaClick )
    EVT_BUTTON( ID_ARTICLE_LOOKUP, CJournalPanel::OnArticleLookup )
////@end CJournalPanel event table entries

END_EVENT_TABLE()


/*
 * CJournalPanel constructors
 */

 CJournalPanel::CJournalPanel() : m_Object(nullptr), m_pPmid(nullptr), m_pMuid(nullptr), m_pSerialNumber(nullptr)
{
    Init();
}

CJournalPanel::CJournalPanel(wxWindow* parent, CSerialObject& object, long * pmid, long * muid, long *serial_number,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
         : m_Object(nullptr), m_pPmid(pmid), m_pMuid(muid), m_pSerialNumber(serial_number)
{
    Init();
    m_Object = dynamic_cast<CCit_jour*>(&object);

    const CCit_jour& journal = dynamic_cast<const CCit_jour&>(*m_Object);
    m_EditedJournal.Reset((CSerialObject*)CCit_jour::GetTypeInfo()->Create());
    m_EditedJournal->Assign(journal);

    Create(parent, id, pos, size, style);
}


/*
 * CJournalPanel creator
 */

bool CJournalPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CJournalPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CJournalPanel creation
    return true;
}


/*
 * CJournalPanel destructor
 */

CJournalPanel::~CJournalPanel()
{
////@begin CJournalPanel destruction
////@end CJournalPanel destruction
}


/*
 * Member initialisation
 */

void CJournalPanel::Init()
{
////@begin CJournalPanel member initialisation
    m_JournalTitle = NULL;
    m_Volume = NULL;
    m_Issue = NULL;
    m_Pages = NULL;
    m_PmidCtrl = NULL;
    m_MuidCtrl = NULL;
    m_SerialNoCtrl = NULL;
    m_ErratumCtrl = NULL;
    m_ErratumExplanation = NULL;
    m_PubstatusCtrl = NULL;
    m_PubDateSizer = NULL;
////@end CJournalPanel member initialisation
    m_PubDatePanel = NULL;
    m_IsIsoJtaTitle = false;
}


/*
 * Control creation for CJournalPanel
 */

void CJournalPanel::CreateControls()
{    
////@begin CJournalPanel content construction
    CJournalPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Journal"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_JournalTitle = new wxTextCtrl( itemPanel1, ID_JOURNAL_TITLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_JournalTitle, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Volume"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Volume = new wxTextCtrl( itemPanel1, ID_JOURNAL_VOLUME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_Volume, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Issue"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Issue = new wxTextCtrl( itemPanel1, ID_JOURNAL_ISSUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_Issue, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Pages = new wxTextCtrl( itemPanel1, ID_JOURNAL_PAGES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_Pages, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("PubMedId"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PmidCtrl = new wxTextCtrl( itemPanel1, ID_JOURNAL_PMID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_PmidCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("muid"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MuidCtrl = new wxTextCtrl( itemPanel1, ID_JOURNAL_MUID, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_MuidCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Serial number"), wxDefaultPosition, wxDefaultSize,wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SerialNoCtrl = new wxTextCtrl( itemPanel1, ID_JOURNAL_SERIALNO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_SerialNoCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemPanel1, wxID_STATIC, _("Erratum"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ErratumCtrlStrings;
    m_ErratumCtrlStrings.Add(wxEmptyString);
    m_ErratumCtrlStrings.Add(_("Retracted"));
    m_ErratumCtrlStrings.Add(_("Notice"));
    m_ErratumCtrlStrings.Add(_("In-error"));
    m_ErratumCtrlStrings.Add(_("Erratum"));
    m_ErratumCtrl = new wxChoice( itemPanel1, ID_JOURNAL_ERRATUM, wxDefaultPosition, wxDefaultSize, m_ErratumCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_ErratumCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Explanation"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ErratumExplanation = new wxTextCtrl( itemPanel1, ID_JOURNAL_EXPLANATION, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_ErratumExplanation, 0, wxALIGN_LEFT|wxGROW|wxALL, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publication Status"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemFlexGridSizer3->Add(itemStaticText22, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_PubstatusCtrlStrings;
    m_PubstatusCtrlStrings.Add(_("Received"));
    m_PubstatusCtrlStrings.Add(_("Accepted"));
    m_PubstatusCtrlStrings.Add(_("Electronic Publication"));
    m_PubstatusCtrlStrings.Add(_("Print Publication"));
    m_PubstatusCtrlStrings.Add(_("Revised"));
    m_PubstatusCtrlStrings.Add(_("PMC Publication"));
    m_PubstatusCtrlStrings.Add(_("PMC Revision"));
    m_PubstatusCtrlStrings.Add(_("Entrez Date"));
    m_PubstatusCtrlStrings.Add(_("PubMed Revision"));
    m_PubstatusCtrlStrings.Add(_("Ahead of Print"));
    m_PubstatusCtrlStrings.Add(_("Pre-Medline (Obsolete)"));
    m_PubstatusCtrlStrings.Add(_("MeSH Date"));
    m_PubstatusCtrlStrings.Add(_("Other"));
    m_PubstatusCtrl = new wxChoice( itemPanel1, ID_JOURNAL_PUBSTATUS, wxDefaultPosition, wxDefaultSize, m_PubstatusCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_PubstatusCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer24, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText25 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publication Date"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PubDateSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer24->Add(m_PubDateSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer27, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton28 = new wxButton( itemPanel1, ID_ARTICLE_LOOKUP, _("Lookup Article"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer27->Add(itemButton28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton29 = new wxButton( itemPanel1, ID_JOURNAL_LOOKUP_RELAXED, _("Lookup Relaxed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer27->Add(itemButton29, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton30 = new wxButton( itemPanel1, ID_JOURNAL_LOOKUP_ISOJTA, _("Lookup ISOJTA"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer27->Add(itemButton30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CJournalPanel content construction

    CCit_jour& journal = dynamic_cast<CCit_jour&>(*m_EditedJournal);

    // add control for publication date
   
    CRef<objects::CDate> p_date(&(journal.SetImp().SetDate()));
//    wxWindow* pubdate = new CFlexibleDatePanel(itemPanel1, p_date);
    m_PubDatePanel = new CFlexibleDatePanel(itemPanel1, p_date);
    m_PubDateSizer->Add ( m_PubDatePanel );

    m_IsIsoJtaTitle = x_HasIsoJtaTitle(journal.GetTitle());
    x_UpdateJournalTitle();

    CSerialObject& imp_object = journal.SetImp();
    m_Volume->SetValidator( CSerialTextValidator(imp_object, "volume") );
    m_Issue->SetValidator( CSerialTextValidator(imp_object, "issue") );
    m_Pages->SetValidator( CSerialTextValidator(imp_object, "pages") );
    m_PmidCtrl->SetValidator( CPmidValidator(m_pPmid));
    m_MuidCtrl->SetValidator( CNumberOrBlankValidator(m_pMuid));
    m_SerialNoCtrl->SetValidator(CNumberOrBlankValidator(m_pSerialNumber));
    
}


void CJournalPanel::x_UpdateJournalTitle()
{
    CCit_jour& journal = dynamic_cast<CCit_jour&>(*m_EditedJournal);
    CSerialObject& title_object = journal.SetTitle();
    m_JournalTitle->SetValidator( CSerialTitleValidator(title_object, 
                            m_IsIsoJtaTitle ? CTitle::C_E::e_Iso_jta : CTitle::C_E::e_Name,
                            "Journal Title", false));
    m_JournalTitle->TransferDataToWindow();
}


bool CJournalPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    CCit_jour& journal = dynamic_cast<CCit_jour&>(*m_EditedJournal);
    if (journal.IsSetImp() && journal.GetImp().IsSetPubstatus()) {
        int pubstatus = journal.GetImp().GetPubstatus();
        switch (pubstatus) {
        case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                m_PubstatusCtrl->SetSelection (pubstatus - 1);
                break;
            case 255:
                m_PubstatusCtrl->SetSelection (13);
                break;
            default:
                m_PubstatusCtrl->SetSelection (14);
                break;
        }
    } else {
        m_PubstatusCtrl->SetSelection (14);
    }

    if (journal.IsSetImp() && journal.GetImp().IsSetRetract() && journal.GetImp().GetRetract().IsSetType()) {
        int erratum_type = journal.GetImp().GetRetract().GetType();
        switch (erratum_type) {
            case 1:
            case 2:
            case 3:
            case 4:
                m_ErratumCtrl->SetSelection (erratum_type);
                m_ErratumExplanation->Enable(true);
                if (journal.GetImp().GetRetract().IsSetExp()) {
                    m_ErratumExplanation->SetValue (ToWxString(journal.GetImp().GetRetract().GetExp()));
                } else {
                    m_ErratumExplanation->SetValue(wxEmptyString);
                }
                break;
            default:
                m_ErratumCtrl->SetSelection(0);
                m_ErratumExplanation->SetValue(wxEmptyString);
                m_ErratumExplanation->Enable(false);
                break;
        }
    } else {
        m_ErratumCtrl->SetSelection(0);
        m_ErratumExplanation->SetValue(wxEmptyString);
        m_ErratumExplanation->Enable(false);
    }

    m_IsIsoJtaTitle = x_HasIsoJtaTitle(journal.GetTitle());
    x_UpdateJournalTitle();

    m_PubDatePanel->TransferDataToWindow();

    return true;
}


bool CJournalPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CCit_jour& journal = dynamic_cast<CCit_jour&>(*m_EditedJournal);
    int pubstatus = m_PubstatusCtrl->GetCurrentSelection();
    if (pubstatus > -1 && pubstatus < 12) {
        journal.SetImp().SetPubstatus(pubstatus + 1);
    } else if (pubstatus == 12) {
        journal.SetImp().SetPubstatus(255);
    } else {
        journal.SetImp().ResetPubstatus();
    } 

    int erratum = m_ErratumCtrl->GetCurrentSelection();
    if (erratum > 0 && erratum < 5) {
        journal.SetImp().SetRetract().SetType((objects::CCitRetract_Base::TType)(erratum));
        string exp = ToStdString (m_ErratumExplanation->GetValue());
        if (!NStr::IsBlank(exp)) {
            journal.SetImp().SetRetract().SetExp(exp);
        } else {
            journal.SetImp().SetRetract().ResetExp();
        }
    } else {
        journal.SetImp().ResetRetract();
    }
   
    if (!m_JournalTitle->TransferDataFromWindow()) {
        return false;
    }
    if (!m_PubDatePanel->TransferDataFromWindow()) {
        return false;
    }

    return true;
}


CRef<CCit_jour> CJournalPanel::GetCit_jour() const
{
    const CCit_jour& journal = dynamic_cast<const CCit_jour&>(*m_EditedJournal);

    CRef<CCit_jour> ret(new CCit_jour());
    ret->Assign(journal);
    return ret;
}


/*
 * Should we show tooltips?
 */

bool CJournalPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CJournalPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CJournalPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CJournalPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CJournalPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CJournalPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CJournalPanel icon retrieval
}


/*
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_JOURNAL_ERRATUM
 */

void CJournalPanel::OnJournalErratumSelected( wxCommandEvent& event )
{
    int erratum = m_ErratumCtrl->GetSelection();

    if (erratum > -1 && erratum < 4) {
        m_ErratumExplanation->Enable(true);
    } else {
        m_ErratumExplanation->Enable(false);
    }
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_JOURNAL_LOOKUP_RELAXED
 */

void CJournalPanel::OnJournalLookupRelaxedClick( wxCommandEvent& event )
{
    wxWindow* parent = this->GetParent();
    CPublicationTypePanel* listpanel = dynamic_cast<CPublicationTypePanel*>(this->GetParent());
    while (parent && !listpanel) 
    {
        parent = parent->GetParent();
        listpanel = dynamic_cast<CPublicationTypePanel*>(parent);
    }
    CRef<objects::CPubdesc> pubdesc(NULL);
    if (listpanel) {
        listpanel->TransferDataFromWindow();
        pubdesc = listpanel->GetPubdesc();
        if (pubdesc) {
            CRelaxedLookupDlg dlg(NULL);
            dlg.SetPubdesc(*pubdesc);
            dlg.SendQuery();
            bool done = false;
            while ( !done && dlg.ShowModal() == wxID_OK ) {
                CRef<CPubdesc> new_pubdesc = dlg.GetPubdesc();
                if (new_pubdesc) {
                    wxWindow* parent = this->GetParent();
                    CPublicationTypePanel* listpanel = dynamic_cast<CPublicationTypePanel*>(this->GetParent());
                    while (parent && !listpanel) 
                    {
                        parent = parent->GetParent();
                        listpanel = dynamic_cast<CPublicationTypePanel*>(parent);
                    }
                    if (listpanel) {
                        listpanel->SetPub(*new_pubdesc);
                    }
                    done = true;
                } else {
                    wxMessageBox(wxT("Please choose a matching publication"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
                }
            }
        }        
    }
}

void CJournalPanel::OnArticleLookup( wxCommandEvent& event )
{
    wxWindow* parent = this->GetParent();
    CPublicationTypePanel* listpanel = dynamic_cast<CPublicationTypePanel*>(this->GetParent());
    while (parent && !listpanel) 
    {
        parent = parent->GetParent();
        listpanel = dynamic_cast<CPublicationTypePanel*>(parent);
    }
    CRef<objects::CPubdesc> pubdesc(NULL);
    if (listpanel) 
    {
        listpanel->TransferDataFromWindow();
        pubdesc = listpanel->GetPubdesc();
        if (pubdesc) 
        {
            CRelaxedLookupDlg dlg(NULL);
            dlg.SetPubdesc(*pubdesc);
            dlg.SetStrict();
            dlg.SendQuery(true);

            CRef<CPubdesc> new_pubdesc = dlg.GetPubdesc();
            if (new_pubdesc) 
            {
                wxWindow* parent = this->GetParent();
                CPublicationTypePanel* listpanel = dynamic_cast<CPublicationTypePanel*>(this->GetParent());
                while (parent && !listpanel) 
                {
                    parent = parent->GetParent();
                    listpanel = dynamic_cast<CPublicationTypePanel*>(parent);
                }
                if (listpanel) 
                {
                    listpanel->SetPub(*new_pubdesc);                   
                }
            } else 
            {
                wxMessageBox(wxT("Publication lookup failed"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
            }            
        }        
    }
}



/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_JOURNAL_LOOKUP_ISOJTA
 */

void CJournalPanel::OnJournalLookupIsojtaClick( wxCommandEvent& event )
{
    string old_title = ToStdString(m_JournalTitle->GetValue());
    if (NStr::IsBlank(old_title)) {
        wxMessageBox(wxT("You must provide a journal name to look up!"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return;
    }
    vector<string> titles;
    CDoiLookup::LookupIsojta(old_title, titles);

    string new_title;
    if (titles.empty()) {
        wxMessageBox(wxT("No matches found!"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return;
    } else if (titles.size() == 1) {
        new_title = titles[0];
    } else {
        CISOJTALookupDlg dlg(this, titles);
        if ( dlg.ShowModal() == wxID_OK ) {
            new_title = dlg.GetISOJTA();
        }
    }
    size_t pos = NStr::Find(new_title, "||");
    if (pos != string::npos) {
        new_title = new_title.substr(0, pos);
    }
    if (!NStr::IsBlank(new_title)) {
        m_IsIsoJtaTitle = true;
        x_UpdateJournalTitle();
        m_JournalTitle->SetValue(ToWxString(new_title));
    }
}

/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_JOURNAL_PMID
 */

void CJournalPanel::OnJournalPmidTextUpdated( wxCommandEvent& event )
{
    string pmid = ToStdString(m_PmidCtrl->GetValue());
    if (NStr::StartsWith(pmid, " ") || NStr::EndsWith(pmid, " ")) {
        NStr::TruncateSpacesInPlace(pmid);
        m_PmidCtrl->SetValue(ToWxString(pmid));
    }
}


bool CJournalPanel::x_HasIsoJtaTitle( const CTitle& title)
{
    if (!title.IsSet()) {
        return false;
    }
    ITERATE(CTitle::Tdata, it, title.Get()) {
        if ((*it)->IsIso_jta()) {
            return true;
        }
    }
    return false;
}


void CJournalPanel::SetFinalValidation(bool val)
{
    if (m_JournalTitle) {
        CCit_jour& journal = dynamic_cast<CCit_jour&>(*m_EditedJournal);
        CSerialObject& title_object = journal.SetTitle();
        m_JournalTitle->SetValidator( CSerialTitleValidator(title_object, 
                                m_IsIsoJtaTitle ? CTitle::C_E::e_Iso_jta : CTitle::C_E::e_Name,
                                "Journal Title", val));
    }
}



END_NCBI_SCOPE
