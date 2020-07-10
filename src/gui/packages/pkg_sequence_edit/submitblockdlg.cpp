/*  $Id: submitblockdlg.cpp 40044 2017-12-13 15:38:02Z katargir $
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
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>

#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/stattext.h>

////@begin includes
#include "wx/imaglist.h"
////@end includes

// for file loader
#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/packages/pkg_sequence_edit/submitblockdlg.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSubmitBlockDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubmitBlockDlg, wxDialog )


/*!
 * CSubmitBlockDlg event table definition
 */

BEGIN_EVENT_TABLE( CSubmitBlockDlg, wxDialog )

////@begin CSubmitBlockDlg event table entries
    EVT_NOTEBOOK_PAGE_CHANGED( ID_SUBMITBLOCKDLGLISTBOOK, CSubmitBlockDlg::OnSubmitblockdlgPageChanged )

    EVT_BUTTON( ID_CLEAR_BTN, CSubmitBlockDlg::OnClearBtnClick )

    EVT_BUTTON( ID_BUTTON15, CSubmitBlockDlg::OnBackBtnClick )

    EVT_BUTTON( ID_BUTTON14, CSubmitBlockDlg::OnNextBtnClick )

    EVT_BUTTON( ID_IMPORT_BTN, CSubmitBlockDlg::OnImportBtnClick )

    EVT_BUTTON( ID_EXPORT_BTN, CSubmitBlockDlg::OnExportBtnClick )

////@end CSubmitBlockDlg event table entries
    EVT_CHECKBOX( ID_SUBMITBLOCKDLG_SAME_AUTHORS, CSubmitBlockDlg::OnSeparateAuthorsClick )
    EVT_BUTTON( ID_COPY_SEQ_AUTH_TO_MAN_AUTH, CSubmitBlockDlg::OnCopySeqAuthToManAuthClick )

END_EVENT_TABLE()


/*!
 * CSubmitBlockDlg constructors
 */

CSubmitBlockDlg::CSubmitBlockDlg()
{
    Init();
}

CSubmitBlockDlg::CSubmitBlockDlg( wxWindow* parent, CRef<objects::CSubmit_block> block, CRef<objects::CCit_gen> gen, 
                                  IWorkbench* workbench,
                                  wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style ) :
  m_Workbench (workbench)
{
    m_SubmitBlock = new objects::CSubmit_block();
    if (block) {
        m_SubmitBlock->Assign(*block);
    }
    m_Gen = new objects::CCit_gen();
    if (gen) {
        m_Gen->Assign(*gen);
    }
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * SubmitBlockDlg creator
 */

bool CSubmitBlockDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmitBlockDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmitBlockDlg creation
    return true;
}


/*!
 * CSubmitBlockDlg destructor
 */

CSubmitBlockDlg::~CSubmitBlockDlg()
{
////@begin CSubmitBlockDlg destruction
////@end CSubmitBlockDlg destruction
}


/*!
 * Member initialisation
 */

void CSubmitBlockDlg::Init()
{
////@begin CSubmitBlockDlg member initialisation
    m_Updatable = NULL;
    m_Book = NULL;
    m_SameAuthSizer = NULL;
    m_BackBtn = NULL;
    m_NextBtn = NULL;
////@end CSubmitBlockDlg member initialisation
    m_ManAuthorsPanel = NULL;
}


#define NCBI_AuthorCompareFields(Field,Obj1,Obj2) \
    if (##Obj1.IsSet##Field()) { \
        if (##Obj2.IsSet##Field()) { \
            if (!NStr::Equal(##Obj1.Get##Field(), ##Obj2.Get##Field())) { \
                return false; \
            } \
        } else if (!NStr::IsBlank(##Obj1.Get##Field())) { \
            return false; \
        } \
    } else { \
        if (##Obj2.IsSet##Field() && !NStr::IsBlank(##Obj2.Get##Field())) { \
            return false; \
        } \
    }

bool s_Name_stdsSame (const objects::CName_std& p1, const objects::CName_std& p2)
{
    if (p1.IsSetLast()) {
        if (p2.IsSetLast()) {
            if (!NStr::Equal(p1.GetLast(), p2.GetLast())) {
                return false;
            }
        } else if (!NStr::IsBlank(p1.GetLast())) {
            return false;
        }
    } else {
        if (p2.IsSetLast() && !NStr::IsBlank(p2.GetLast())) {
            return false;
        }
    }
    if (p1.IsSetFirst()) {
        if (p2.IsSetFirst()) {
            if (!NStr::Equal(p1.GetFirst(), p2.GetFirst())) {
                return false;
            }
        } else if (!NStr::IsBlank(p1.GetFirst())) {
            return false;
        }
    } else {
        if (p2.IsSetFirst() && !NStr::IsBlank(p2.GetFirst())) {
            return false;
        }
    }
    if (p1.IsSetInitials()) {
        if (p2.IsSetInitials()) {
            if (!NStr::Equal(p1.GetInitials(), p2.GetInitials())) {
                return false;
            }
        } else if (!NStr::IsBlank(p1.GetInitials())) {
            return false;
        }
    } else {
        if (p2.IsSetInitials() && !NStr::IsBlank(p2.GetInitials())) {
            return false;
        }
    }
    if (p1.IsSetSuffix()) {
        if (p2.IsSetSuffix()) {
            if (!NStr::Equal(p1.GetSuffix(), p2.GetSuffix())) {
                return false;
            }
        } else if (!NStr::IsBlank(p1.GetSuffix())) {
            return false;
        }
    } else {
        if (p2.IsSetSuffix() && !NStr::IsBlank(p2.GetSuffix())) {
            return false;
        }
    }

    return true;
}


bool s_AffilSame (const objects::CAffil& a1, const objects::CAffil& a2)
{
    if (a1.IsStr() && !a2.IsStr()) {
        return false;
    } else if (!a1.IsStr() && a2.IsStr()) {
        return false;
    } else if (a1.IsStr()) {
        return NStr::Equal(a1.GetStr(), a2.GetStr());
    } else if (a1.IsStd() && !a2.IsStd()) {
        return false;
    } else if (!a1.IsStd() && a2.IsStd()) {
        return false;
    } else if (a1.IsStd()) {
        const objects::CAffil::TStd& af1 = a1.GetStd();
        const objects::CAffil::TStd& af2 = a2.GetStd();
        string val1 = "";
        string val2 = "";
        if (af1.IsSetAffil()) {
            val1 = af1.GetAffil();
        }
        if (af2.IsSetAffil()) {
            val2 = af2.GetAffil();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetDiv()) {
            val1 = af1.GetDiv();
        }
        if (af2.IsSetDiv()) {
            val2 = af2.GetDiv();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetCity()) {
            val1 = af1.GetCity();
        }
        if (af2.IsSetCity()) {
            val2 = af2.GetCity();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetSub()) {
            val1 = af1.GetSub();
        }
        if (af2.IsSetSub()) {
            val2 = af2.GetSub();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetCountry()) {
            val1 = af1.GetCountry();
        }
        if (af2.IsSetCountry()) {
            val2 = af2.GetCountry();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetStreet()) {
            val1 = af1.GetStreet();
        }
        if (af2.IsSetStreet()) {
            val2 = af2.GetStreet();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetEmail()) {
            val1 = af1.GetEmail();
        }
        if (af2.IsSetEmail()) {
            val2 = af2.GetEmail();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetFax()) {
            val1 = af1.GetFax();
        }
        if (af2.IsSetFax()) {
            val2 = af2.GetFax();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetPhone()) {
            val1 = af1.GetPhone();
        }
        if (af2.IsSetPhone()) {
            val2 = af2.GetPhone();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
        val1 = "";
        val2 = "";
        if (af1.IsSetPostal_code()) {
            val1 = af1.GetPostal_code();
        }
        if (af2.IsSetPostal_code()) {
            val2 = af2.GetPostal_code();
        }
        if (!NStr::Equal(val1, val2)) {
            return false;
        }
    }
    return true;
}


bool s_AuthorsSame(const objects::CAuthor& a1, const objects::CAuthor& a2)
{
    if (a1.IsSetName() && !a2.IsSetName()) {
        return false;
    } else if (!a1.IsSetName() && a2.IsSetName()) {
        return false;
    } else if (!a1.IsSetName()) {
        return true;
    } else if (a1.GetName().IsStr() && !a2.GetName().IsStr()) {
        return false;
    } else if (!a1.GetName().IsStr() && a2.GetName().IsStr()) {
        return false;
    } else if (a1.GetName().IsStr()) {
        return NStr::Equal(a1.GetName().GetStr(), a2.GetName().GetStr());
    } else if (a1.GetName().IsName() && !a2.GetName().IsName()) {
        return false;
    } else if (!a1.GetName().IsName() && a2.GetName().IsName()) {
        return false;
    } else if (a1.GetName().IsName()) {
        return s_Name_stdsSame(a1.GetName().GetName(), a2.GetName().GetName());
    } else {
        return false;
    }
}


bool s_AuthorListsSame(const objects::CAuth_list& list1, const objects::CAuth_list& list2)
{
    if (list1.IsSetNames() && !list2.IsSetNames()) {
        return false;
    } else if (!list1.IsSetNames() && list2.IsSetNames()) {
        return false;
    } else if (!list1.IsSetNames()) {
        return true;
    } else if (list1.GetNames().Which() != list2.GetNames().Which()) {
        return true;
    } else if (list1.GetNames().IsStd()) {
        objects::CAuth_list::C_Names::TStd::const_iterator it1 = list1.GetNames().GetStd().begin();
        objects::CAuth_list::C_Names::TStd::const_iterator it2 = list2.GetNames().GetStd().begin();
        while (it1 != list1.GetNames().GetStd().end() && it2 != list2.GetNames().GetStd().end()) {
            if (!s_AuthorsSame(**it1, **it2)) {
                return false;
            }
            ++it1;
            ++it2;
        }
        if (it1 != list1.GetNames().GetStd().end() || it2 != list2.GetNames().GetStd().end()) {
            return false;
        }
        return true;
    } else if (list1.GetNames().IsStr()) {
        objects::CAuth_list::C_Names::TStr::const_iterator it1 = list1.GetNames().GetStr().begin();
        objects::CAuth_list::C_Names::TStr::const_iterator it2 = list2.GetNames().GetStr().begin();
        while (it1 != list1.GetNames().GetStr().end() && it2 != list2.GetNames().GetStr().end()) {
            if (!NStr::Equal(*it1, *it2)) {
                return false;
            }
            ++it1;
            ++it2;
        }
        if (it1 != list1.GetNames().GetStr().end() || it2 != list2.GetNames().GetStr().end()) {
            return false;
        }
        return true;
    } else {
        return true;
    }
}


/*!
 * Control creation for SubmitBlockDlg
 */

void CSubmitBlockDlg::CreateControls()
{    
////@begin CSubmitBlockDlg content construction
    // Generated by DialogBlocks, 10/06/2013 17:56:49 (unregistered)

    CSubmitBlockDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Updatable = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_Updatable, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Book = new wxNotebook( itemDialog1, ID_SUBMITBLOCKDLGLISTBOOK, wxDefaultPosition, wxSize(600, -1), wxBK_DEFAULT );

    m_Updatable->Add(m_Book, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_SameAuthSizer = new wxBoxSizer(wxVERTICAL);
    m_Updatable->Add(m_SameAuthSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_CLEAR_BTN, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_BackBtn = new wxButton( itemDialog1, ID_BUTTON15, _("< Back"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BackBtn->Enable(false);
    itemBoxSizer8->Add(m_BackBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NextBtn = new wxButton( itemDialog1, ID_BUTTON14, _("Next >"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, ID_IMPORT_BTN, _("Import Template"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_EXPORT_BTN, _("Export Template"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSubmitBlockDlg content construction

    x_BuildPages();
}


void CSubmitBlockDlg::x_BuildPages()
{
    m_ReleaseDatePanel = new CReleaseDatePanel(m_Book, *m_SubmitBlock);
    m_Book->AddPage(m_ReleaseDatePanel, wxT("Release Date"));

    wxPanel* contact_panel = new wxPanel(m_Book);    
    wxSizer* contact_sizer = new wxBoxSizer(wxVERTICAL);
    contact_panel->SetSizer(contact_sizer);
    m_ContactPanel = new CContactPanel (contact_panel, m_SubmitBlock->SetContact());
    contact_sizer->Add(m_ContactPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxSizer* alt_sizer = new wxBoxSizer(wxHORIZONTAL);
    contact_sizer->Add(alt_sizer, 0, wxALIGN_LEFT|wxALL, 5);
    wxStaticText* label = new wxStaticText(contact_panel, wxID_ANY, _("Alternate Email Address"));
    alt_sizer->Add(label, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_AlternateEmail = new wxTextCtrl(contact_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, -1));
    alt_sizer->Add(m_AlternateEmail, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);  

    m_Book->AddPage(contact_panel, wxT("Contact"));

    objects::CAuth_list& auth_list = m_SubmitBlock->SetCit().SetAuthors();
    m_SequenceAuthorsPanel = new CAuthorNamesPanel(m_Book, auth_list, false);
    m_Book->AddPage(m_SequenceAuthorsPanel, wxT("Sequence Authors"));
    m_AffilPanel = new CAuthorAffiliationPanel(m_Book, auth_list.SetAffil()); 
    m_Book->AddPage (m_AffilPanel, wxT("Authors Affiliation"));

    m_UnpublishedPanel = new CUnpublishedPanel(m_Book, m_Gen->IsSetTitle() ? m_Gen->GetTitle() : kEmptyStr);
    m_Book->AddPage(m_UnpublishedPanel, wxT("Manuscript Title"));

    objects::CAuth_list& man_auth_list = m_Gen->SetAuthors();
    bool authors_same = s_AuthorListsSame(auth_list, man_auth_list);

    m_AuthorsSame = new wxCheckBox (this, ID_SUBMITBLOCKDLG_SAME_AUTHORS, _("Manuscript authors are same as sequence authors"));
    m_SameAuthSizer->Add(m_AuthorsSame, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_AuthorsSame->SetValue(authors_same);
    m_CopySeqAuthToManAuth = new wxButton( this, ID_COPY_SEQ_AUTH_TO_MAN_AUTH, _("Copy Sequence Authors to Manuscript Authors"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SameAuthSizer->Add(m_CopySeqAuthToManAuth, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_CopySeqAuthToManAuth->Enable(!m_AuthorsSame);
    if (!authors_same) {
        m_ManAuthorsPanel = new CAuthorNamesPanel(m_Book, man_auth_list, false);
        m_Book->AddPage (m_ManAuthorsPanel, wxT("Manuscript Authors"));
    }

    m_ReleaseDatePanel->TransferDataToWindow();
    m_ContactPanel->TransferDataToWindow();
    m_SequenceAuthorsPanel->TransferDataToWindow();
    m_AffilPanel->TransferDataToWindow();
    if (m_ManAuthorsPanel) {
        m_ManAuthorsPanel->TransferDataToWindow();
    }
}


/*!
 * Should we show tooltips?
 */

bool CSubmitBlockDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSubmitBlockDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmitBlockDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmitBlockDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSubmitBlockDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmitBlockDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmitBlockDlg icon retrieval
}


static bool s_NamesNotSet (const objects::CAuth_list::TNames& names)
{
    if (names.Which() == objects::CAuth_list::TNames::e_not_set
        || (names.IsStr() && names.GetStr().empty())
        || (names.IsStd() && names.GetStd().empty())
        || (names.IsMl() && names.GetMl().empty())) 
    {
        return true;
    }
    else
    {
        return false;
    }
}
    

bool s_AuthorNameBlank (const objects::CAuthor& auth)
{
    bool is_blank = false;
    if (!auth.IsSetName())
    {
        is_blank = true;
    }
    else if (auth.GetName().IsStr()) 
    {
        if (NStr::IsBlank(auth.GetName().GetStr()))
        {
            is_blank = true;
        }
    } 
    else if (auth.GetName().IsName())
    {
        const objects::CName_std& name = auth.GetName().GetName();
        if ((!name.IsSetLast() || NStr::IsBlank(name.GetLast())) 
            && (!name.IsSetFirst() || NStr::IsBlank(name.GetFirst()))
            && (!name.IsSetInitials() || NStr::IsBlank(name.GetInitials()))
            && (!name.IsSetSuffix() || NStr::IsBlank(name.GetSuffix())))
        {
            is_blank = true;
        }
    }
    return is_blank;
}


bool CSubmitBlockDlg::x_CopyAuthors()
{
    bool rval = false;
    objects::CAuth_list::TNames& names = m_SubmitBlock->SetCit().SetAuthors().SetNames();
    objects::CContact_info& contact = m_SubmitBlock->SetContact();
    if (s_NamesNotSet(names) || (names.IsStr() && names.GetStr().size() == 1 && NStr::IsBlank(names.GetStr().front()))) 
    {
        // if no authors, but contact filled in, add contact author
        if (!s_AuthorNameBlank(contact.SetContact())) 
        {
            CRef<objects::CAuthor> new_auth(new objects::CAuthor());      
            new_auth->Assign (contact.GetContact());
            names.SetStd().push_back(new_auth);
            rval = true;
        }
    }
    else
    {
        // if contact is blank, copy first author
        if (s_AuthorNameBlank(contact.SetContact()) && names.IsStd()) 
        {
            contact.SetContact().Assign(*names.GetStd().front());
            rval = true;
        }
    }
    return rval;
}


bool s_AuthListEmpty (const objects::CAuth_list::TNames& names)
{
    bool any = false;
    if (s_NamesNotSet(names)) {
        return true;
    } else if (names.IsStr()) {
        ITERATE (objects::CAuth_list::TNames::TStr, it, names.GetStr()) {
            if (!NStr::IsBlank(*it)) {
                any = true;
                break;
            }
        }
    } else if (names.IsStd()) {
        ITERATE (objects::CAuth_list::TNames::TStd, it, names.GetStd()) {
            if (!s_AuthorNameBlank(**it)) {
                any = true;
                break;
            }
        }
    }
    return !any;
}


const string kAllSubmitMissing = "Sequence authors, affiliation, and contact information is missing!";
const string kSequenceAuthorsMissing = "You must provide sequence authors.";
const string kAffilMissing = "You must provide an affiliation.";
const string kContactMissing = "You must provide a contact name.";
const string kEmailMissing = "You must provide an email address.";
const string kCityMissing = "You must provide a city in affiliation.";
const string kCountryMissing = "You must provide a country in affiliation.";

const string kAllCitGenMissing = "Manuscript title, authors, and affiliation is missing!";
const string kTitleMissing = "You must provide a manuscript title.";
const string kManuscriptAuthorsMissing = "You must provide authors.";
const string kInvalidEmail = "Email address is invalid.";


string CSubmitBlockDlg::GetBlockProblems(CRef<objects::CSubmit_block> block)
{
    string err = "";
    bool missing_authors = true;
    bool missing_affil = true;
    bool missing_contact = true;
    bool missing_email = true;
    bool missing_city = true;
    bool missing_country = true;

    if (block && block->IsSetCit()) {
        if (block->GetCit().IsSetAuthors()) {
            const objects::CAuth_list& auth_list = block->GetCit().GetAuthors();
            if (auth_list.IsSetNames()
                && !s_AuthListEmpty(auth_list.GetNames())) {
                missing_authors = false;
            }
            if (auth_list.IsSetAffil()) {
                if (((auth_list.GetAffil().IsStr() && !NStr::IsBlank(auth_list.GetAffil().GetStr()))
                    || (auth_list.GetAffil().IsStd() && auth_list.GetAffil().GetStd().IsSetAffil()
                        && !NStr::IsBlank(auth_list.GetAffil().GetStd().GetAffil())))) {
                    missing_affil = false;
                }
                if (auth_list.GetAffil().IsStd()) {
                    if (auth_list.GetAffil().GetStd().IsSetCity() && !NStr::IsBlank(auth_list.GetAffil().GetStd().GetCity())) {
                        missing_city = false;
                    }
                    if (auth_list.GetAffil().GetStd().IsSetCountry() && !NStr::IsBlank(auth_list.GetAffil().GetStd().GetCountry())) {
                        missing_country = false;
                    }
                }
            }            
        }
    }
    if (block && block->IsSetContact()) {
        if (block->GetContact().IsSetContact()) {
            missing_contact = false;
        }
        if (block->GetContact().IsSetEmail()) {
            missing_email = false;
        }
    } 

    if (missing_authors && missing_affil && missing_contact && missing_email) {
        err = kAllSubmitMissing;
    } else if (missing_authors) {
        err = kSequenceAuthorsMissing;
    } else if (missing_affil) {
        err = kAffilMissing;
    } else if (missing_contact) {
        err = kContactMissing;
    } else if (missing_email) {
        err = kEmailMissing;
    } else if (missing_city) {
        err = kCityMissing;
    } else if (missing_country) {
        err = kCountryMissing;
    }

    if (NStr::IsBlank(err)) {
        if (block->IsSetHup() && block->IsSetReldate()) {
            try {
                CTime release_date = block->GetReldate().AsCTime();
                if (release_date < CTime(CTime::eCurrent)) {
                    err = "Release date is in the past.";
                }
            } catch (exception &) {
                err = "Release date is invalid";
            }
        }
    }
    if (NStr::IsBlank(err)) {
        if (!IsValidEmail(block->GetContact().GetEmail())) {
            err = kInvalidEmail;
        }
    }

    return err;
}


enum
{
    ePageDate = 0,
    ePageContact = 1,
    ePageSequenceAuth = 2,
    ePageAffil = 3 ,
    ePageTitle = 4,
    ePageManAuth = 5
};


void CSubmitBlockDlg::SetPageForError(string errors)
{
    if (NStr::IsBlank(errors)) {
        return;
    }
    if (NStr::FindNoCase(errors, kAllSubmitMissing) != string::npos) {
        m_Book->SetSelection(ePageContact);
    } else if (NStr::FindNoCase(errors, kSequenceAuthorsMissing) != string::npos) {
        m_Book->SetSelection(ePageSequenceAuth);
    } else if (NStr::FindNoCase(errors, kAffilMissing) != string::npos
               || NStr::FindNoCase(errors, kCityMissing) != string::npos
               || NStr::FindNoCase(errors, kCountryMissing) != string::npos) {
        m_Book->SetSelection(ePageAffil);
    } else if (NStr::FindNoCase(errors, kContactMissing) != string::npos) {
        m_Book->SetSelection(ePageContact);
    } else if (NStr::FindNoCase(errors, "release date") != string::npos) {
        m_Book->SetSelection(ePageDate);
    } else if (NStr::FindNoCase(errors, kAllCitGenMissing) != string::npos) {
        m_Book->SetSelection(ePageTitle);
    } else if (NStr::FindNoCase(errors, kTitleMissing) != string::npos) {
        m_Book->SetSelection(ePageTitle);
    } else if (NStr::FindNoCase(errors, kManuscriptAuthorsMissing) != string::npos) {
        if (m_Book->GetPageCount() == 6) {
            m_Book->SetSelection(ePageManAuth);
        } else {
            m_Book->SetSelection(ePageSequenceAuth);
        }
    } else if (NStr::FindNoCase(errors, kEmailMissing) != string::npos
               || NStr::FindNoCase(errors, kInvalidEmail) != string::npos
               || NStr::FindNoCase(errors, "email") != string::npos) {
        m_Book->SetSelection(ePageContact);
    }
}


void CSubmitBlockDlg::UnselectReleaseDateChoice()
{
    m_ReleaseDatePanel->UnselectReleaseDateChoice();
}


CRef<objects::CSubmit_block> CSubmitBlockDlg::x_CreateBlock()
{
    m_ReleaseDatePanel->TransferDataFromWindow();
    m_ContactPanel->TransferDataFromWindow();
    objects::CAuth_list& auth_list = m_SubmitBlock->SetCit().SetAuthors();
    m_SequenceAuthorsPanel->TransferDataFromWindow();
    m_SequenceAuthorsPanel->PopulateAuthors(auth_list);
    m_AffilPanel->TransferDataFromWindow();
    CRef<objects::CAffil> affil = m_AffilPanel->GetAffil();
    if (affil)
        auth_list.SetAffil(*affil);
    else
        auth_list.ResetAffil();
   

    if (x_CopyAuthors()) {
        m_ContactPanel->TransferDataToWindow();
        m_ContactPanel->TransferDataFromWindow();
        m_SequenceAuthorsPanel->TransferDataToWindow();
        m_SequenceAuthorsPanel->TransferDataFromWindow();
        m_SequenceAuthorsPanel->PopulateAuthors(m_SubmitBlock->SetCit().SetAuthors());
    }

    CRef<objects::CSubmit_block> block(new objects::CSubmit_block());
    block->Assign(*m_SubmitBlock);
    block->SetTool("Genome Workbench Submission Preparation Tool " + kSubmissionPreparationToolVersion);
    objects::CAuth_list::TNames& names = block->SetCit().SetAuthors().SetNames();
    if (s_NamesNotSet(names)) {
        names.SetStr().push_back(" ");
    }
    if (!block->GetCit().GetAuthors().IsSetAffil()) {
        block->SetCit().SetAuthors().SetAffil().SetStd().SetAffil(" ");
    }
    block->SetContact().SetContact().SetAffil().Assign(block->GetCit().GetAuthors().GetAffil());
    if (s_AuthorNameBlank(block->SetContact().SetContact())) {
        block->SetContact().SetContact().SetName().SetStr(" ");
    }

    // set submit date.  use today as default value
    if (!block->GetCit().IsSetDate()) {
        CRef<objects::CDate> submit_date(new objects::CDate(CTime(CTime::eCurrent), objects::CDate::ePrecision_day));
        block->SetCit().SetDate(*submit_date);        
    }

    return block;
}


CRef<objects::CSubmit_block> CSubmitBlockDlg::GetSubmitBlock()
{
    CRef<objects::CSubmit_block> block = x_CreateBlock();

    string err = m_ReleaseDatePanel->GetProblems();
    if (NStr::IsBlank(err)) {
        err = GetBlockProblems(block);
    }
    if (!NStr::IsBlank(err)) {
        CRef<objects::CSubmit_block> empty;
        return empty;
    }
    
    return block;
}


CRef<objects::CCit_gen> CSubmitBlockDlg::x_CreateCitGen()
{
    string title = m_UnpublishedPanel->GetTitle();
    if (title.empty())
        m_Gen->ResetTitle();
    else
        m_Gen->SetTitle(title);

    if (m_ManAuthorsPanel) {
        m_ManAuthorsPanel->TransferDataFromWindow();
        m_ManAuthorsPanel->PopulateAuthors(m_Gen->SetAuthors());
    } else {
        m_SequenceAuthorsPanel->TransferDataFromWindow();
        m_SequenceAuthorsPanel->PopulateAuthors(m_Gen->SetAuthors());
    }
    CAuth_list& auth_list = m_SubmitBlock->SetCit().SetAuthors();
    m_AffilPanel->TransferDataFromWindow();
    CRef<objects::CAffil> affil = m_AffilPanel->GetAffil();
    if (affil)
    {
        auth_list.SetAffil(*affil);
        m_Gen->SetAuthors().SetAffil(*affil);
    }
    else
    {
        auth_list.ResetAffil();
        m_Gen->SetAuthors().ResetAffil();
    }
    

    m_Gen->SetCit("Unpublished");

    // set submit date.  use today as default value
    if (!m_Gen->IsSetDate()) {
        CRef<objects::CDate> submit_date(new objects::CDate(CTime(CTime::eCurrent), objects::CDate::ePrecision_day));
        m_Gen->SetDate(*submit_date);        
    }

    CRef<objects::CCit_gen> gen(new objects::CCit_gen());
    gen->Assign(*m_Gen);
    return gen;
}


string CSubmitBlockDlg::GetCitGenProblems(CRef<objects::CCit_gen> gen)
{
    string err = "";
    bool missing_authors = true;
    bool missing_affil = true;
    bool missing_title = true;

    if (gen) {
        if (gen->IsSetTitle() && !NStr::IsBlank(gen->GetTitle())) {
            missing_title = false;
        }
        if (gen->IsSetAuthors()) {
            const objects::CAuth_list& auth_list = gen->GetAuthors();
            if (auth_list.IsSetNames()
                && !s_AuthListEmpty(auth_list.GetNames())) {
                missing_authors = false;
            }
            if (auth_list.IsSetAffil()
                && ((auth_list.GetAffil().IsStr() && !NStr::IsBlank(auth_list.GetAffil().GetStr()))
                    || (auth_list.GetAffil().IsStd() && auth_list.GetAffil().GetStd().IsSetAffil()
                        && !NStr::IsBlank(auth_list.GetAffil().GetStd().GetAffil())))) {
                missing_affil = false;
            }
        }
    } 

    if (missing_title && missing_authors && missing_affil) {
        err = kAllCitGenMissing;
    } else if (missing_title) {
        err = kTitleMissing ;
    } else if (missing_authors) {
        err = kManuscriptAuthorsMissing;
    } else if (missing_affil) {
        err = kAffilMissing;
    }

    return err;
}


CRef<objects::CCit_gen> CSubmitBlockDlg::GetCitGen ()
{
    CRef<objects::CCit_gen> gen = x_CreateCitGen();

    string err = GetCitGenProblems(gen);
    if (!NStr::IsBlank(err)) {
        CRef<objects::CCit_gen> empty;
        return empty;
    }
    
    return gen;
}


void CSubmitBlockDlg::SetAlternateEmailAddress (string alt_email)
{
    m_AlternateEmail->SetValue(ToWxString(alt_email));
}


string CSubmitBlockDlg::GetAlternateEmailAddress ()
{
    string alt_email = ToAsciiStdString(m_AlternateEmail->GetValue());
    return alt_email;
}


string CSubmitBlockDlg::GetErrors()
{
    CRef<objects::CSubmit_block> block = x_CreateBlock();
    CRef<objects::CCit_gen> gen = x_CreateCitGen();

    string err = m_ReleaseDatePanel->GetProblems();
    string block_err = GetBlockProblems(block);
    if (!NStr::IsBlank(block_err)) {
        if (!NStr::IsBlank(err)) {
            err += " ";
        }
        err += block_err;
    }
    string cit_gen_err = GetCitGenProblems(gen);
    if (!NStr::IsBlank(cit_gen_err)) {
        if (!NStr::IsBlank(err)) {
            err += " ";
        }
        err += cit_gen_err;
    }

    return err;
}


/*!
 * wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED event handler for ID_SUBMITBLOCKDLGLISTBOOK
 */

void CSubmitBlockDlg::OnSubmitblockdlgPageChanged( wxNotebookEvent& event )
{   
    if ( this->wxWindow::IsShown() ) // Added by Igor, otherwise Crashes on Linux - see Pitfals at http://wiki.wxwidgets.org/WxNoteBook 
    {
        m_ContactPanel->TransferDataFromWindow();
        m_SequenceAuthorsPanel->TransferDataFromWindow();
        m_SequenceAuthorsPanel->PopulateAuthors(m_SubmitBlock->SetCit().SetAuthors());
        if (x_CopyAuthors()) {
            m_ContactPanel->TransferDataToWindow();
            m_SequenceAuthorsPanel->TransferDataToWindow();
        }
        int curr = m_Book->GetSelection();
        if (curr == 0) {
            m_BackBtn->Enable(false);
        } else {
            m_BackBtn->Enable(true);
        }
        if (curr == m_Book->GetPageCount() - 1) {
            m_NextBtn->Enable(false);
        } else {
            m_NextBtn->Enable(true);
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON15
 */

void CSubmitBlockDlg::OnBackBtnClick( wxCommandEvent& event )
{
    int curr = m_Book->GetSelection();
    if (curr > 0) {
        m_Book->SetSelection(curr - 1);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON14
 */

void CSubmitBlockDlg::OnNextBtnClick( wxCommandEvent& event )
{
    int curr = m_Book->GetSelection();
    if (curr < m_Book->GetPageCount() - 1) {
        m_Book->SetSelection(curr + 1);
    }
}


void CSubmitBlockDlg::OnSeparateAuthorsClick( wxCommandEvent& event )
{
    if (m_AuthorsSame->GetValue()) {        
        if (m_ManAuthorsPanel) {
            m_Book->RemovePage(m_Book->GetPageCount() - 1);
            m_ManAuthorsPanel = NULL;
        }
        m_CopySeqAuthToManAuth->Enable(false);
    } else {
        objects::CAuth_list& man_auth_list = m_Gen->SetAuthors();
        if (m_ManAuthorsPanel) {
            m_ManAuthorsPanel->SetAuthors(man_auth_list);
        } else {
            m_ManAuthorsPanel = new CAuthorNamesPanel(m_Book, man_auth_list, false);
            m_Book->AddPage (m_ManAuthorsPanel, wxT("Manuscript Authors"));
            m_ManAuthorsPanel->TransferDataToWindow();
        }
        m_CopySeqAuthToManAuth->Enable(true);
    }
    Refresh();
}


void CSubmitBlockDlg::OnCopySeqAuthToManAuthClick( wxCommandEvent& event )
{
    objects::CAuth_list& man_auth_list = m_Gen->SetAuthors();
    m_SequenceAuthorsPanel->TransferDataFromWindow();
    m_SequenceAuthorsPanel->PopulateAuthors(man_auth_list);
    m_ManAuthorsPanel->SetAuthors(man_auth_list);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT_BTN
 */

void CSubmitBlockDlg::OnImportBtnClick( wxCommandEvent& event )
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
	vector<string> format_ids;
	format_ids.push_back("file_loader_asn");
	fileManager->LoadFormats(format_ids);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTable");
    dlg.SetManagers(loadManagers);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) {
            wxMessageBox(wxT("Failed to get object loader"), wxT("Error"),
                         wxOK | wxICON_ERROR);
        } else {
            IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
            if (execute_unit) {
                if (!execute_unit->PreExecute())
                    return;

                if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
                    return; // Canceled

                if (!execute_unit->PostExecute())
                    return;
            }
            const IObjectLoader::TObjects& objects = object_loader->GetObjects();
            bool any_unrecognized = false;
            bool any_change = false;
            string alt_email = "";
            ITERATE(IObjectLoader::TObjects, obj_it, objects) {
                const CObject& ptr = obj_it->GetObject();
                const objects::CSubmit_block* block = dynamic_cast<const objects::CSubmit_block*>(&ptr);
                if (block) {    
                    if (!m_SubmitBlock) {
                        m_SubmitBlock.Reset(new objects::CSubmit_block());
                    }
                    m_SubmitBlock->Assign(*block);
                    any_change = true;
                } else {
                    const objects::CSeqdesc* desc = dynamic_cast<const objects::CSeqdesc*>(&ptr);
                    if (desc) {
                        if (desc->IsPub() && desc->GetPub().IsSetPub() && desc->GetPub().GetPub().IsSet()
                            && desc->GetPub().GetPub().Get().size() == 1
                            && desc->GetPub().GetPub().Get().front()->IsGen()) {
                            if (!m_Gen) {
                                m_Gen.Reset(new objects::CCit_gen());
                            }
                            m_Gen->Assign(desc->GetPub().GetPub().Get().front()->GetGen());
                            any_change = true;
                        } else if (desc->IsUser() && IsBankItSubmissionObject(desc->GetUser())) {
                            alt_email = GetDescAlternateEmailAddress(*desc);
                            any_change = true;
                        } else {
                            any_unrecognized = true;
                        }
                    } else {
                        any_unrecognized = true;
                    }
                }
            }
            if (any_unrecognized) {
                wxMessageBox(wxT("Warning: Unrecognized objects in template file"), wxT("Error"),
                             wxOK | wxICON_ERROR);
            }
            if (any_change) {      
                while (m_Updatable->GetItemCount() > 0) {
                    size_t pos = 0;
                    m_Updatable->GetItem(pos)->DeleteWindows();
                    m_Updatable->Remove(pos);
                }
                m_Book = NULL;
                
                m_ManAuthorsPanel = NULL; 
                m_SameAuthSizer = NULL;
                m_ReleaseDatePanel = NULL;
                m_SequenceAuthorsPanel = NULL;
                m_AffilPanel = NULL;
                m_ContactPanel = NULL;
                m_ManAuthorsPanel = NULL;
                m_UnpublishedPanel = NULL; 
                m_AuthorsSame = NULL;
                m_AffilsSame = NULL;

                m_Book = new wxNotebook( this, ID_SUBMITBLOCKDLGLISTBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );
                m_Updatable->Add(m_Book, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
                m_SameAuthSizer = new wxBoxSizer(wxVERTICAL);
                m_Updatable->Add(m_SameAuthSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

                x_BuildPages();
                m_AlternateEmail->SetValue(ToWxString(alt_email));
                Layout();
                Refresh();

                CRef<objects::CSubmit_block> block = x_CreateBlock();
                if (block && block->IsSetHup() && block->IsSetReldate()) {
                    CTime release_date = block->GetReldate().AsCTime();
                    if (release_date < CTime(CTime::eCurrent)) {
                        wxMessageBox(wxT("Release date is in the past."), wxT("Error"),
                                 wxOK | wxICON_ERROR);
                    }
                }

            }
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_BTN
 */

void CSubmitBlockDlg::OnExportBtnClick( wxCommandEvent& event )
{
    string errors = GetErrors();
    if (!NStr::IsBlank(errors)) {
        wxString wxs(errors);
        wxMessageBox(wxT("Cannot export invalid template: " + wxs), wxT("Error"),
                  wxOK | wxICON_ERROR);
        return;
    }        

    wxString extensions = GetAsnSqnExtensions();
    wxFileDialog asn_save_file(this, wxT("Select a file"), m_SaveFileDir, m_SaveFileName,
                               //CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               wxT("ASN.1 files (")+extensions + wxT(")|") + extensions + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (asn_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = asn_save_file.GetPath();
        wxString name = asn_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {
            wxMessageBox(wxT("Please, select file name"), wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            return; 
        }

        if (name.Find(wxUniChar('.')) == wxNOT_FOUND)
        {
            path += wxT(".sqn");
            name += wxT(".sqn");
        }

        ios::openmode mode = ios::out;
        CNcbiOfstream os(path.fn_str(), mode); 
        if (!os)
        {
            wxMessageBox(wxT("Cannot open file ")+name, wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            return; 
        }
        os << MSerial_AsnText;
        
        CRef<objects::CSubmit_block> block = GetSubmitBlock();
        const CSerialObject* so = block.GetPointer();
        
        if (so) 
            os << *so;       

        CRef<objects::CCit_gen> gen = GetCitGen ();
        if (gen) {
            CRef<objects::CPub> pub(new objects::CPub());
            pub->SetGen().Assign(*gen);
            CRef<objects::CSeqdesc> desc(new objects::CSeqdesc());
            desc->SetPub().SetPub().Set().push_back(pub);
            so = desc.GetPointer();
            os << *so;
        }

        string alt_email = ToAsciiStdString(m_AlternateEmail->GetValue());
        if (!NStr::IsBlank(alt_email)) {
            CRef<objects::CUser_object> u = MakeBankItSubmissionObject ();
            SetUserAlternateEmailAddress(*u, alt_email);
            CRef<objects::CSeqdesc> desc(new objects::CSeqdesc());
            desc->SetUser().Assign(*u);
            so = desc.GetPointer();
            os << *so;
        }
        
        m_SaveFileDir = asn_save_file.GetDirectory();
        m_SaveFileName = name;
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_BTN
 */

void CSubmitBlockDlg::OnClearBtnClick( wxCommandEvent& event )
{
    // clear currently shown page
    switch (m_Book->GetSelection()) {
        case ePageDate:
            m_SubmitBlock->ResetReldate();
            m_ReleaseDatePanel->TransferDataToWindow();
            m_ReleaseDatePanel->UnselectReleaseDateChoice();
            break;
        case ePageContact:
            m_SubmitBlock->SetContact().Reset();
            m_ContactPanel->TransferDataToWindow();
            m_AlternateEmail->SetValue(wxEmptyString);
            break;
        case ePageSequenceAuth:
            m_SubmitBlock->SetCit().SetAuthors().ResetNames();
            m_SequenceAuthorsPanel->SetAuthors(m_SubmitBlock->SetCit().SetAuthors());
            break;
        case ePageAffil:
            m_SubmitBlock->SetCit().SetAuthors().SetAffil().Reset();
            m_AffilPanel->SetAffil(CRef<objects::CAffil>(NULL));
            m_AffilPanel->TransferDataToWindow();
            break;
        case ePageTitle:
            m_Gen->ResetTitle();
            m_UnpublishedPanel->SetTitle(kEmptyStr);
            break;
        case ePageManAuth:
            m_Gen->ResetAuthors();
            m_ManAuthorsPanel->SetAuthors(m_Gen->SetAuthors());
            break;        
    }
}


END_NCBI_SCOPE


