/*  $Id: score_methods_dlg.cpp 27635 2013-03-15 19:33:00Z katargir@NCBI.NLM.NIH.GOV $
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

////@begin includes
////@end includes

#include "score_methods_dlg.hpp"

#include <gui/widgets/wx/ui_tool.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/aln_score/scoring_method.hpp>
#include <gui/widgets/aln_score/template_methods.hpp>
#include <gui/widgets/aln_score/simple_methods.hpp>
#include <gui/widgets/aln_score/quality_methods.hpp>
#include <gui/widgets/aln_score/scoring_methods_dlg.hpp>
#include <gui/widgets/aln_score/properties_panel_dlg.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


/*!
 * CScoreMethodsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CScoreMethodsDlg, wxDialog )


/*!
 * CScoreMethodsDlg event table definition
 */

BEGIN_EVENT_TABLE( CScoreMethodsDlg, wxDialog )

////@begin CScoreMethodsDlg event table entries
    EVT_CHOICE( ID_CHOICE1, CScoreMethodsDlg::OnChoice1Selected )

    EVT_LISTBOX( ID_LISTBOX1, CScoreMethodsDlg::OnListbox1Selected )

    EVT_BUTTON( ID_BUTTON2, CScoreMethodsDlg::OnButton2Click )

    EVT_BUTTON( ID_BUTTON1, CScoreMethodsDlg::OnButton1Click )

////@end CScoreMethodsDlg event table entries

END_EVENT_TABLE()


/*!
 * CScoreMethodsDlg constructors
 */

CScoreMethodsDlg::CScoreMethodsDlg()
{
    Init();
}

CScoreMethodsDlg::CScoreMethodsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CScoreMethodsDlg creator
 */

bool CScoreMethodsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CScoreMethodsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CScoreMethodsDlg creation
    return true;
}


/*!
 * CScoreMethodsDlg destructor
 */

CScoreMethodsDlg::~CScoreMethodsDlg()
{
////@begin CScoreMethodsDlg destruction
////@end CScoreMethodsDlg destruction
}


/*!
 * Member initialisation
 */

static const wxChar*  kDefUserScoreDir = wxT("<home>/align_scores");
static const wxChar*  kDefScoreDir = wxT("<std>/etc/align_scores");
static const char*  kUIToolsRegKey = "GBENCH.UITools";

class CScoringMethodSelector : public CUIToolRegistry::ISelector
{
public:
    virtual bool Select(const IUITool& tool)
    {
        return dynamic_cast<const IScoringMethod*>(&tool) != NULL;
    }
};

void CScoreMethodsDlg::Init()
{
////@begin CScoreMethodsDlg member initialisation
////@end CScoreMethodsDlg member initialisation



// Withgout this the metods won't be registered in static build
    CColorTableMethod();
    CSimpleScoringMethod();
    CQualityScoringMethodNA();

    list<wxString> dirs;
    dirs.push_back(kDefUserScoreDir);
    dirs.push_back(kDefScoreDir);

    CUIToolRegistry* reg = CUIToolRegistry::GetInstance();
    reg->LoadTemplateToolsInfo(dirs);

    reg->SetRegistryPath(kUIToolsRegKey);
    reg->LoadSettings();

    vector<CConstIRef<IUITool> > tools;
    CScoringMethodSelector sel;
    CUIToolRegistry::GetInstance()->GetTools(tools, sel);

    for( size_t i = 0; i < tools.size(); i++ )  {
        const IScoringMethod* method =
            dynamic_cast<const IScoringMethod*>(tools[i].GetPointer());
        _ASSERT(method);
        if(method) {
            CIRef<IUITool> tool = reg->CreateToolInstance(method->GetName());
            if (!tool)
                return;

            IScoringMethod* toolMethod = dynamic_cast<IScoringMethod*>(tool.GetPointer());
            if (toolMethod)
                m_Tools.push_back(CIRef<IScoringMethod>(toolMethod));
        }
    }
}

/*!
 * Control creation for CScoreMethodsDlg
 */

void CScoreMethodsDlg::CreateControls()
{
////@begin CScoreMethodsDlg content construction
    CScoreMethodsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Methods:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice7Strings;
    itemChoice7Strings.Add(_("DNA"));
    itemChoice7Strings.Add(_("Protein"));
    itemChoice7Strings.Add(_("Mixed"));
    itemChoice7Strings.Add(_("Homogenous"));
    wxChoice* itemChoice7 = new wxChoice( itemDialog1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, itemChoice7Strings, 0 );
    itemChoice7->SetStringSelection(_("DNA"));
    itemBoxSizer3->Add(itemChoice7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemListBox8Strings;
    wxListBox* itemListBox8 = new wxListBox( itemDialog1, ID_LISTBOX1, wxDefaultPosition, wxSize(250, 200), itemListBox8Strings, wxLB_SINGLE );
    itemBoxSizer2->Add(itemListBox8, 1, wxGROW|wxLEFT|wxRIGHT, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(-1, 80), wxTE_MULTILINE|wxTE_READONLY );
    itemBoxSizer2->Add(itemTextCtrl9, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemDialog1, ID_STATICLINE, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine10, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, ID_BUTTON2, _("Select Method Dlg..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_BUTTON1, _("Properties"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CScoreMethodsDlg content construction

    x_LoadTools();
}


/*!
 * Should we show tooltips?
 */

bool CScoreMethodsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CScoreMethodsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CScoreMethodsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CScoreMethodsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CScoreMethodsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CScoreMethodsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CScoreMethodsDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void CScoreMethodsDlg::OnButton1Click( wxCommandEvent& WXUNUSED(event) )
{
    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    string methodName = ToStdString(itemListBox->GetStringSelection());
    if (methodName.empty())
        return;

    IAlnExplorer::EAlignType type = x_GetSelectedType();
    if (type == IAlnExplorer::fInvalid)
        return;

    IUIToolWithGUI* method = 0;
    NON_CONST_ITERATE(vector<CIRef<IScoringMethod> >, it, m_Tools) {
        if ((*it)->GetType() & type && (*it)->GetName() == methodName) {
            method = dynamic_cast<IUIToolWithGUI*>(it->GetPointer());
            break;
        }
    }

    if (method) {
        CPropertiesPanelDlg dlg(this, method);
        dlg.ShowModal();
    }
}

void CScoreMethodsDlg::x_LoadTools()
{
    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    itemListBox->Clear();


    IAlnExplorer::EAlignType type = x_GetSelectedType();
    if (type == IAlnExplorer::fInvalid)
        return;

    ITERATE(vector<CIRef<IScoringMethod> >, it, m_Tools) {
        if ((*it)->GetType() & type) {
            itemListBox->Append(ToWxString((*it)->GetName()));
        }
    }

    if (itemListBox->GetCount() > 0) itemListBox->SetSelection(0);

    x_UpdateDescription();
}

void CScoreMethodsDlg::x_UpdateDescription()
{
    wxTextCtrl* itemTextCtrl9 = (wxTextCtrl*)FindWindow(ID_TEXTCTRL1);
    itemTextCtrl9->Clear();

    wxListBox* itemListBox = (wxListBox*)FindWindow(ID_LISTBOX1);
    string methodName = ToStdString(itemListBox->GetStringSelection());
    if (methodName.empty())
        return;

    *itemTextCtrl9 << ToWxString(CUIToolRegistry::GetInstance()->GetToolDescription(methodName));
    itemTextCtrl9->SetInsertionPoint(0);
}

IAlnExplorer::EAlignType CScoreMethodsDlg::x_GetSelectedType()
{
    wxChoice* itemChoice3 = (wxChoice*)FindWindow(ID_CHOICE1);
    int index = itemChoice3->GetSelection();
    IAlnExplorer::EAlignType type = IAlnExplorer::fInvalid;

    switch(index) {
    case 0 :
        type = IAlnExplorer::fDNA;
        break;
    case 1 :
        type = IAlnExplorer::fProtein;
        break;
    case 2 :
        type = IAlnExplorer::fMixed;
        break;
    case 3 :
        type = IAlnExplorer::fHomogenous;
        break;
    }

    return type;
}

/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
 */

void CScoreMethodsDlg::OnChoice1Selected( wxCommandEvent& WXUNUSED(event) )
{
    x_LoadTools();
}

/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX1
 */

void CScoreMethodsDlg::OnListbox1Selected( wxCommandEvent& WXUNUSED(event) )
{
    x_UpdateDescription();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void CScoreMethodsDlg::OnButton2Click( wxCommandEvent& WXUNUSED(event) )
{
    /// FIXME: don't use a static here!!!!
    static string selectedMethod;

    IAlnExplorer::EAlignType type = x_GetSelectedType();
    if (type == IAlnExplorer::fInvalid)
        return;

    CScoringMethodsDlg dlg;
    dlg.Setup(selectedMethod, type);
    dlg.Create(this);

    if (dlg.ShowModal() == wxID_OK) {
        selectedMethod = ToStdString(dlg.GetSelectedMethod());
    }
}

END_NCBI_SCOPE
