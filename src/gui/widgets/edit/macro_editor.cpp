/*  $Id: macro_editor.cpp 43774 2019-08-29 18:23:30Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/macrofloweditor.hpp>
#include <gui/widgets/edit/macro_edit_action_panel.hpp>
#include <gui/widgets/edit/macro_editor.hpp>

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/app.h>
#include <wx/evtloop.h>
#include <wx/notifmsg.h>
#include <wx/generic/notifmsg.h>
#include <wx/utils.h> 
#include <wx/display.h>
BEGIN_NCBI_SCOPE

/*!
 * CMacroEditor type definition
 */

 IMPLEMENT_CLASS(CMacroEditor, wxFrame)


/*!
 * CMacroEditor event table definition
 */
BEGIN_EVENT_TABLE(CMacroEditor, wxFrame)
    EVT_BUTTON( wxID_OK, CMacroEditor::OnOkClick)
    EVT_BUTTON( wxID_CANCEL, CMacroEditor::OnCloseClick)
END_EVENT_TABLE()


/*!
 * CMacroEditor constructors
 */

CMacroEditor::CMacroEditor()
{
    Init();
}

CMacroEditor::CMacroEditor( wxWindow* parent, int w, int h, int x, int y, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    :   m_width(w), m_height(h), m_pos_x(x), m_pos_y(y)
{
    Init();
    Create( parent, id, caption, pos, size, style );
    SetPositionAndSize();
    NMacroStats::ReportUsage(caption, "open");
}


/*!
 * CMacroEditor creator
 */

bool CMacroEditor::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroEditor creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
////@end CMacroEditor creation
    return true;
}


/*!
 * CMacroEditor destructor
 */

CMacroEditor::~CMacroEditor()
{
    wxWindow* w = this->GetParent();
    if (w) {
        CMacroFlowEditor* parent = dynamic_cast<CMacroFlowEditor*>(w);
        if (parent) {
            parent->SaveAddMacroSizeAndPosition(GetScreenRect().GetWidth(), GetScreenRect().GetHeight(), GetScreenPosition().x, GetScreenPosition().y);
            parent->RemoveEditor();
        }
    }
}

void CMacroEditor::SetPositionAndSize()
{
    if (m_width >= 0  && m_height >= 0)
        SetSize(wxSize(m_width, m_height));
    
    int width = GetScreenRect().GetWidth();
    int height = GetScreenRect().GetHeight();
    if (m_pos_x >= 0  && m_pos_y >= 0)
    {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
           wxDisplay display(i);
           max_x += display.GetGeometry().GetWidth();
       }
       if (m_pos_x + width > max_x) m_pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (m_pos_y + height > wxGetDisplaySize().GetHeight()) m_pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(m_pos_x, m_pos_y));
    }
}

/*!
 * Member initialisation
 */

void CMacroEditor::Init()
{
////@begin CMacroEditor member initialisation
    m_ActionPanel = NULL;
    m_AddToScript = NULL;
    m_Status = NULL;
    m_MacroRep.Reset();
////@end CMacroEditor member initialisation
}


/*!
 * Control creation for CMacroEditor
 */

void CMacroEditor::CreateControls()
{    
////@begin CMacroEditor content construction
    CMacroEditor* itemFrame1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemFrame1, wxID_ANY);
    itemBoxSizer2->Add(itemPanel3, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    m_ActionPanel = new CMacroEditingActionPanel(itemPanel3, ID_MACROACTIONPANEL, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    itemBoxSizer4->Add(m_ActionPanel, 0, wxEXPAND|wxALL, 0);
    m_ActionPanel->SetParentFrame(this);
    
    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel3, wxID_ANY, _("Validation"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxHORIZONTAL);
    itemBoxSizer4->Add(itemStaticBoxSizer6, 0, wxEXPAND | wxALL, 5);

    m_Status = new wxStaticText(itemPanel3, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer6->Add(m_Status, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer12, 0, wxEXPAND | wxALL, 0);

    itemBoxSizer12->AddStretchSpacer();

    m_AddToScript = new wxButton(itemPanel3, wxID_OK, _("Add to script"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(m_AddToScript, 0, wxALL, 5);
    m_AddToScript->Disable();

    wxButton* itemButton15 = new wxButton( itemPanel3, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALL, 5);

////@end CMacroEditor content construction
}


/*!
 * Should we show tooltips?
 */

bool CMacroEditor::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMacroEditor::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroEditor bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroEditor bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMacroEditor::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroEditor icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroEditor icon retrieval
}

void CMacroEditor::Update()
{
    string macro_script;
    try {
        macro_script = m_ActionPanel->GetMacro();
#ifdef _DEBUG
        LOG_POST(Info << macro_script);
#endif 
    }
    catch (const CException& e) {
        NcbiMessageBox(e.GetMsg());
    }

    m_MacroRep.Reset(m_MEngine.Parse(macro_script));
    if (!m_MEngine.GetStatus()) {
        m_Status->SetForegroundColour(wxColor(*wxRED));
        m_Status->SetLabel(ToWxString("Invalid script"));
        LOG_POST(Info << "Parsing did not succeed: " << m_MEngine.GetErrorMessage());
        if (m_AddToScript->IsEnabled()) {
            m_AddToScript->Disable();
        }
    }
    else {
        if (m_ActionPanel->GetWarning().empty()) {
            m_Status->SetForegroundColour(wxColour(0, 128, 0));
            m_Status->SetLabel(ToWxString("Valid script"));
            m_AddToScript->Enable();
        }
        else {
            m_Status->SetForegroundColour(wxColour(255, 165, 0));
            m_Status->SetLabel(ToWxString("Valid script - " + m_ActionPanel->GetWarning()));
            m_AddToScript->Disable();
        }
    }
}

void CMacroEditor::Clear()
{
    m_Status->SetLabel(wxEmptyString);
    m_AddToScript->Disable();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void CMacroEditor::OnOkClick(wxCommandEvent& event)
{
    if (m_MacroRep) {
        wxWindow* w = this->GetParent();
        while (w != NULL) {
            CMacroFlowEditor* parent = dynamic_cast<CMacroFlowEditor*>(w);
            if (parent) {
                parent->AddNewMacro(m_MacroRep);
                NMacroStats::ReportUsage(this->GetLabel(), "add new macro");
                NMacroStats::SaveScriptCreated(m_MacroRep->GetSource());
                DisplayNotice();
                return;
            }
            else {
                w = w->GetParent();
            }
        }
    }
    else {
        NCBI_THROW(CException, eUnknown, "No macro script was generated");
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void CMacroEditor::OnCloseClick(wxCommandEvent& event)
{
    Destroy();
}

void CMacroEditor::DisplayNotice()
{
    HideWithEffect(wxSHOW_EFFECT_BLEND);
    if (wxTheApp && wxTheApp->GetMainLoop())
    {
	if (!wxTheApp->GetMainLoop()->IsYielding())
	    wxTheApp->Yield(true);
	wxTheApp->ProcessPendingEvents();    
    }
    ShowWithEffect(wxSHOW_EFFECT_BLEND);
#ifdef __WXMSW__
    Refresh();
    Update();
#endif
    wxGenericNotificationMessage notice(wxEmptyString, wxT("Macro added"));
    wxWindow* w = GetParent();
    if (w) 
    {
        CMacroFlowEditor* frame = dynamic_cast<CMacroFlowEditor*>(w);
	wxIcon icon;
	icon.CopyFromBitmap(frame->GetPlusBitmap());
	notice.SetIcon(icon);
    }
    notice.Show();  
}

END_NCBI_SCOPE

