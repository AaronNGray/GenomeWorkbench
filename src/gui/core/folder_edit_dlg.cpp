/*  $Id: folder_edit_dlg.cpp 34244 2015-12-03 16:46:55Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/core/folder_edit_dlg.hpp>
#include <gui/core/fname_validator.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/gbproj/ProjectItem.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>

#include <serial/iterator.hpp>

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CFolderEditDlg, CDialog )

BEGIN_EVENT_TABLE( CFolderEditDlg, CDialog )

////@begin CFolderEditDlg event table entries
    EVT_INIT_DIALOG( CFolderEditDlg::OnInitDialog )

    EVT_BUTTON( ID_RESTORE, CFolderEditDlg::OnRestoreBtn )

    EVT_BUTTON( wxID_OK, CFolderEditDlg::OnOkClick )

////@end CFolderEditDlg event table entries

END_EVENT_TABLE()

CFolderEditDlg::CFolderEditDlg()
{
    Init();
}

CFolderEditDlg::CFolderEditDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CFolderEditDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFolderEditDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFolderEditDlg creation
    return true;
}


CFolderEditDlg::~CFolderEditDlg()
{
////@begin CFolderEditDlg destruction
////@end CFolderEditDlg destruction
}


void CFolderEditDlg::Init()
{
////@begin CFolderEditDlg member initialisation
    m_NameCtrl = NULL;
    m_RestoreBtn = NULL;
    m_DescrCtrl = NULL;
////@end CFolderEditDlg member initialisation
    m_Object = NULL;
    m_Scope = NULL;
    m_NameValidator = NULL;
    m_ReadOnly = false;
}


void CFolderEditDlg::CreateControls()
{
////@begin CFolderEditDlg content construction
    CFolderEditDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer5, 1, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    m_NameCtrl = new wxTextCtrl( itemCDialog1, ID_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(m_NameCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RestoreBtn = new wxButton( itemCDialog1, ID_RESTORE, _("Restore from Data"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(m_RestoreBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    m_DescrCtrl = new wxTextCtrl( itemCDialog1, ID_DESCR, wxEmptyString, wxDefaultPosition, itemCDialog1->ConvertDialogToPixels(wxSize(160, 60)), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(m_DescrCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(1);
    itemFlexGridSizer3->AddGrowableCol(1);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer10 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer10, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton11 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton11->SetDefault();
    itemStdDialogButtonSizer10->AddButton(itemButton11);

    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer10->AddButton(itemButton12);

    itemStdDialogButtonSizer10->Realize();

    // Set validators
    m_NameCtrl->SetValidator( wxGenericValidator(& m_Name) );
    m_DescrCtrl->SetValidator( wxGenericValidator(& m_Descr) );
////@end CFolderEditDlg content construction
}


void CFolderEditDlg::OnOkClick( wxCommandEvent& WXUNUSED(event) )
{
    if(TransferDataFromWindow())    {
        if(m_NameValidator) {
            string err;
            if( ! m_NameValidator->IsValid(ToStdString(m_Name), err))    {
                NcbiErrorBox(err);
                return;
            }
        }
        EndModal(wxID_OK);
    }
}


void CFolderEditDlg::OnRestoreBtn( wxCommandEvent& WXUNUSED(event) )
{
    if (m_Object  &&  m_Scope) {
        string str;
        CLabel::GetLabel(*m_Object, &str, CLabel::eDefault, m_Scope);
        m_Name = ToWxString(str);
        m_NameCtrl->SetValue(m_Name);
    }
}


void CFolderEditDlg::OnInitDialog( wxInitDialogEvent& event )
{
    CDialog::OnInitDialog(event);

    if(m_ReadOnly)    {
        m_NameCtrl->Enable(false);
        m_DescrCtrl->Enable(false);
    }

    if (m_ReadOnly || m_Scope == NULL || m_Object == NULL ||
        dynamic_cast<const objects::CSeq_annot*>(m_Object)) {
        m_RestoreBtn->Enable(false);
        m_RestoreBtn->Hide();
    }
}


void CFolderEditDlg::SetDataObject(const CSerialObject* object, objects::CScope* scope)
{
    m_Object = object;
    m_Scope = scope;
}


void CFolderEditDlg::SetNameValidator(IFNameValidator* validator)
{
    m_NameValidator = validator;
}



bool CFolderEditDlg::ShowToolTips()
{
    return true;
}


wxBitmap CFolderEditDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFolderEditDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFolderEditDlg bitmap retrieval
}


wxIcon CFolderEditDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFolderEditDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFolderEditDlg icon retrieval
}


END_NCBI_SCOPE
