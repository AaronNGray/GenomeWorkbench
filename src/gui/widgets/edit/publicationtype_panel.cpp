/*  $Id: publicationtype_panel.cpp 44591 2020-01-28 17:06:06Z filippov $
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
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/Meeting.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/biblio/ArticleIdSet.hpp>
#include <objects/biblio/ArticleId.hpp>
#include <objects/biblio/PubMedId.hpp>
#include <objects/biblio/biblio_macros.hpp>
#include <gui/objutils/doi_lookup.hpp>
#include <misc/pmcidconv_client/pmcidconv_client.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>

#include <gui/widgets/edit/author_affiliation_panel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>
#include <gui/widgets/edit/consortium_panel.hpp>
#include "journal_panel.hpp"
#include "book_panel.hpp"
#include "bookchapter_panel.hpp"
#include "citsub_panel.hpp"
#include "locale_panel.hpp"
#include "onlinepublication_panel.hpp"
#include "patent_panel.hpp"
#include "proceedings_panel.hpp"
#include "proceedingschapter_panel.hpp"
#include "publisher_panel.hpp"
#include "remarksandserialnumber_panel.hpp"
#include "titlepanel.hpp"
#include <gui/widgets/edit/publicationtype_panel.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CPublicationTypePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPublicationTypePanel, wxPanel )


/*
 * CPublicationTypePanel event table definition
 */

BEGIN_EVENT_TABLE( CPublicationTypePanel, wxPanel )

////@begin CPublicationTypePanel event table entries
    EVT_RADIOBOX( ID_UNPUB_INPRESS_PUB, CPublicationTypePanel::OnUnpubInpressPubSelected )
    EVT_RADIOBOX( ID_PUBCLASS, CPublicationTypePanel::OnPubclassSelected )
    EVT_BUTTON( ID_DOI_LOOKUP, CPublicationTypePanel::OnDoiLookup )
////@end CPublicationTypePanel event table entries

END_EVENT_TABLE()


/*
 * CPublicationTypePanel constructors
 */

CPublicationTypePanel::CPublicationTypePanel()
{
    Init();
}

CPublicationTypePanel::CPublicationTypePanel( wxWindow* parent, CRef<objects::CPubdesc> pubdesc, 
    bool create_mode, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_SerialNumber(0), m_CreateMode(create_mode), m_CurrentStatus(0), m_CurrentType(ePubTypeJournalArticle), m_Pmid(0), m_Muid(0), m_PublisherPanel(0)
{
    Init();
    m_Pubdesc = new objects::CPubdesc();
    if (pubdesc) {
        m_Pubdesc->Assign(*pubdesc);
    }
    Create(parent, id, pos, size, style);
}


/*
 * CPublicationTypePanel creator
 */

bool CPublicationTypePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPublicationTypePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPublicationTypePanel creation
    return true;
}


/*
 * CPublicationTypePanel destructor
 */

CPublicationTypePanel::~CPublicationTypePanel()
{
////@begin CPublicationTypePanel destruction
////@end CPublicationTypePanel destruction
}


/*
 * Member initialisation
 */

void CPublicationTypePanel::Init()
{
////@begin CPublicationTypePanel member initialisation
    m_Status = NULL;
    m_PubClass = NULL;
    m_PubdescDetails = NULL;
////@end CPublicationTypePanel member initialisation
    m_PublisherPanel = NULL;
    m_JournalPanel = NULL;
    m_PatentPanel = NULL;
    m_CitSubPanel = NULL;
    m_Unpublished = NULL;
    m_PmidCtrl = NULL;
    m_LookupButton = NULL;
}


/*
 * Control creation for CPublicationTypePanel
 */

void CPublicationTypePanel::CreateControls()
{    
////@begin CPublicationTypePanel content construction
    CPublicationTypePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_TOP|wxALL, 5);

    wxArrayString m_StatusStrings;
    m_StatusStrings.Add(_("&Unpublished"));
    m_StatusStrings.Add(_("&In Press"));
    m_StatusStrings.Add(_("&Published"));
    m_Status = new wxRadioBox( itemPanel1, ID_UNPUB_INPRESS_PUB, _("Status"), wxDefaultPosition, wxDefaultSize, m_StatusStrings, 1, wxRA_SPECIFY_COLS );
    m_Status->SetSelection(0);
    itemBoxSizer3->Add(m_Status, 0, wxGROW|wxALL, 5);

    wxArrayString m_PubClassStrings;
    m_PubClassStrings.Add(_("&Journal"));
    m_PubClassStrings.Add(_("Book &Chapter"));
    m_PubClassStrings.Add(_("&Book"));
    m_PubClassStrings.Add(_("&Thesis/Monograph"));
    m_PubClassStrings.Add(_("&Proceedings Chapter"));
    m_PubClassStrings.Add(_("&Proceedings"));
    m_PubClassStrings.Add(_("&Patent"));
    m_PubClassStrings.Add(_("&Submission"));
    m_PubClass = new wxRadioBox( itemPanel1, ID_PUBCLASS, _("Class"), wxDefaultPosition, wxDefaultSize, m_PubClassStrings, 1, wxRA_SPECIFY_COLS );
    m_PubClass->SetSelection(0);
    itemBoxSizer3->Add(m_PubClass, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer6, 1, wxGROW|wxALL, 0);

    m_PubdescDetails = new wxNotebook( itemPanel1, ID_TREEBOOK, wxDefaultPosition, wxSize(700, 630), wxBK_DEFAULT );

    itemBoxSizer6->Add(m_PubdescDetails, 1, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer6->Add(500, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);    

////@end CPublicationTypePanel content construction
    wxWindow* panel;
    panel = new CRemarksAndSerialNumberPanel (m_PubdescDetails, *m_Pubdesc);
    m_PubdescDetails->AddPage (panel, wxT("Remarks"));

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer6->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_PmidCtrl = new wxTextCtrl( itemPanel1, ID_DOI_PMID, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer7->Add(m_PmidCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LookupButton = new wxButton( itemPanel1, ID_DOI_LOOKUP, _("Lookup DOI/PMID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_LookupButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);   
}

static bool s_IsOnlySetSerialNumber(const CCit_gen& gen)
{
    if (gen.IsSetSerial_number()) {
        if (!gen.IsSetCit()
            && !gen.IsSetAuthors()
            && !gen.IsSetMuid()
            && !gen.IsSetJournal()
            && !gen.IsSetVolume()
            && !gen.IsSetIssue()
            && !gen.IsSetPages()
            && !gen.IsSetDate()
            && !gen.IsSetTitle()
            && !gen.IsSetPmid()) {
            return true;
        }
    }
    return false;
}


bool CPublicationTypePanel::TransferDataToWindow()
{
    CRef<CPub> main_pub;

    NON_CONST_ITERATE (CPub_equiv::Tdata, pub_it, m_Pubdesc->SetPub().Set()) {
        switch ((*pub_it)->Which()) {
            case CPub::e_Muid:
                m_Muid = (*pub_it)->GetMuid();
                break;
            case CPub::e_Pmid:
                m_Pmid = (*pub_it)->GetPmid();
                break;
            case CPub::e_Gen:
                {
                    if ((*pub_it)->GetGen().IsSetSerial_number()) {
                        m_SerialNumber = (*pub_it)->GetGen().GetSerial_number();
                        if (s_IsOnlySetSerialNumber((*pub_it)->GetGen())) {
                            break;
                        }
                    }
                }
                // note - deliberately falling through to default case
            default:
                if (!main_pub) {
                    main_pub = *pub_it;
                }
                break;
        }
    }
    
    if (!main_pub) {
        m_CurrentType = ePubTypeJournalArticle;
        if (m_Pmid > 0 || m_Muid > 0) {
            m_CurrentStatus = 2;
        } else {
            m_CurrentStatus = 0;
        }
            
        main_pub = x_GetMainPub();
    }
    Freeze();
    x_CreatePubControls (main_pub);
    wxWindow* panel;
    panel = new CRemarksAndSerialNumberPanel (m_PubdescDetails, *m_Pubdesc);
    m_PubdescDetails->AddPage (panel, wxT("Remarks"));

    if (m_Pmid > 0)
        m_PmidCtrl->SetValue(wxString::Format(wxT("%ld"), m_Pmid));
    else   {       
        if (m_Pubdesc->IsSetPub() && m_Pubdesc->GetPub().IsSet()) {
            ITERATE(CPubdesc::TPub::Tdata, it, m_Pubdesc->GetPub().Get()) {
                if ((*it)->IsArticle()) {
                    if ((*it)->GetArticle().IsSetIds()) {
                        ITERATE(CArticleIdSet::Tdata, id, (*it)->GetArticle().GetIds().Get()) {
                            if ((*id)->IsDoi()) {
                                m_PmidCtrl->SetValue(wxString((*id)->GetDoi()));
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    x_SetStatusAndClass (main_pub);
    if (m_CurrentType == ePubTypeSub) {
        m_PubClass->Enable(false);
        m_PubdescDetails->SetSelection(2);
    } else if (!m_CreateMode) {
        m_PubClass->Enable(ePubTypeSub, false);
    }
  
    m_PubdescDetails->TransferDataToWindow();
    for (size_t i = 0; i < m_PubdescDetails->GetPageCount(); ++i) {
        m_PubdescDetails->GetPage(i)->TransferDataToWindow();
    }
    Thaw();
    Refresh();
 
    return wxPanel::TransferDataToWindow();
}


void CPublicationTypePanel::x_SetStatus (const CImprint& imprint)
{
    if (imprint.IsSetPrepub() && imprint.GetPrepub() == CImprint::ePrepub_in_press) {
        m_CurrentStatus = 1;
    } 
    else if (imprint.IsSetPubstatus() && imprint.GetPubstatus() == ePubStatus_aheadofprint) {
        m_CurrentStatus = 1;
    } 
    else
    {
        m_CurrentStatus = 2;
    }
}


void CPublicationTypePanel::x_SetStatusAndClass (CRef<CPub> pub)
{
    if (!pub) {
        m_CurrentStatus = 0;
        m_CurrentType = ePubTypeJournalArticle;
    } else {
        switch (pub->Which()) {
            case CPub::e_Gen:
                m_CurrentStatus = 0;
                break;
            case CPub::e_Article:
                {
                    objects::CCit_art& art = pub->SetArticle();
                    if (art.IsSetFrom()) 
                    {
                        switch (art.GetFrom().Which()) 
                        {
                            case objects::CCit_art::C_From::e_Journal:
                                x_SetStatus (art.SetFrom().SetJournal().SetImp());
                                m_CurrentType = ePubTypeJournalArticle;
                                break;
                            case objects::CCit_art::C_From::e_Book:
                                x_SetStatus (art.SetFrom().SetBook().SetImp());
                                m_CurrentType = ePubTypeBookChapter;
                                break;
                            case objects::CCit_art::C_From::e_Proc:
                                x_SetStatus (art.SetFrom().SetProc().SetBook().SetImp());
                                m_CurrentType = ePubTypeProcChapter;
                                break;
                            default:
                                m_CurrentStatus = 0;
                                m_CurrentType = ePubTypeJournalArticle;
                                break;
                        }
                    } else {
                        m_CurrentStatus = 0;
                        m_CurrentType = ePubTypeJournalArticle;
                    }
                }
                break;
            case CPub::e_Book:
                x_SetStatus (pub->SetBook().SetImp());
                m_CurrentType = ePubTypeBook;
                break;
            case CPub::e_Proc:
                m_CurrentStatus = 2;
                m_CurrentType = ePubTypeProc;
                break;
            case CPub::e_Sub:
                m_CurrentType = ePubTypeSub;
                break;
            case CPub::e_Patent:
                m_CurrentStatus = 2;
                m_CurrentType = ePubTypePatent;
                break;
            case CPub::e_Man:
                x_SetStatus (pub->SetMan().SetCit().SetImp());
                m_CurrentType = ePubTypeThesis;
                break;
            case CPub::e_Pmid:
            case CPub::e_Muid:
                m_CurrentStatus = 2;
                m_CurrentType = ePubTypeJournalArticle;
                break;
            default:
                m_CurrentStatus = 0;
                m_CurrentType = ePubTypeJournalArticle;
                break;
        }
    }

    if (m_CurrentType == ePubTypeSub) {
        m_PubClass->SetSelection(ePubTypeSub);
        m_Status->Enable(false);
        m_PmidCtrl->Hide();
        m_LookupButton->Hide();
    } else {  
        m_Status->SetSelection(m_CurrentStatus);
        m_Status->Enable(true);
        m_PmidCtrl->Show();
        m_LookupButton->Show();
        if (m_CurrentStatus == 0) {
            m_Status->Enable(true);
            m_PubClass->Enable(false);
        } else {
            m_PubClass->Enable(true);
            m_PubClass->SetSelection(m_CurrentType);
            if (m_CurrentType == ePubTypePatent || m_CurrentType == ePubTypeProc || m_CurrentType == ePubTypeProcChapter) {
                m_Status->Enable(1, false);
            } else {
                m_Status->Enable(1, true);
            }
            if (!m_CreateMode) {
                m_PubClass->Enable(ePubTypeSub, false);
            }
        }
    }
}


static bool s_IsJournalArticle (CRef<CPub> pub)
{
    if (!pub) {
        return false;
    }
    if (pub->IsArticle() && pub->GetArticle().IsSetFrom() && pub->GetArticle().GetFrom().IsJournal()) {
        return true;
    } else {
        return false;
    }
}


static bool s_IsProcChapter (CRef<CPub> pub)
{
    if (!pub) {
        return false;
    }
    if (pub->IsArticle() && pub->GetArticle().IsSetFrom() && pub->GetArticle().GetFrom().IsProc()) {
        return true;
    } else {
        return false;
    }
}


static bool s_IsBookChapter (CRef<CPub> pub)
{
    if (!pub) {
        return false;
    }
    if (pub->IsArticle() && pub->GetArticle().IsSetFrom() && pub->GetArticle().GetFrom().IsBook()) {
        return true;
    } else {
        return false;
    }
}


CRef<CPub> CPublicationTypePanel::x_PutMainPubFirst(int pub_type, int status)
{
    if (!m_Pubdesc->IsSetPub() || m_Pubdesc->GetPub().Get().size() == 0) {
        return CRef<CPub>(NULL);
    }

    bool found = false;

    CPub_equiv::Tdata::iterator pub_it = m_Pubdesc->SetPub().Set().begin();
    while (pub_it != m_Pubdesc->SetPub().Set().end() && !found) {
         if (pub_type == ePubTypeSub) {
            // looking for citsub
            if ((*pub_it)->IsSub()) {
                found = true;
            }
        } else if (status == 0) {
            // looking for unpublished pub
            if ((*pub_it)->IsGen()) {
                found = true;
            }
        } else {
            switch (pub_type) {
                case ePubTypeJournalArticle:
                    if (s_IsJournalArticle(*pub_it)) {
                        found = true;
                    }
                    break;
                case ePubTypeBookChapter:
                    if (s_IsBookChapter(*pub_it)) {
                        found = true;
                    }
                    break;
                case ePubTypeProcChapter:
                    if (s_IsProcChapter(*pub_it)) {
                        found = true;
                    }
                    break;
                case ePubTypeBook:
                    if ((*pub_it)->IsBook()) {
                        found = true;
                    }
                    break;
                case ePubTypeThesis:
                    if ((*pub_it)->IsMan()) {
                        found = true;
                    }
                    break;
                case ePubTypeProc:
                    if ((*pub_it)->IsProc()) {
                        found = true;
                    }
                    break;
                case ePubTypePatent:
                    if ((*pub_it)->IsPatent()) {
                        found = true;
                    }
                    break;
                default:
                    break;
            }
        }
        if (!found) {
            pub_it++;
        }
    }
    if (found) {
        if (pub_it != m_Pubdesc->SetPub().Set().begin()) {
            CRef<CPub> tmp(new CPub());
            tmp->Assign(**pub_it);
            m_Pubdesc->SetPub().Set().erase(pub_it);
            m_Pubdesc->SetPub().Set().insert(m_Pubdesc->SetPub().Set().begin(), tmp);
            return tmp;
        } else {
            return *pub_it;
        }
    } else {
        return CRef<CPub>(NULL);
    }
}


CRef<objects::CPub> CPublicationTypePanel::x_GetMainPub(void)
{
    CRef<objects::CPub> main_pub;

    NON_CONST_ITERATE (CPub_equiv::Tdata, pub_it, m_Pubdesc->SetPub().Set()) {
        if (m_CurrentType == ePubTypeSub) {
            // looking for citsub
            if ((*pub_it)->IsSub()) {
                main_pub = *pub_it;
                break;
            }
        } else if (m_CurrentStatus == 0) {
            // looking for unpublished pub
            if ((*pub_it)->IsGen()) {
                main_pub = *pub_it;
                break;
            }
        } else {
            switch (m_CurrentType) {
                case ePubTypeJournalArticle:
                    if (s_IsJournalArticle(*pub_it)) {
                        main_pub = *pub_it;
                    }
                    break;
                case ePubTypeBookChapter:
                    if (s_IsBookChapter(*pub_it)) {
                        main_pub = *pub_it;
                    }
                    break;
                case ePubTypeProcChapter:
                    if (s_IsProcChapter(*pub_it)) {
                        main_pub = *pub_it;
                    }
                    break;
                case ePubTypeBook:
                    if ((*pub_it)->IsBook()) {
                        main_pub = *pub_it;
                    }
                    break;
                case ePubTypeThesis:
                    if ((*pub_it)->IsMan()) {
                        main_pub = *pub_it;
                    }
                    break;
                case ePubTypeProc:
                    if ((*pub_it)->IsProc()) {
                        main_pub = *pub_it;
                    }
                    break;
                case ePubTypePatent:
                    if ((*pub_it)->IsPatent()) {
                        main_pub = *pub_it;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    if (!main_pub) {
        // TODO might be secondary identifier
    }

    if (!main_pub) {
        // create new pub to hold data
        CRef<objects::CPub> new_pub(new CPub());
        if (m_CurrentType == ePubTypeSub) {
            new_pub->SetSub();
        } else if (m_CurrentStatus == 0) {            
            new_pub->SetGen().SetCit("unpublished");
        } else {
            switch (m_CurrentType) {
                case ePubTypeJournalArticle:
                    new_pub->SetArticle().SetFrom().SetJournal();
                    break;
                case ePubTypeBookChapter:
                    new_pub->SetArticle().SetFrom().SetBook();
                    break;
                case ePubTypeBook:
                    new_pub->SetBook();
                    break;
                case ePubTypeThesis:
                    new_pub->SetMan();
                    break;
                case ePubTypeProcChapter:
                    new_pub->SetArticle().SetFrom().SetProc();
                    break;
                case ePubTypeProc:
                    new_pub->SetProc();
                    break;
                case ePubTypePatent:
                    new_pub->SetPatent();
                    break;
                default:
                    new_pub->SetGen().SetCit("unpublished");
                    break;
            }
            main_pub = new_pub;
        }  
        m_Pubdesc->SetPub().Set().insert(m_Pubdesc->SetPub().Set().begin(), new_pub);
        main_pub = new_pub;
    }

    main_pub = x_PutMainPubFirst(m_CurrentType, m_CurrentStatus);
    return main_pub;
}


void CPublicationTypePanel::x_CreatePubControls(CRef<CPub> pub)
{ 
    m_PubdescDetails->DeleteAllPages();
    m_PublisherPanel = NULL;
    m_JournalPanel = NULL;
    m_PatentPanel = NULL;
    m_CitSubPanel = NULL;
    m_Unpublished = NULL;
    switch (pub->Which()) {
    case CPub::e_Gen:
        x_CreateUnpubControls(pub->SetGen());
        break;
    case CPub::e_Article:
        if (s_IsJournalArticle(pub)) {
            x_CreateJournalArticleControls(pub->SetArticle());
        } else if (s_IsBookChapter (pub)) {
            x_CreateBookChapterControls(pub->SetArticle());
        } else if (s_IsProcChapter (pub)) {
            x_CreateProcChapterControls(pub->SetArticle());
        } else {
            x_CreateJournalArticleControls(pub->SetArticle());
        }
        break;
    case CPub::e_Book:
        x_CreateBookControls(pub->SetBook(), false);
        break;
    case CPub::e_Proc:
        x_CreateProcControls(pub->SetProc());
        break;
    case CPub::e_Sub:
        x_CreateSubControls(pub->SetSub());
        break;
    case CPub::e_Patent:
        x_CreatePatentControls(pub->SetPatent());
        break;
    case CPub::e_Man:
        x_CreateThesisControls(pub->SetMan());
        break;
    default:
        x_CreateUnpubControls (pub->SetGen());
        break;
    }    
}


void CPublicationTypePanel::x_CreateUnpubControls(objects::CCit_gen& cit_gen)
{
    wxWindow* panel;
    m_Unpublished = new CUnpublishedPanel(m_PubdescDetails, cit_gen.IsSetTitle() ? cit_gen.GetTitle() : kEmptyStr);
    m_PubdescDetails->AddPage (m_Unpublished, wxT("Title"));

    CAuth_list& auth_list = cit_gen.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Author Names"));

    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Author Affiliation"));
}


void CPublicationTypePanel::x_CreateJournalArticleControls(objects::CCit_art& art)
{
    wxWindow* panel;
    CCit_jour& journal = art.SetFrom().SetJournal();

    if (!art.IsSetTitle() || art.GetTitle().Get().empty()) {
        CRef<objects::CTitle::C_E> blank_title(new objects::CTitle::C_E());
        blank_title->SetName("?");
        art.SetTitle().Set().push_back(blank_title);
    }
    if (art.IsSetIds())
    {
        FOR_EACH_ARTICLEID_ON_CITART(id, art)
        {
            if ((*id)->IsPubmed() && m_Pmid <= 0)
                m_Pmid = (*id)->GetPubmed();
        }
    }
    // panel for article title
    panel = new CTitlePanel (m_PubdescDetails, art.SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Article Title"));
    m_JournalPanel = new CJournalPanel(m_PubdescDetails, journal, &m_Pmid, &m_Muid, (long*)&m_SerialNumber);
    m_PubdescDetails->AddPage (m_JournalPanel, wxT("Journal"));

    CAuth_list& auth_list = art.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Author Names"));

    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Author Affiliation"));
}


void CPublicationTypePanel::x_CreateBookChapterControls(objects::CCit_art& art)

{
    wxWindow* panel;
    
    CCit_book& book = art.SetFrom().SetBook();

    if (!art.IsSetTitle() || art.GetTitle().Get().empty()) {
        CRef<objects::CTitle::C_E> blank_title(new objects::CTitle::C_E());
        blank_title->SetName("?");
        art.SetTitle().Set().push_back(blank_title);
    }
    // panel for chapter title
    panel = new CTitlePanel (m_PubdescDetails, art.SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Chapter Title"));

    if (!book.IsSetTitle() || book.GetTitle().Get().empty()) {
        CRef<objects::CTitle::C_E> blank_title(new objects::CTitle::C_E());
        blank_title->SetName("?");
        book.SetTitle().Set().push_back(blank_title);
    }

    CAuth_list& auth_list = art.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Chapter Authors"));

    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Chapter Author Affiliation"));

    x_CreateBookControls (art.SetFrom().SetBook(), true);
}


void CPublicationTypePanel::x_CreateProcChapterControls(objects::CCit_art& art)
{
    wxWindow* panel;

    if (!art.IsSetTitle() || art.GetTitle().Get().empty()) {
        CRef<objects::CTitle::C_E> blank_title(new objects::CTitle::C_E());
        blank_title->SetName("?");
        art.SetTitle().Set().push_back(blank_title);
    }
    panel = new CTitlePanel (m_PubdescDetails, art.SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Abstract Title"));

    CCit_proc& proc = art.SetFrom().SetProc();
    CCit_book& book = proc.SetBook();
    if (!book.IsSetTitle() || book.GetTitle().Get().empty()) {
        CRef<objects::CTitle::C_E> blank_title(new objects::CTitle::C_E());
        blank_title->SetName("?");
        book.SetTitle().Set().push_back(blank_title);
    }
    panel = new CTitlePanel (m_PubdescDetails, book.SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Proceedings Title"));

    CAuth_list& auth_list = art.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Presenters"));

    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Presenter Affiliation"));

    CAuth_list& editors = book.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, editors, true);
    m_PubdescDetails->AddPage (panel, wxT("Chairpersons"));

    panel = new CAuthorAffiliationPanel(m_PubdescDetails, editors.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Chairperson Affiliation"));

    panel = new CLocalePanel (m_PubdescDetails, proc.SetMeet());
    m_PubdescDetails->AddPage (panel, wxT("Locale"));

    m_PublisherPanel = new CPublisherPanel (m_PubdescDetails, book.SetImp(), true);
    m_PubdescDetails->AddPage (m_PublisherPanel, wxT("Publisher"));
}


void CPublicationTypePanel::x_CreateProcControls(objects::CCit_proc& proc)
{
    wxWindow* panel;

    CRef<CCit_book> book(&(proc.SetBook()));
    panel = new CTitlePanel (m_PubdescDetails, book->SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Proceedings Title"));

    CAuth_list& auth_list = book->SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Conveners"));
    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Convener Affiliation"));
    panel = new CLocalePanel (m_PubdescDetails, proc.SetMeet());
    m_PubdescDetails->AddPage (panel, wxT("Locale"));
    m_PublisherPanel = new CPublisherPanel (m_PubdescDetails, book->SetImp());
    m_PubdescDetails->AddPage (m_PublisherPanel, wxT("Publisher"));
}


void CPublicationTypePanel::x_CreateBookControls(objects::CCit_book& book, bool is_chapter)
{
    wxWindow* panel;

    if (!book.IsSetTitle() || book.GetTitle().Get().empty()) {
        CRef<objects::CTitle::C_E> blank_title(new objects::CTitle::C_E());
        blank_title->SetName("?");
        book.SetTitle().Set().push_back(blank_title);
    }
    // panel for book title
    panel = new CTitlePanel (m_PubdescDetails, book.SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Book Title"));

    CAuth_list& auth_list = book.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, is_chapter ? wxT("Editor Names") : wxT("Author Names"));

    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, is_chapter ? wxT("Editor Affiliation") : wxT("Author Affiliation"));
    m_PublisherPanel = new CPublisherPanel (m_PubdescDetails, book.SetImp(), is_chapter);
    m_PubdescDetails->AddPage (m_PublisherPanel, wxT("Publisher"));

}


void CPublicationTypePanel::x_CreateSubControls(objects::CCit_sub& sub)
{
    wxWindow* panel;

    m_CitSubPanel = new CCitSubPanel (m_PubdescDetails, sub);
    m_PubdescDetails->AddPage (m_CitSubPanel, wxT("CitSub"));                
    CAuth_list& auth_list = sub.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Authors"));
    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Author Affiliation"));
}


void CPublicationTypePanel::x_CreatePatentControls(objects::CCit_pat &pat)
{
    wxWindow* panel;
    m_PatentPanel = new CPatentPanel (m_PubdescDetails, pat);
    m_PubdescDetails->AddPage (m_PatentPanel, wxT("Patent"));
    CAuth_list& auth_list = pat.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Authors"));
    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Author Affiliation"));
    panel = new CAuthorNamesPanel(m_PubdescDetails, pat.SetApplicants(), true);
    m_PubdescDetails->AddPage (panel, wxT("Applicants"));
    panel = new CAuthorAffiliationPanel(m_PubdescDetails, pat.SetApplicants().SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Applicant Affiliation"));
    panel = new CAuthorNamesPanel(m_PubdescDetails, pat.SetAssignees(), true);
    m_PubdescDetails->AddPage (panel, wxT("Assignees"));
    panel = new CAuthorAffiliationPanel(m_PubdescDetails, pat.SetAssignees().SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Assignee Affiliation"));
}


void CPublicationTypePanel::x_CreateThesisControls(objects::CCit_let &let)
{
    wxWindow* panel;
    CCit_book& book = let.SetCit();
    panel = new CTitlePanel (m_PubdescDetails, book.SetTitle().Set().front());
    m_PubdescDetails->AddPage (panel, wxT("Title"));

    CAuth_list& auth_list = book.SetAuthors();
    panel = new CAuthorNamesPanel(m_PubdescDetails, auth_list, true);
    m_PubdescDetails->AddPage (panel, wxT("Author Names"));
    panel = new CAuthorAffiliationPanel(m_PubdescDetails, auth_list.SetAffil());
    m_PubdescDetails->AddPage (panel, wxT("Author Affiliation"));
    m_PublisherPanel = new CPublisherPanel (m_PubdescDetails, book.SetImp());
    m_PubdescDetails->AddPage (m_PublisherPanel, wxT("Publisher"));
}


static CRef<objects::CImprint> s_GetPubImprint(CRef<CPub> pub);
static void s_SetPubImprint(CRef<CPub> pub, CRef<objects::CImprint>);

void s_ApplyStatusToPub(int status, CRef<CPub> pub)
{
    // apply status
    if (status != 0) {
        CRef<CImprint> imprint = s_GetPubImprint (pub);
        if (imprint) {
            if (status == 1) {
                if (!imprint->IsSetPrepub() || imprint->GetPrepub() != CImprint::ePrepub_in_press) {
                    imprint->SetPrepub(CImprint::ePrepub_in_press);
                    s_SetPubImprint (pub, imprint);
                }
            } else {
                if (imprint->IsSetPrepub()) {
                    imprint->ResetPrepub();
                    s_SetPubImprint (pub, imprint);
                }
            }
        }
    }
}

void CPublicationTypePanel::SetFinalValidation()
{
  if (m_JournalPanel) 
  {
      m_JournalPanel->SetFinalValidation(true);
  }
}

bool CPublicationTypePanel::TransferDataFromWindow()
{
  
    if (!wxPanel::TransferDataFromWindow())
        return false;

    if (!x_CollectDetails()) {
        return false;
    }

    // remove "placeholder" pubs for storing unused information
    if (!m_Pubdesc->IsSetPub() 
        || !m_Pubdesc->GetPub().IsSet() 
        || m_Pubdesc->GetPub().Get().empty()) {
        return false;
    }

    CPub_equiv::Tdata::iterator pub_it = m_Pubdesc->SetPub().Set().begin();
    while (pub_it != m_Pubdesc->SetPub().Set().end()) {
        if (m_CurrentType == ePubTypeSub) {
            // only keep the Cit-sub pubs
            if (!(*pub_it)->IsSub()) {
                pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
            } else {
                if (m_Unpublished)
                {
                    string title = m_Unpublished->GetTitle();
                    if (title.empty())
                        (*pub_it)->SetSub().ResetDescr();
                    else
                        (*pub_it)->SetSub().SetDescr(title);
                }
                ++pub_it;
            }
        } else if (m_CurrentStatus == 0) {
            // only keep unpublished pub
            if (!(*pub_it)->IsGen()) {
                pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
            } else {
                if (m_Unpublished)
                {
                    string title = m_Unpublished->GetTitle();
                    if (title.empty())
                        (*pub_it)->SetGen().ResetTitle();
                    else
                        (*pub_it)->SetGen().SetTitle(title);
                }
                ++pub_it;
            }
        } else {
            switch (m_CurrentType) {
                case ePubTypeJournalArticle:
                    if (!s_IsJournalArticle(*pub_it)) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        if ((*pub_it)->SetArticle().IsSetIds())
                        {
                            EDIT_EACH_ARTICLEID_ON_CITART(id, (*pub_it)->SetArticle())
                            {
                                if ((*id)->IsPubmed())
                                {
                                    if (m_Pmid <= 0)
                                    {
                                        ERASE_ARTICLEID_ON_CITART(id, (*pub_it)->SetArticle());
                                    }
                                    else
                                    {
                                        (*id)->SetPubmed(CPubMedId(m_Pmid));
                                    }
                                }
                            }
                            if ((*pub_it)->GetArticle().GetIds().Get().empty())
                                (*pub_it)->SetArticle().ResetIds();
                        }
                        ++pub_it;
                    }
                    break;
                case ePubTypeBookChapter:
                    if (!s_IsBookChapter(*pub_it)) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        ++pub_it;
                    }
                    break;
                case ePubTypeProcChapter:
                    if (!s_IsProcChapter(*pub_it)) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        ++pub_it;
                    }
                    break;
                case ePubTypeBook:
                    if (!(*pub_it)->IsBook()) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        ++pub_it;
                    }
                    break;
                case ePubTypeThesis:
                    if (!(*pub_it)->IsMan()) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        ++pub_it;
                    }
                    break;
                case ePubTypeProc:
                    if (!(*pub_it)->IsProc()) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        ++pub_it;
                    }
                    break;
                case ePubTypePatent:
                    if (!(*pub_it)->IsPatent()) {
                        pub_it = m_Pubdesc->SetPub().Set().erase(pub_it);
                    } else {
                        ++pub_it;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // add pmid and muid pubs
    if (m_CurrentStatus != 0 && m_CurrentType == ePubTypeJournalArticle) {
        if (m_Pmid > 0) {
            CRef<CPub> new_pub(new CPub());
            new_pub->SetPmid().Set(m_Pmid);
            m_Pubdesc->SetPub().Set().push_back(new_pub);
        }
        if (m_Muid > 0) {
            CRef<CPub> new_pub(new CPub());
            new_pub->SetMuid(m_Muid);
            m_Pubdesc->SetPub().Set().push_back(new_pub);
        }
        if (m_SerialNumber > 0) {
            CRef<CPub> new_pub(new CPub);
            new_pub->SetGen().SetSerial_number(m_SerialNumber);
            m_Pubdesc->SetPub().Set().push_back(new_pub);
        }
    }

    // apply status
    if (m_CurrentStatus != 0) {
        CRef<CPub> pub = x_GetMainPub();
        s_ApplyStatusToPub(m_CurrentStatus, pub);
    }

    return true;
}


CRef<objects::CPubdesc> CPublicationTypePanel::GetPubdesc() const
{
    if (m_Pubdesc)
        return m_Pubdesc;

    return CRef<objects::CPubdesc>();
}


/*
 * Should we show tooltips?
 */

bool CPublicationTypePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPublicationTypePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPublicationTypePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPublicationTypePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CPublicationTypePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPublicationTypePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPublicationTypePanel icon retrieval
}


static string s_GetPubTitle(CRef<CPub> pub) 
{
    string title = "";
    if (!pub) {
        return title;
    }

    switch (pub->Which()) {
        case CPub::e_Article:
            if (pub->GetArticle().IsSetTitle()) {
                title = pub->GetArticle().GetTitle().GetTitle();
            }
            break;
        case CPub::e_Book:
            if (pub->GetBook().IsSetTitle()) {
                title = pub->GetBook().GetTitle().GetTitle();
            }
            break;
        case CPub::e_Gen:
            if (pub->GetGen().IsSetTitle()) {
                title = pub->GetGen().GetTitle();
            }
            break;
        case CPub::e_Journal:
            if (pub->GetJournal().IsSetTitle()) {
                title = pub->GetJournal().GetTitle().GetTitle();
            }
            break;
        case CPub::e_Man:
            if (pub->GetMan().IsSetCit() && pub->GetMan().GetCit().IsSetTitle()) {
                title = pub->GetMan().GetCit().GetTitle().GetTitle();
            }
            break;
        case CPub::e_Patent:
            if (pub->GetPatent().IsSetTitle()) {
                title = pub->GetPatent().GetTitle();
            }
            break;
        case CPub::e_Proc:
            if (pub->GetProc().IsSetBook() && pub->GetProc().GetBook().IsSetTitle()) {
                title = pub->GetProc().GetBook().GetTitle().GetTitle();
            }
            break;
        case CPub::e_Sub:
            if (pub->GetSub().IsSetDescr()) {
                title = pub->GetSub().GetDescr();
            }
            break;
        default:
            break;
    }
    return title;
}


static void s_SetPubTitle(CRef<CPub> pub, string title) 
{
    if (!pub) {
        return;
    } else if (pub->IsGen()) {
        pub->SetGen().SetTitle(title);
        return;
    } else if (pub->IsPatent()) {
        pub->SetPatent().SetTitle(title);
        return;
    } else if (pub->IsSub()) {
        pub->SetSub().SetDescr(title);
        return;
    }

    CRef<objects::CTitle::C_E> otitle(new objects::CTitle::C_E());

    switch (pub->Which()) {
        case CPub::e_Article:
            if (!pub->GetArticle().IsSetTitle() || pub->GetArticle().GetTitle().Get().empty()) {
                pub->SetArticle().SetTitle().Set().push_back(otitle);
            } else {
                otitle = pub->SetArticle().SetTitle().Set().front();
            }
            break;
        case CPub::e_Book:
            if (!pub->GetBook().IsSetTitle() || pub->GetBook().GetTitle().Get().empty()) {
                pub->SetBook().SetTitle().Set().push_back(otitle);
            } else {
                otitle = pub->SetBook().SetTitle().Set().front();
            }
            break;
        case CPub::e_Man:
            if (!pub->GetMan().IsSetCit() || !pub->GetMan().GetCit().IsSetTitle() || pub->GetMan().GetCit().GetTitle().Get().empty()) {
                pub->SetMan().SetCit().SetTitle().Set().push_back(otitle);
            } else {
                otitle = pub->SetMan().SetCit().SetTitle().Set().front();
            }
            break;
        case CPub::e_Proc:
            if (!pub->GetProc().IsSetBook() || !pub->GetProc().GetBook().IsSetTitle() || pub->GetProc().GetBook().GetTitle().Get().empty()) {
                pub->SetProc().SetBook().SetTitle().Set().push_back(otitle);
            } else {
                otitle = pub->SetProc().SetBook().SetTitle().Set().front();
            }
            break;
        default:
            return;
            break;
    }
    otitle->SetName(title);
}


static void s_SetPubAuthors (CRef<CPub> pub, const CAuth_list& auth_list) 
{
    if (!pub) {
        return;
    }

    switch (pub->Which()) {
        case CPub::e_Article:
            pub->SetArticle().SetAuthors().Assign(auth_list);
            break;
        case CPub::e_Book:
            pub->SetBook().SetAuthors().Assign(auth_list);
            break;
        case CPub::e_Gen:
            pub->SetGen().SetAuthors().Assign(auth_list);
            break;
        case CPub::e_Man:
            pub->SetMan().SetCit().SetAuthors().Assign(auth_list);
            break;
        case CPub::e_Patent:
            pub->SetPatent().SetAuthors().Assign(auth_list);
            break;
        case CPub::e_Proc:
            pub->SetProc().SetBook().SetAuthors().Assign(auth_list);
            break;
        case CPub::e_Sub:
            pub->SetSub().SetAuthors().Assign(auth_list);
            break;
        default:
            break;
    }
}


static CRef<objects::CImprint> s_GetPubImprint(CRef<CPub> pub)
{
    CRef<objects::CImprint> imprint(new CImprint());

    try {
        switch (pub->Which()) {
        case CPub::e_Article:
            if (pub->GetArticle().IsSetFrom()) {
                switch (pub->GetArticle().GetFrom().Which()) {
                case objects::CCit_art::C_From::e_Journal:                
                    imprint->Assign(pub->GetArticle().GetFrom().GetJournal().GetImp());
                    break;
                case objects::CCit_art::C_From::e_Book:
                    imprint->Assign(pub->GetArticle().GetFrom().GetBook().GetImp());
                    break;
                case objects::CCit_art::C_From::e_Proc:
                    imprint->Assign(pub->GetArticle().GetFrom().GetProc().GetBook().GetImp());
                    break;
                default:
                    imprint.Reset();
                    break;
                }
            } else {
                imprint.Reset();
            }
            break;
        case CPub::e_Book:
            imprint->Assign(pub->GetBook().GetImp());
            break;
        case CPub::e_Man:
            imprint->Assign(pub->GetMan().GetCit().GetImp());
            break;
        case CPub::e_Proc:
            imprint->Assign(pub->GetProc().GetBook().GetImp());
            break;
        default:
            imprint.Reset();
            break;
        }
    } catch (const CException& e) {
        LOG_POST(Error << "In s_GetPubImprint(): " << e.GetMsg());
        imprint.Reset();
    }
    catch (const exception& e) {
        LOG_POST(Error << "In s_GetPubImprint(): " << e.what());
        imprint.Reset();
    }
    return imprint;
}


static void s_SetPubImprint(CRef<CPub> pub, CRef<objects::CImprint> imprint)
{
    if (!imprint) {
        return;
    }
    if (!imprint->IsSetDate() || !imprint->GetDate().IsStd()) {
        imprint->SetDate().SetStr("?");
    }
    switch (pub->Which()) {
    case CPub::e_Article:
        if (pub->GetArticle().IsSetFrom()) {
            switch (pub->GetArticle().GetFrom().Which()) {
            case objects::CCit_art::C_From::e_Journal: 
                pub->SetArticle().SetFrom().SetJournal().SetImp(*imprint);
                break;
            case objects::CCit_art::C_From::e_Book:
                pub->SetArticle().SetFrom().SetBook().SetImp(*imprint);
                break;
            case objects::CCit_art::C_From::e_Proc:
                pub->SetArticle().SetFrom().SetProc().SetBook().SetImp(*imprint);
                break;
            default:
                break;
            }
        }
        break;
    case CPub::e_Book:
        pub->SetBook().SetImp(*imprint);
        break;
    case CPub::e_Man:
        pub->SetMan().SetCit().SetImp(*imprint);
        break;
    case CPub::e_Proc:
        pub->SetProc().SetBook().SetImp(*imprint);
        break;
    default:
        break;
    }
}


static void s_SetPubLocale (CRef<CPub> pub, CRef<CMeeting> locale)
{
    if (!pub) {
        return;
    }
    if (s_IsProcChapter(pub)) {
        if (locale) {
            pub->SetArticle().SetFrom().SetProc().SetMeet(*locale);
        } else {
            pub->SetArticle().SetFrom().SetProc().ResetMeet();
        }
    } else if (pub->IsProc()) {
        if (locale) {
            pub->SetProc().SetMeet(*locale);
        } else {
            pub->SetProc().ResetMeet();
        }
    }
}


static void s_TransferPubData (CRef<CPub> src_pub, CRef<CPub> dest_pub)
{
    if (!src_pub || !dest_pub) {
        return;
    }

    // special cases
    // proc to procchapter
    if (src_pub->IsProc() && s_IsProcChapter(dest_pub)) {
        CRef<CCit_proc> proc(new CCit_proc());
        proc->Assign(src_pub->GetProc());
        dest_pub->SetArticle().SetFrom().SetProc(*proc);
    } else if (dest_pub->IsProc() && s_IsProcChapter(src_pub)) {
        CRef<CCit_proc> proc(new CCit_proc());
        proc->Assign(src_pub->GetArticle().GetFrom().GetProc());
        dest_pub->SetProc(*proc);
    }

    // book to bookchapter
    if (src_pub->IsBook() && s_IsBookChapter(dest_pub)) {
        CRef<CCit_book> book(new CCit_book());
        book->Assign(src_pub->GetBook());
        dest_pub->SetArticle().SetFrom().SetBook(*book);
    } else if (dest_pub->IsBook() && s_IsBookChapter(src_pub)) {
        CRef<CCit_book> book(new CCit_book());
        book->Assign(src_pub->GetArticle().GetFrom().GetBook());
        dest_pub->SetBook(*book);
    }

    // default - only transfer authors, title, imprint
    if (src_pub->IsSetAuthors()) {
        CRef<CAuth_list> authors(new CAuth_list());
        authors->Assign(src_pub->GetAuthors());
        s_SetPubAuthors(dest_pub, *authors);
    }
    string title = s_GetPubTitle(src_pub);
    if (!NStr::IsBlank(title)) {
        s_SetPubTitle(dest_pub, title);
    }

    CRef<objects::CImprint> imprint = s_GetPubImprint(src_pub);
    if (imprint) {
        s_SetPubImprint(dest_pub, imprint);
    }
    if (dest_pub->IsMan() && !dest_pub->GetMan().IsSetType())
    {
        dest_pub->SetMan().SetType( CCit_let::eType_thesis );
    }
}


bool CPublicationTypePanel::x_CollectAuthors()
{
    bool rval = true;
    CRef<objects::CPub> pub = x_GetMainPub();

    // get authors and affiliations
    int author_pos = 0, affil_pos = 0;
    for (size_t i = 0; i < m_PubdescDetails->GetPageCount(); ++i) {
        CAuthorNamesPanel* auth_panel;
        auth_panel = dynamic_cast<CAuthorNamesPanel*>(m_PubdescDetails->GetPage(i));
        if (auth_panel) {
            rval &= auth_panel->TransferDataFromWindow();
            if (author_pos == 0) {
                CRef<CAuth_list> authors(new CAuth_list());
                authors->Assign(pub->GetAuthors());
                auth_panel->PopulateAuthors(*authors);
                s_SetPubAuthors(pub, *authors);
            } else {
                // some pubs have additional authors
                if (s_IsProcChapter(pub)) {
                    // these are the chairpersons
                    CAuth_list& authors = pub->SetArticle().SetFrom().SetProc().SetBook().SetAuthors();
                    auth_panel->PopulateAuthors(authors);
                } else if (s_IsBookChapter(pub)) {
                    // these are the book authors
                    CAuth_list& authors = pub->SetArticle().SetFrom().SetBook().SetAuthors();
                    auth_panel->PopulateAuthors(authors);
                } else if (pub->IsPatent()) {
                    if (author_pos == 1) {
                        // these are the applicants
                        CAuth_list& authors = pub->SetPatent().SetApplicants();
                        auth_panel->PopulateAuthors(authors);
                    } else {
                        // these are the assignees
                        CAuth_list& authors = pub->SetPatent().SetAssignees();
                        auth_panel->PopulateAuthors(authors);
                    }
                }
            }
            author_pos++;
        } else {
            CAuthorAffiliationPanel *affil_panel = dynamic_cast<CAuthorAffiliationPanel*>(m_PubdescDetails->GetPage(i));
            if (affil_panel) {
                rval &= affil_panel->TransferDataFromWindow();
                CRef<CAffil> affil = affil_panel->GetAffil();
                if (affil_pos == 0) {
                    CRef<CAuth_list> authors(new CAuth_list());
                    authors->Assign(pub->GetAuthors());
                    if (affil) {
                        authors->SetAffil(*affil);
                    } else {
                        authors->ResetAffil();
                    }
                    s_SetPubAuthors(pub, *authors);
                } else {
                    // some pubs have additional authors
                    if (s_IsProcChapter(pub)) {
                        // these are the chairpersons
                        CAuth_list& authors = pub->SetArticle().SetFrom().SetProc().SetBook().SetAuthors();
                        if (affil) {
                            authors.SetAffil(*affil);
                        } else {
                            authors.ResetAffil();
                        }
                    } else if (s_IsBookChapter(pub)) {
                        // these are the book authors
                        CAuth_list& authors = pub->SetArticle().SetFrom().SetBook().SetAuthors();
                        if (affil) {
                            authors.SetAffil(*affil);
                        } else {
                            authors.ResetAffil();
                        }
                    } else if (pub->IsPatent()) {
                        if (affil_pos == 1) {
                            // these are the applicants
                            CAuth_list& authors = pub->SetPatent().SetApplicants();
                            if (affil) {
                                authors.SetAffil(*affil);
                            } else {
                                authors.ResetAffil();
                            }
                        } else {
                            // these are the assignees
                            CAuth_list& authors = pub->SetPatent().SetAssignees();
                            if (affil) {
                                authors.SetAffil(*affil);
                            } else {
                                authors.ResetAffil();
                            }
                        }
                    }
                }
                affil_pos++;
            }
        }   
    }
    return rval;
}


bool CPublicationTypePanel::x_CollectDetails(void)
{
    if (!m_PubdescDetails->TransferDataFromWindow()) {
        return false;
    }
    for (size_t i = 0; i < m_PubdescDetails->GetPageCount(); ++i) {
        if (!m_PubdescDetails->GetPage(i)->TransferDataFromWindow()) {
            return false;
        }
    }

    CRef<objects::CPub> pub = x_GetMainPub();

    // get Journal info
    if (m_JournalPanel) {
        if (!m_JournalPanel->TransferDataFromWindow()) {
            return false;
        }
        // copy to src_pub
        CRef<CCit_jour> journal = m_JournalPanel->GetCit_jour();
        pub->SetArticle().SetFrom().SetJournal(*journal);
    }

    // patent info
    if (m_PatentPanel) {
        if (!m_PatentPanel->TransferDataFromWindow()) {
            return false;
        }
        // copy to pub
        CRef<CCit_pat> patent = m_PatentPanel->GetCit_pat();
        pub->SetPatent(*patent);
    }

    // citsub info
    if (m_CitSubPanel) {
        if (!m_CitSubPanel->TransferDataFromWindow()) {
            return false;
        }
        // copy to pub
        CRef<CCit_sub> sub = m_CitSubPanel->GetCit_sub();
        pub->SetSub(*sub);
    }

    // locale, for proc and proc chapter
    for (size_t i = 0; i < m_PubdescDetails->GetPageCount(); ++i) {
        CLocalePanel *locale_panel = dynamic_cast<CLocalePanel*>(m_PubdescDetails->GetPage(i));
        if (locale_panel) {
            if (!locale_panel->TransferDataFromWindow()) {
                return false;
            }
            CRef<CMeeting> locale = locale_panel->GetMeeting();
            s_SetPubLocale (pub, locale);
        }
    }
    
    // get publisher info
    if (m_PublisherPanel) {
        if (!m_PublisherPanel->TransferDataFromWindow()) {
            return false;
        }
        // copy to src_pub
        CRef<CImprint> imprint = s_GetPubImprint (pub);
        if (imprint) {
            m_PublisherPanel->PopulatePublisherFields (*imprint);
            s_SetPubImprint (pub, imprint);
        }        
    }

    if (m_Unpublished)
    {
        string title = m_Unpublished->GetTitle();
        if (title.empty())
        {
            if (pub->IsGen())
                pub->SetGen().ResetTitle();
            if (pub->IsSub())
                pub->SetSub().ResetDescr();
        }
        else
        {
            if (pub->IsGen())
                pub->SetGen().SetTitle(title);
            if (pub->IsSub())
                pub->SetSub().SetDescr(title);
        }
    }

    // note - collect authors last, otherwise data will be overridden from earlier input
    return x_CollectAuthors();
}


/*
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_UNPUB_INPRESS_PUB
 */

void CPublicationTypePanel::OnUnpubInpressPubSelected( wxCommandEvent& event )
{
    // if changing from in-press or published to unpub (or vice-versa),
    // need to change format of data, panel to represent
    int new_status = m_Status->GetSelection();
    if (new_status == m_CurrentStatus) 
    {
        // no change
        return;
    } 

	// find the pub currently holding information
    x_CollectDetails();
    CRef<objects::CPub> prev_pub = x_GetMainPub();

    m_CurrentStatus = new_status;

    // find/create destination pub
    CRef<CPub> dest_pub = x_GetMainPub();
    s_ApplyStatusToPub(new_status, dest_pub);
    if (prev_pub == dest_pub) {
        return;
    }

    s_TransferPubData (prev_pub, dest_pub);

    TransferDataToWindow();
}


/*
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_PUBCLASS
 */

void CPublicationTypePanel::OnPubclassSelected( wxCommandEvent& event )
{
    // find the pub currently holding information
    x_CollectDetails();
    CRef<objects::CPub> prev_pub = x_GetMainPub();
    int new_pub_type = m_PubClass->GetSelection();
    // find/create destination pub
    m_CurrentType = new_pub_type;
    CRef<CPub> dest_pub = x_GetMainPub();    
    // transfer data from old to new pub
    s_TransferPubData (prev_pub, dest_pub);
    TransferDataToWindow();
}


void CPublicationTypePanel::SetPub(const CPubdesc& pubdesc)
{
    m_Pubdesc.Reset(new CPubdesc);
    m_Pubdesc->Assign(pubdesc);
    TransferDataToWindow();
    Refresh();
    m_PubdescDetails->ChangeSelection(1);
}


void CPublicationTypePanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    SetPub(desc.GetPub());
    TransferDataToWindow();
    if (m_JournalPanel) {
        m_JournalPanel->SetFinalValidation(true);
        TransferDataToWindow();
        m_JournalPanel->SetFinalValidation(false);
    }
}


void CPublicationTypePanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    if (m_JournalPanel) {
        m_JournalPanel->SetFinalValidation(true);
    }

    if (TransferDataFromWindow()) {
        CRef<objects::CPubdesc> pub = GetPubdesc();
        desc.SetPub(*pub);
    } else {
        desc.Reset();
    }
    if (m_JournalPanel) {
        m_JournalPanel->SetFinalValidation(false);
    }
}

bool CPublicationTypePanel::IsImportEnabled()
{
    if (m_PubdescDetails)
    {
        CImportExportAccess* window = dynamic_cast<CImportExportAccess*>(m_PubdescDetails->GetCurrentPage());
        if (window)
            return window->IsImportEnabled();
    }
    return true;
}

bool CPublicationTypePanel::IsExportEnabled()
{
    if (m_PubdescDetails)
    {
        CImportExportAccess* window = dynamic_cast<CImportExportAccess*>(m_PubdescDetails->GetCurrentPage());
        if (window)
            return window->IsExportEnabled();
    }
    return true;
}

CRef<CSerialObject> CPublicationTypePanel::OnExport()
{
    if (m_PubdescDetails)
    {
        CImportExportAccess* window = dynamic_cast<CImportExportAccess*>(m_PubdescDetails->GetCurrentPage());
        if (window)
            return window->OnExport();
    }

    if (!TransferDataFromWindow())
        return CRef<CSerialObject>(NULL);

    CRef<CSerialObject> so((CSerialObject*)(new objects::CPubdesc));
    so->Assign(*m_Pubdesc);
    return so;
}

void CPublicationTypePanel::OnImport( CNcbiIfstream &istr)
{
    if (m_PubdescDetails)
    {
        CImportExportAccess* window = dynamic_cast<CImportExportAccess*>(m_PubdescDetails->GetCurrentPage());
        if (window)
        {
            window->OnImport(istr);
            return;
        }
    }

    CRef<objects::CPubdesc> pub(new objects::CPubdesc);
    istr >> MSerial_AsnText >> *pub;
    SetPub(*pub);
}

void CPublicationTypePanel::OnDoiLookup( wxCommandEvent& event )
{
    if (!m_PmidCtrl)
	return;
    string doi = m_PmidCtrl->GetValue().ToStdString();
    NStr::TruncateSpacesInPlace(doi);
    int pmid = NStr::StringToInt(doi, NStr::fConvErr_NoThrow);
    if (NStr::StartsWith(doi, "PMC"))
    {
        CPMCIDSearch converter;
        CPMCIDSearch::TResults results;
        vector<string> ids;
        ids.push_back(doi);
        if (converter.DoPMCIDSearch(ids, results)) 
            pmid = results[0];
        else
            wxMessageBox(wxT("PMC ID not found"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }

    CRef<CPubdesc> pubdesc;
    if (pmid > 0)
    {
        pubdesc = GetPubdescFromEntrezById(pmid);
        if (!pubdesc)
            wxMessageBox(wxT("No article found"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }
    else
    {
        pair<CRef<CPubdesc>, string> pubdesc_str = CDoiLookup::GetPubFromCrossRef(doi);
        pubdesc = pubdesc_str.first;
        if (!pubdesc) 
            wxMessageBox(wxT("Problem resolving DOI \n") + wxString(pubdesc_str.second), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }

    if (pubdesc)
        SetPub(*pubdesc);
}

END_NCBI_SCOPE

