/*  $Id: scoring_methods_dlg.cpp 37492 2017-01-13 21:51:26Z shkeda $
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
 * Authors:  Andrey Yazhuk, Roman Katargin
 *
 * File Description:
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/aln_score/scoring_methods_dlg.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>
#include <gui/widgets/aln_score/properties_panel_dlg.hpp>

#include <gui/widgets/wx/ui_tool.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CScoringMethodsDlg, CDialog )

BEGIN_EVENT_TABLE( CScoringMethodsDlg, CDialog )

////@begin CScoringMethodsDlg event table entries
    EVT_LISTBOX( ID_LISTBOX1, CScoringMethodsDlg::OnMethodSelected )
    EVT_LISTBOX_DCLICK( ID_LISTBOX1, CScoringMethodsDlg::OnListbox1DoubleClicked )

    EVT_BUTTON( ID_BUTTON1, CScoringMethodsDlg::OnPropertiesClick )

    EVT_BUTTON( wxID_OK, CScoringMethodsDlg::OnOkClick )

////@end CScoringMethodsDlg event table entries

END_EVENT_TABLE()


CScoringMethodsDlg::CScoringMethodsDlg()
:  m_Type(IAlnExplorer::fDNA)
{
    Init();
}

CScoringMethodsDlg::CScoringMethodsDlg(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
:   m_Type(IAlnExplorer::fDNA)
{

    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CScoringMethodsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CScoringMethodsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CScoringMethodsDlg creation
    return true;
}


CScoringMethodsDlg::~CScoringMethodsDlg()
{
////@begin CScoringMethodsDlg destruction
////@end CScoringMethodsDlg destruction
}


void CScoringMethodsDlg::Setup( const string& sel_method, IAlnExplorer::EAlignType type)
{
    m_SelMethod = ToWxString(sel_method);
    m_Type = type;
}


void CScoringMethodsDlg::Init()
{
////@begin CScoringMethodsDlg member initialisation
////@end CScoringMethodsDlg member initialisation
}


class CScoringMethodSelector : public CUIToolRegistry::ISelector
{
public:
    virtual bool Select(const IUITool& tool)
    {
        return dynamic_cast<const IScoringMethod*>(&tool) != NULL;
    }
};


void CScoringMethodsDlg::CreateControls()
{
////@begin CScoringMethodsDlg content construction
    CScoringMethodsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Methods List"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 6, wxGROW|wxALL, 5);

    wxArrayString itemListBox5Strings;
    wxListBox* itemListBox5 = new wxListBox( itemCDialog1, ID_LISTBOX1, wxDefaultPosition, wxSize(-1, itemCDialog1->ConvertDialogToPixels(wxSize(-1, 110)).y), itemListBox5Strings, wxLB_SINGLE );
    itemStaticBoxSizer4->Add(itemListBox5, 1, wxGROW|wxALL, 1);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer6, 7, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Method Description"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizer7, 1, wxGROW|wxALL, 5);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
    itemStaticBoxSizer7->Add(itemTextCtrl8, 1, wxGROW|wxALL, 1);

    // Temporary removed until coloring methods settings become persistent
    //wxButton* itemButton9 = new wxButton( itemCDialog1, ID_BUTTON1, _("Properties"), wxDefaultPosition, wxDefaultSize, 0 );
    //itemBoxSizer6->Add(itemButton9, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT, 5);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemCDialog1, ID_STATICLINE, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine10, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_OK, _("&Select"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CScoringMethodsDlg content construction


    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);

    // obtain a collection of IScoringMethod tools from the Registry
    CUIToolRegistry* reg = CUIToolRegistry::GetInstance();

    vector<CConstIRef<IUITool> > tools;
    CScoringMethodSelector sel;
    reg->GetTools(tools, sel);

    // fill the List with the methods applicable to this type of alignment
    size_t i;
    for( i = 0; i < tools.size(); i++ )  {
        const IScoringMethod* method =
            dynamic_cast<const IScoringMethod*>(tools[i].GetPointer());
        _ASSERT(method);

        if(method  &&  method->GetType() & m_Type)  {
            wxString method_name = ToWxString(method->GetName());
            itemListBox->Append(method_name);
        }
    }

    for( i = 0; i < itemListBox->GetCount(); i++ )  {
        if (m_SelMethod == itemListBox->GetString((unsigned)i))
            break;
    }

    int index = (int)((i >= itemListBox->GetCount()) ? 0 : i);
    if (itemListBox->GetCount() > 0)
        itemListBox->SetSelection(index);

    x_UpdateDescription();
}


bool CScoringMethodsDlg::ShowToolTips()
{
    return true;
}


wxBitmap CScoringMethodsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CScoringMethodsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CScoringMethodsDlg bitmap retrieval
}


wxIcon CScoringMethodsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CScoringMethodsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CScoringMethodsDlg icon retrieval
}


void CScoringMethodsDlg::x_UpdateDescription()
{
    wxTextCtrl* itemTextCtrl = (wxTextCtrl*)FindWindow(ID_TEXTCTRL1);
    itemTextCtrl->Clear();

    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    wxString methodName = itemListBox->GetStringSelection();
    if (methodName.empty())
        return;

    *itemTextCtrl << ToWxString(CUIToolRegistry::GetInstance()->GetToolDescription(ToStdString(methodName)));
    itemTextCtrl->SetInsertionPoint(0);
}

void CScoringMethodsDlg::OnMethodSelected( wxCommandEvent& event )
{
    wxUnusedVar(event);
    x_UpdateDescription();
}

void CScoringMethodsDlg::OnPropertiesClick( wxCommandEvent& event )
{
    wxUnusedVar(event);

    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    wxString methodName = itemListBox->GetStringSelection();
    if (methodName.empty())
        return;

    CIRef<IUITool> tool = CUIToolRegistry::GetInstance()->CreateToolInstance(ToStdString(methodName));
    IUIToolWithGUI* method = tool ? dynamic_cast<IUIToolWithGUI*>(tool.GetPointer()) : 0;

    if (!method) {
        wxString msg = _("Failed to create instance of tool '") + methodName + _("'.");
        wxMessageBox(msg, _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    CPropertiesPanelDlg dlg(this, method);
    dlg.ShowModal();
}


void CScoringMethodsDlg::OnOkClick( wxCommandEvent& event )
{
    wxUnusedVar(event);

    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    m_SelMethod = itemListBox->GetStringSelection();
    EndModal(wxID_OK);
}


void CScoringMethodsDlg::OnListbox1DoubleClicked( wxCommandEvent& event )
{
    wxUnusedVar(event);

    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    m_SelMethod = itemListBox->GetStringSelection();
    EndModal(wxID_OK);
}


END_NCBI_SCOPE
