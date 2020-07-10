/*  $Id: macro_simple.cpp 40876 2018-04-25 18:19:21Z filippov $
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
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/widgets/edit/macrofloweditor.hpp>
#include <gui/widgets/edit/macro_simple.hpp>

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
BEGIN_NCBI_SCOPE

/*!
 * CMacroSimple type definition
 */

 IMPLEMENT_CLASS(CMacroSimple, wxDialog)


/*!
 * CMacroSimple event table definition
 */
BEGIN_EVENT_TABLE(CMacroSimple, wxDialog)
    EVT_TEXT(ID_MACROACTIONPANEL, CMacroSimple::OnUpdateText)
END_EVENT_TABLE()


/*!
 * CMacroSimple constructors
 */

CMacroSimple::CMacroSimple()
{
    Init();
}

CMacroSimple::CMacroSimple( wxWindow* parent, const string& text, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Text(text)
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * CMacroSimple creator
 */

bool CMacroSimple::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroSimple creation
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
////@end CMacroSimple creation
    return true;
}


/*!
 * CMacroSimple destructor
 */

CMacroSimple::~CMacroSimple()
{   
}


/*!
 * Member initialisation
 */

void CMacroSimple::Init()
{
////@begin CMacroSimple member initialisation
    m_ActionPanel = NULL;
    m_AddToScript = NULL;
    m_Status = NULL;
////@end CMacroSimple member initialisation
}


/*!
 * Control creation for CMacroSimple
 */

void CMacroSimple::CreateControls()
{    
////@begin CMacroSimple content construction
    CMacroSimple* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemFrame1, wxID_ANY);
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    m_ActionPanel = new wxTextCtrl(itemPanel3, ID_MACROACTIONPANEL, wxString(m_Text), wxDefaultPosition, wxSize(500, 300), wxTE_MULTILINE);
    itemBoxSizer4->Add(m_ActionPanel, 1, wxEXPAND|wxALL, 0);
    m_ActionPanel->Connect(ID_MACROACTIONPANEL, wxEVT_KEY_DOWN, wxKeyEventHandler(CMacroSimple::OnKey), NULL, this);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel3, wxID_ANY, _("Validation"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxHORIZONTAL);
    itemBoxSizer4->Add(itemStaticBoxSizer6, 0, wxEXPAND | wxALL, 5);

    m_Status = new wxStaticText(itemPanel3, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer6->Add(m_Status, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer12, 0, wxEXPAND | wxALL, 0);

    itemBoxSizer12->AddStretchSpacer();

    m_AddToScript = new wxButton(itemPanel3, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(m_AddToScript, 0, wxALL, 5);
    m_AddToScript->Disable();

    wxButton* itemButton15 = new wxButton( itemPanel3, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALL, 5);

////@end CMacroSimple content construction
}


/*!
 * Should we show tooltips?
 */

bool CMacroSimple::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMacroSimple::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroSimple bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroSimple bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMacroSimple::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroSimple icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroSimple icon retrieval
}

void CMacroSimple::OnUpdateText(wxCommandEvent& event)
{
    if (!m_ActionPanel || !m_Status || !m_AddToScript)
	return;

    const string macro_script = m_ActionPanel->GetValue().ToStdString();

    CRef<macro::CMacroRep> macro(m_MEngine.Parse(macro_script));
    if (!m_MEngine.GetStatus()) {
        m_Status->SetForegroundColour(wxColor(*wxRED));
        m_Status->SetLabel(_("Validation failed"));
        if (m_AddToScript->IsEnabled()) {
            m_AddToScript->Disable();
        }
    }
    else {
        m_Status->SetForegroundColour(wxColour(0, 128, 0));
        m_Status->SetLabel(ToWxString("Validation successful"));
        m_AddToScript->Enable();
    }

    // make the warning yellow if special functions are used like UpdatemRNAProduct
    // Warning: The function <UpdatemRNAProduct> will act on all objects. Did you forget to add a constraint?
    // for now, check whether there are any constraints at all - later distinguish "relevant constraints"
}

CRef<macro::CMacroRep> CMacroSimple::GetMacro()
{
    const string macro_script = m_ActionPanel->GetValue().ToStdString();
    
    CRef<macro::CMacroRep> macro(m_MEngine.Parse(macro_script));
    if (!m_MEngine.GetStatus())
	macro.Reset();
    return macro;
}

void CMacroSimple::OnKey(wxKeyEvent & event)
{
   switch(event.GetKeyCode())
   {
       case 'a':
       case 'A':
              if(event.ControlDown())
              {
		  m_ActionPanel->SetSelection(-1, -1);
              }
              else
                  event.Skip();
              break;

       default:
              event.Skip();
              break;
   }
}

END_NCBI_SCOPE

