/*  $Id: simple_text_edit_dlg.cpp 40897 2018-04-26 20:46:56Z filippov $
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

#include <gui/widgets/edit/simple_text_edit_dlg.hpp>

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
BEGIN_NCBI_SCOPE

/*!
 * CSimpleTextEditor type definition
 */

 IMPLEMENT_CLASS(CSimpleTextEditor, wxDialog)


/*!
 * CSimpleTextEditor event table definition
 */
BEGIN_EVENT_TABLE(CSimpleTextEditor, wxDialog)
END_EVENT_TABLE()


/*!
 * CSimpleTextEditor constructors
 */

CSimpleTextEditor::CSimpleTextEditor()
{
    Init();
}

CSimpleTextEditor::CSimpleTextEditor( wxWindow* parent, const wxString& text, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Text(text)
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*!
 * CSimpleTextEditor creator
 */

bool CSimpleTextEditor::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSimpleTextEditor creation
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
////@end CSimpleTextEditor creation
    return true;
}


/*!
 * CSimpleTextEditor destructor
 */

CSimpleTextEditor::~CSimpleTextEditor()
{   
}


/*!
 * Member initialisation
 */

void CSimpleTextEditor::Init()
{
////@begin CSimpleTextEditor member initialisation
    m_Panel = NULL;
////@end CSimpleTextEditor member initialisation
}


/*!
 * Control creation for CSimpleTextEditor
 */

void CSimpleTextEditor::CreateControls()
{    
////@begin CSimpleTextEditor content construction
    CSimpleTextEditor* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemFrame1, wxID_ANY);
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    m_Panel = new wxTextCtrl(itemPanel3, ID_SIMPLE_TEXT_EDIT_PANEL, m_Text, wxDefaultPosition, wxSize(500, 300), wxTE_MULTILINE);
    itemBoxSizer4->Add(m_Panel, 1, wxEXPAND|wxALL, 0);
    m_Panel->Connect(ID_SIMPLE_TEXT_EDIT_PANEL, wxEVT_KEY_DOWN, wxKeyEventHandler(CSimpleTextEditor::OnKey), NULL, this);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer12, 0, wxEXPAND | wxALL, 0);

    itemBoxSizer12->AddStretchSpacer();

    wxButton* itemButton14 = new wxButton(itemPanel3, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton14, 0, wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemPanel3, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALL, 5);

////@end CSimpleTextEditor content construction
}


/*!
 * Should we show tooltips?
 */

bool CSimpleTextEditor::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSimpleTextEditor::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSimpleTextEditor bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSimpleTextEditor bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSimpleTextEditor::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSimpleTextEditor icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSimpleTextEditor icon retrieval
}



wxString CSimpleTextEditor::GetValue()
{
    return m_Panel->GetValue();
}

void CSimpleTextEditor::OnKey(wxKeyEvent & event)
{
   switch(event.GetKeyCode())
   {
       case 'a':
       case 'A':
              if(event.ControlDown())
              {
		  m_Panel->SetSelection(-1, -1);
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

