/*  $Id: remarksandserialnumber_panel.cpp 41364 2018-07-13 18:19:04Z filippov $
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
#include <objects/biblio/ArticleId.hpp>
#include <objects/biblio/ArticleIdSet.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>

#include <misc/pmcidconv_client/pmcidconv_client.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/msgdlg.h> 

#include <gui/widgets/edit/publicationtype_panel.hpp>
#include "remarksandserialnumber_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CRemarksAndSerialNumberPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRemarksAndSerialNumberPanel, wxPanel )


/*
 * CRemarksAndSerialNumberPanel event table definition
 */

BEGIN_EVENT_TABLE( CRemarksAndSerialNumberPanel, wxPanel )

////@begin CRemarksAndSerialNumberPanel event table entries
//    EVT_BUTTON( ID_BUTTON, CRemarksAndSerialNumberPanel::OnButtonClick )

////@end CRemarksAndSerialNumberPanel event table entries

END_EVENT_TABLE()


/*
 * CRemarksAndSerialNumberPanel constructors
 */

CRemarksAndSerialNumberPanel::CRemarksAndSerialNumberPanel()
{
    Init();
}

CRemarksAndSerialNumberPanel::CRemarksAndSerialNumberPanel( wxWindow* parent, CSerialObject& object,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CPubdesc*>(&object);
    Create(parent, id, pos, size, style);
}


/*
 * CRemarksAndSerialNumberPanel creator
 */

bool CRemarksAndSerialNumberPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRemarksAndSerialNumberPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRemarksAndSerialNumberPanel creation
    return true;
}


/*
 * CRemarksAndSerialNumberPanel destructor
 */

CRemarksAndSerialNumberPanel::~CRemarksAndSerialNumberPanel()
{
////@begin CRemarksAndSerialNumberPanel destruction
////@end CRemarksAndSerialNumberPanel destruction
}


/*
 * Member initialisation
 */

void CRemarksAndSerialNumberPanel::Init()
{
////@begin CRemarksAndSerialNumberPanel member initialisation
    m_Comment = NULL;
    m_DOISizer = NULL;
    m_DOI = NULL;
////@end CRemarksAndSerialNumberPanel member initialisation
}


/*
 * Control creation for CRemarksAndSerialNumberPanel
 */

void CRemarksAndSerialNumberPanel::CreateControls()
{
////@begin CRemarksAndSerialNumberPanel content construction
    CRemarksAndSerialNumberPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Remark"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemPanel1, ID_REMARK, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE );
    itemBoxSizer2->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_DOISizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_DOISizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("DOI"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DOISizer->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DOI = new wxTextCtrl( itemPanel1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_DOISizer->Add(m_DOI, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

//    wxButton* itemButton8 = new wxButton( itemPanel1, ID_BUTTON, _("Lookup DOI"), wxDefaultPosition, wxDefaultSize, 0 );
//    m_DOISizer->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer2->Add(400, 350, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CRemarksAndSerialNumberPanel content construction
}


bool CRemarksAndSerialNumberPanel::TransferDataToWindow()
{
    CPubdesc* pubdesc = dynamic_cast<CPubdesc*>(m_Object);
    string comment = kEmptyStr;
    string doi = kEmptyStr;
    bool show_doi = false;
    if (pubdesc) {
        if (pubdesc->IsSetComment()) {
            comment = pubdesc->GetComment();
        }
        if (pubdesc->IsSetPub() && pubdesc->GetPub().IsSet()) {
            ITERATE(CPubdesc::TPub::Tdata, it, pubdesc->GetPub().Get()) {
                if ((*it)->IsArticle()) {
                    show_doi = true;
                    if ((*it)->GetArticle().IsSetIds()) {
                        ITERATE(CArticleIdSet::Tdata, id, (*it)->GetArticle().GetIds().Get()) {
                            if ((*id)->IsDoi()) {
                                doi = (*id)->GetDoi();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    m_Comment->SetValue(ToWxString(comment));
    m_DOI->SetValue(ToWxString(doi));
    m_DOISizer->Show(show_doi);
    return true;
}


bool CRemarksAndSerialNumberPanel::TransferDataFromWindow()
{
    CPubdesc* pubdesc = dynamic_cast<CPubdesc*>(m_Object);
    if (pubdesc) {
        string comment = ToStdString(m_Comment->GetValue());
        if (NStr::IsBlank(comment)) {
            pubdesc->ResetComment();
        } else {
            pubdesc->SetComment(comment);
        }

        bool found_storage = false;
        string doi = ToStdString(m_DOI->GetValue());
        if (pubdesc->IsSetPub() && pubdesc->GetPub().IsSet()) {
            NON_CONST_ITERATE(CPubdesc::TPub::Tdata, it, pubdesc->SetPub().Set()) {
                if ((*it)->IsArticle()) {
                    if ((*it)->GetArticle().IsSetIds()) {
                        NON_CONST_ITERATE(CArticleIdSet::Tdata, id, (*it)->SetArticle().SetIds().Set()) {
                            if ((*id)->IsDoi()) {
                                if (NStr::IsBlank(doi)) {
                                    (*it)->SetArticle().SetIds().Set().erase(id);
                                    if ((*it)->GetArticle().GetIds().Get().empty()) {
                                        (*it)->SetArticle().ResetIds();
                                    }
                                } else {
                                    (*id)->SetDoi().Set(doi);
                                }
                                found_storage = true;
                                break;
                            }
                        }
                    }
                    if (!found_storage && !NStr::IsBlank(doi)) {
                        CRef<CArticleId> art_id(new CArticleId());
                        art_id->SetDoi().Set(doi);
                        (*it)->SetArticle().SetIds().Set().push_back(art_id);
                    }
                    break;
                }
            }
        }
        return true;
    } else {
        return false;
    }
}


/*
 * Should we show tooltips?
 */

bool CRemarksAndSerialNumberPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CRemarksAndSerialNumberPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRemarksAndSerialNumberPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRemarksAndSerialNumberPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CRemarksAndSerialNumberPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRemarksAndSerialNumberPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRemarksAndSerialNumberPanel icon retrieval
}

/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CRemarksAndSerialNumberPanel::OnButtonClick( wxCommandEvent& event )
{
    string doi = ToStdString(m_DOI->GetValue());
    if (NStr::IsBlank(doi)) {
        return;
    }
    // use pmcidconv_client to get PMID
    CPMCIDSearch converter;
    CPMCIDSearch::TResults results;
    vector<string> ids;
    ids.push_back(doi);
    int pmid = 0;
    if (converter.DoPMCIDSearch(ids, results)) {
        // if successful, replace with PMID for lookup (and update field)
        doi = NStr::NumericToString(results[0]);
        pmid = results[0];
    } else {
        // otherwise return
        if (NStr::StartsWith(doi, "PMC")) {
            wxMessageBox(wxT("PMC ID not found"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        } else {
            wxMessageBox(wxT("DOI not found"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        }
        return;
    }

    CRef<CPubdesc> pubdesc = GetPubdescFromEntrezById(pmid);
    if (pubdesc) {
        wxWindow* parent = this->GetParent();
        CPublicationTypePanel* listpanel = dynamic_cast<CPublicationTypePanel*>(this->GetParent());
        while (parent && !listpanel)
        {
            parent = parent->GetParent();
            listpanel = dynamic_cast<CPublicationTypePanel*>(parent);
        }
        if (listpanel) {
            listpanel->SetPub(*pubdesc);
        }
    } else {
        wxMessageBox(wxT("No article found"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }
}

END_NCBI_SCOPE

