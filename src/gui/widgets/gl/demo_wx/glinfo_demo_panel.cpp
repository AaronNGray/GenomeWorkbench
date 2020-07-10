/*  $Id: glinfo_demo_panel.cpp 41267 2018-06-26 17:02:49Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include <gui/widgets/wx/glcanvas.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/feedback/dump_sysinfo.hpp>

#include "glinfo_demo_panel.hpp"

BEGIN_NCBI_SCOPE

#define ID_SAVE_BTN wxID_HIGHEST + 1
#define ID_TEXT_CTRL wxID_HIGHEST + 2

BEGIN_EVENT_TABLE(CGlInfoDemoPanel, wxPanel)
    EVT_BUTTON(ID_SAVE_BTN, CGlInfoDemoPanel::OnButtonSave)
END_EVENT_TABLE()

CGlInfoDemoPanel::CGlInfoDemoPanel()
{
}

CGlInfoDemoPanel::CGlInfoDemoPanel(wxWindow* parent, wxWindowID id)
{
    Create(parent, id);
}

bool CGlInfoDemoPanel::Create(wxWindow* parent, wxWindowID id)
{
    wxPanel::Create(parent, id, wxDefaultPosition, wxSize(0,0), 0);
    CreateControls();
    return true;
}

class CGlInfoWindow : public CGLCanvas
{
public:
    CGlInfoWindow(wxWindow* parent, wxWindowID id, wxTextCtrl* textCtrl) :
      CGLCanvas(parent, id, wxDefaultPosition, wxSize(1,16)), m_TextCtrl(textCtrl) {}
protected:
    virtual void x_Render();
    wxTextCtrl* m_TextCtrl;
};

void CGlInfoWindow::x_Render()
{
    CGLCanvas::x_Render();

    if (m_TextCtrl) {
        CNcbiOstrstream ostr;
        CDumpSysInfo::DumpSystemInfo(ostr);
        string str = CNcbiOstrstreamToString(ostr);
        m_TextCtrl->AppendText(ToWxString(str));
        m_TextCtrl->SetInsertionPoint(0);
        m_TextCtrl = NULL;
    }
}

void CGlInfoDemoPanel::CreateControls()
{
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *barSizer = new wxBoxSizer(wxHORIZONTAL);

    topSizer->Add(barSizer, 0, wxALIGN_RIGHT);

    wxTextCtrl *textCtrl= new wxTextCtrl(this, ID_TEXT_CTRL, wxEmptyString,
                                         wxDefaultPosition, wxDefaultSize,
                                         wxTE_READONLY|wxTE_MULTILINE);

    wxFont fixedFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    textCtrl->SetFont(fixedFont);

    topSizer->Add(textCtrl, 1, wxEXPAND);

    barSizer->Add(new CGlInfoWindow(this, wxID_ANY, textCtrl), 0, wxALIGN_CENTER_VERTICAL);
    barSizer->Add(new wxButton(this, ID_SAVE_BTN, wxT("Save")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    SetSizer(topSizer);
}

void CGlInfoDemoPanel::OnButtonSave(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, wxT("Save OpenGL Info"), wxEmptyString, wxEmptyString,
                        wxT("Text files (*.txt)|*.txt|All files (*.*)|*.*"),
                        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() != wxID_OK)
        return;

    wxTextCtrl *textCtrl= (wxTextCtrl*)FindWindow(ID_TEXT_CTRL);
    wxString str = textCtrl->GetValue();
    CNcbiOfstream ostr(dialog.GetPath().fn_str(), ios::out|ios::trunc);
    if (ostr) {
        ostr << str.ToUTF8() << endl;
        ostr.close();
    }

    if (ostr.fail()) {
        wxMessageDialog msgDialog(this, wxT("Failed to save file: ") + dialog.GetFilename(),
                                 wxT("Error"), wxICON_ERROR|wxOK);
        msgDialog.ShowModal();
    }
}

END_NCBI_SCOPE

