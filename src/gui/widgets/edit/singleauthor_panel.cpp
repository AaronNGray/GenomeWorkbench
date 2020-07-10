/*  $Id: singleauthor_panel.cpp 43930 2019-09-20 18:43:51Z asztalos $
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
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <gui/objutils/macro_fn_pubfields.hpp>
#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/widgets/edit/author_names_container.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char *arrows_xpm[] = {
/* columns rows colors chars-per-pixel */
"8 14 2 1 ",
"  c black",
". c None",
/* pixels */
"........",
"........",
".. .....",
".  .....",
"       .",
".  .....",
".. .....",
"..... ..",
".....  .",
".       ",
".....  .",
"..... ..",
"........",
"........"
};

/*
 * CSingleAuthorPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleAuthorPanel, wxPanel )


/*
 * CSingleAuthorPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleAuthorPanel, wxPanel )

    EVT_COMMAND(wxID_ANY, wxEVT_LARGE_SPIN_CTRL_EVENT, CSingleAuthorPanel::OnMoveAuthorUpdated)
    EVT_BUTTON( ID_AUTH_SWAP_NAME, CSingleAuthorPanel::OnSwapName )
END_EVENT_TABLE()


/*
 * CSingleAuthorPanel constructors
 */

CSingleAuthorPanel::CSingleAuthorPanel()
{
    Init();
}

CSingleAuthorPanel::CSingleAuthorPanel( wxWindow* parent, CAuthor& author,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Author(0)
{
    Init();
    m_Author.Reset(new CAuthor());
    m_Author->Assign(author);
    m_Author->SetName().SetName();
    Create(parent, id, pos, size, style);
}


/*
 * CSingleAuthorPanel creator
 */

bool CSingleAuthorPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleAuthorPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleAuthorPanel creation
    return true;
}


/*
 * CSingleAuthorPanel destructor
 */

CSingleAuthorPanel::~CSingleAuthorPanel()
{
////@begin CSingleAuthorPanel destruction
////@end CSingleAuthorPanel destruction
}


/*
 * Member initialisation
 */

void CSingleAuthorPanel::Init()
{
////@begin CSingleAuthorPanel member initialisation
    m_FirstNameCtrl = NULL;
    m_MiddleInitial = NULL;
    m_LastNameCtrl = NULL;
    m_Suffix = NULL;
////@end CSingleAuthorPanel member initialisation
}


/*
 * Control creation for CSingleAuthorPanel
 */

void CSingleAuthorPanel::CreateControls()
{    
////@begin CSingleAuthorPanel content construction
    CSingleAuthorPanel* itemPanel1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(2, 7, 0, 0);
    itemPanel1->SetSizer(itemFlexGridSizer2);

    m_FirstNameCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_FIRST, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_PROCESS_ENTER );
    itemFlexGridSizer2->Add(m_FirstNameCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_FirstNameCtrl->SetToolTip(wxEmptyString);
    
    m_MiddleInitial = new wxTextCtrl( itemPanel1, ID_AUTH_MI, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_PROCESS_ENTER );
    itemFlexGridSizer2->Add(m_MiddleInitial, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_MiddleInitial->SetToolTip(wxEmptyString);

    m_LastNameCtrl = new wxTextCtrl( itemPanel1, ID_AUTH_LAST, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxTE_PROCESS_ENTER );
    itemFlexGridSizer2->Add(m_LastNameCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_LastNameCtrl->SetToolTip(wxEmptyString);
    
    m_FirstNameCtrl->Connect(ID_AUTH_FIRST, wxEVT_KILL_FOCUS, wxFocusEventHandler(CSingleAuthorPanel::OnFirstNameKillFocus), NULL, this);

    m_SuffixStrings.Add(wxEmptyString);
    m_SuffixStrings.Add(_("Jr."));
    m_SuffixStrings.Add(_("Sr."));
    m_SuffixStrings.Add(_("II"));
    m_SuffixStrings.Add(_("III"));
    m_SuffixStrings.Add(_("IV"));
    m_SuffixStrings.Add(_("V"));
    m_SuffixStrings.Add(_("VI"));
    m_Suffix = new CNoTabChoice( itemPanel1, ID_AUTH_SUFFIX, wxDefaultPosition, wxSize(50, -1), m_SuffixStrings, 0 );
    itemFlexGridSizer2->Add(m_Suffix, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_Suffix->SetToolTip(wxEmptyString);

////@end CSingleAuthorPanel content construction
    TransferDataToWindow();
    m_FirstNameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_FirstName ));
    m_LastNameCtrl->SetValidator( wxTextValidator(wxFILTER_NONE, & m_LastName ));     

    // add spinner for inserting a new author
    m_InsertSpinCtrl = new CLargeNoTabSpinControl(itemPanel1, wxID_ANY, wxDefaultPosition);
    m_InsertSpinCtrl->UseImageSet(1);
    itemFlexGridSizer2->Add(m_InsertSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    // add spinner for moving authors up and down
    m_MoveSpinCtrl = new CLargeNoTabSpinControl(itemPanel1, wxID_ANY, wxDefaultPosition);
    itemFlexGridSizer2->Add(m_MoveSpinCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    wxBitmap bmp(arrows_xpm);
    m_SwapButton = new CNoTabBitmapButton(itemPanel1, ID_AUTH_SWAP_NAME, bmp, wxDefaultPosition, wxSize(CLargeSpinControl::kWidth / 2 + 4, CLargeSpinControl::kHeight + 4));
    itemFlexGridSizer2->Add(m_SwapButton, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);
    m_SwapButton->SetCanFocus(false);
}

bool CSingleAuthorPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    macro::CMacroFunction_AuthorFix::s_BuildName(ToAsciiStdString(m_FirstName),
                                                ToAsciiStdString(m_MiddleInitial->GetValue()),
                                                ToAsciiStdString(m_LastName),
                                                ToAsciiStdString(m_Suffix->GetStringSelection()),
                                                m_Author->SetName().SetName());
    return true;
}

bool CSingleAuthorPanel::TransferDataToWindow()
{
    if (m_Author->GetName().GetName().IsSetFirst()) {
        m_FirstName = ToWxString (m_Author->GetName().GetName().GetFirst());
    } else {
        m_FirstName = wxEmptyString;
    }
    if (m_Author->GetName().GetName().IsSetLast()) {
        m_LastName = ToWxString (m_Author->GetName().GetName().GetLast());
    } else {
        m_LastName = wxEmptyString;
    }

    string middle_init = x_ParseInitialsFromAuthor();
    m_MiddleInitial->ChangeValue(ToWxString(middle_init));

    m_Suffix->SetSelection(1);
    string suffix = (m_Author->SetName().SetName().IsSetSuffix()) ? m_Author->GetName().GetName().GetSuffix() : kEmptyStr;
    unsigned int i;
    for (i = 0; i < m_SuffixStrings.size(); i++) {
        if (NStr::EqualNocase(ToAsciiStdString(m_SuffixStrings[i]), suffix)) {
            m_Suffix->SetSelection (i);
            break;
        }
    }   
    return wxPanel::TransferDataToWindow();
}

void CSingleAuthorPanel::SetAuthor(const objects::CAuthor &author)
{
    m_Author.Reset(new CAuthor());
    m_Author->Assign(author);
    m_Author->SetName().SetName();
    TransferDataToWindow();
    Refresh();
}

string CSingleAuthorPanel::s_ParseInitialsFromName(const CName_std& name)
{
    if (!name.IsSetInitials())
        return kEmptyStr;

    string first_init = (name.IsSetFirst()) ? macro::CMacroFunction_AuthorFix::s_GetFirstNameInitials(name.GetFirst()) : kEmptyStr;
    string middle_init = (name.IsSetInitials()) ? name.GetInitials(): kEmptyStr;

    if (NStr::StartsWith(middle_init, first_init, NStr::eNocase)) {
        middle_init = middle_init.substr(first_init.length());
    }
    if (NStr::EndsWith(middle_init, ".")) {
        middle_init = middle_init.substr(0, middle_init.length() - 1);
    }

    return middle_init;
}

string CSingleAuthorPanel::x_ParseInitialsFromAuthor()
{
    if (m_Author->GetName().IsName()) {
        return s_ParseInitialsFromName(m_Author->GetName().GetName());
    }

    return kEmptyStr;
}

CRef<CAuthor> CSingleAuthorPanel::GetAuthor() const
{
    if (m_Author)
        return m_Author;

    return CRef<CAuthor>();
}

/*
 * Should we show tooltips?
 */

bool CSingleAuthorPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSingleAuthorPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleAuthorPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleAuthorPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSingleAuthorPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleAuthorPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleAuthorPanel icon retrieval
}

void CSingleAuthorPanel::HideNonTextCtrls()
{
    m_InsertSpinCtrl->Hide();
    m_MoveSpinCtrl->Hide();
    m_SwapButton->Hide();
    GetSizer()->SetSizeHints(this);
}


CAuthorNamesContainer* s_FindListPanel(wxWindow* parent)
{
    CAuthorNamesContainer* authlistpanel = dynamic_cast<CAuthorNamesContainer*>(parent);
    while (parent && !authlistpanel) {
        parent = parent->GetParent();
        authlistpanel = dynamic_cast<CAuthorNamesContainer*>(parent);
    }
    return authlistpanel;
}


void CSingleAuthorPanel::OnFirstNameKillFocus(wxFocusEvent& event)
{
    event.Skip();
    CAuthorNamesContainer* parent = s_FindListPanel(this->GetParent());
    if (parent) {
        parent->AddLastAuthor((wxWindow*)this);
    }
}

/*
 * wxEVT_COMMAND_SPINCTRL_UPDATED event handler for ID_MOVE_AUTHOR
 */

void CSingleAuthorPanel::OnMoveAuthorUpdated(  wxCommandEvent& evt )
{
    CAuthorNamesContainer* parent = s_FindListPanel(this->GetParent());
    if (!parent) {
        return;
    }

    switch (evt.GetId()) {
    case CLargeSpinControl::kBtnUp:
        parent->ShiftAuthorUp((wxWindow*)this);
        break;
    case CLargeSpinControl::kBtnDn:
        parent->ShiftAuthorDown((wxWindow*)this);
        break;
    case 2 + CLargeSpinControl::kBtnUp:
        parent->InsertAuthorBefore((wxWindow*)this);
        break;
    case 2 + CLargeSpinControl::kBtnDn:
        parent->InsertAuthorAfter((wxWindow*)this);
        break;
    }
}

void CSingleAuthorPanel::OnSwapName(  wxCommandEvent& evt )
{
    wxString first = m_FirstNameCtrl->GetValue();
    wxString last = m_LastNameCtrl->GetValue();
    m_FirstNameCtrl->SetValue(last);
    m_LastNameCtrl->SetValue(first);
}


bool CSingleAuthorPanel::IsPlaceholder(const CAuthor& author)
{
    if (!author.IsSetName()) {
        return false;
    }
    bool rval = false;
    const auto& p = author.GetName();
    if (p.IsName()) {
        const auto& person = p.GetName();
        if (person.IsSetLast() && person.GetLast() == "?" &&
            (!person.IsSetFirst() || NStr::IsBlank(person.GetFirst())) &&
            (!person.IsSetInitials() || NStr::IsBlank(person.GetInitials())) &&
            (!person.IsSetSuffix() || NStr::IsBlank(person.GetSuffix()))) {
            rval = true;
        }
    }
    else if (p.IsStr() && p.GetStr() == "?") {
        rval = true;
    }
    return rval;
}




END_NCBI_SCOPE


