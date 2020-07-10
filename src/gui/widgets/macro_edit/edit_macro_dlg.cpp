/*  $Id: edit_macro_dlg.cpp 45062 2020-05-19 19:26:03Z asztalos $
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
 * Authors: Roman Katargin, Anatoly Osipov
 */

#include <ncbi_pch.hpp>

#include <sstream>

#include <gui/widgets/macro_edit/edit_macro_dlg.hpp>
#include <gui/widgets/macro_edit/var_resolver_dlg.hpp>
#include <gui/widgets/data/macro_error_dlg.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/macro_ex.hpp>
#include <gui/objutils/macro_engine_parallel.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/utils.hpp>
#include <util/line_reader.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <chrono>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(macro);

/*!
 * CEditMacroDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditMacroDlg, CDialog )


/*!
 * CEditMacroDlg event table definition
 */

BEGIN_EVENT_TABLE( CEditMacroDlg, CDialog )

    EVT_BUTTON( ID_RUN, CEditMacroDlg::OnRunClick)
    EVT_BUTTON( ID_PRINT, CEditMacroDlg::OnPrintClick)
    EVT_BUTTON( ID_PRINTPARALLEL, CEditMacroDlg::OnPrintParallelClick)
    EVT_BUTTON( ID_AUTOFIXGB, CEditMacroDlg::LoadAutofix_GB)
    EVT_BUTTON( ID_AUTOFIXTSA, CEditMacroDlg::LoadAutofix_TSA)
    EVT_BUTTON( ID_AUTOFIXWGS, CEditMacroDlg::LoadAutofix_WGS)
    EVT_BUTTON( ID_OTHERMACROS, CEditMacroDlg::LoadOtherMacros)

    EVT_LISTBOX( ID_LISTBOX, CEditMacroDlg::OnListItemSelected)

END_EVENT_TABLE()


/*!
 * CEditMacroDlg constructors
 */

CEditMacroDlg::CEditMacroDlg()
{
    Init();
}

CEditMacroDlg::CEditMacroDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CEditMacroDlg creator
 */

bool CEditMacroDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditMacroDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditMacroDlg creation
    return true;
}


/*!
 * CEditMacroDlg destructor
 */

CEditMacroDlg::~CEditMacroDlg()
{
////@begin CEditMacroDlg destruction
////@end CEditMacroDlg destruction
}


/*!
 * Member initialisation
 */

void CEditMacroDlg::Init()
{
////@begin CEditMacroDlg member initialisation
    m_TextCtrl = NULL;
    m_MacroList = NULL;
////@end CEditMacroDlg member initialisation
    x_LoadMacroNames();
    SetRegistryPath( "Workbench.Dialogs.MacroEditor" );
}


/*!
 * Control creation for CEditMacroDlg
 */

void CEditMacroDlg::CreateControls()
{    
////@begin CEditMacroDlg content construction
    wxBoxSizer* bs_vertical = new wxBoxSizer(wxVERTICAL);
    SetSizer(bs_vertical);

    wxBoxSizer* bs_hor_top = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bs_hor_bottom = new wxBoxSizer(wxHORIZONTAL);

    bs_vertical->Add(bs_hor_top, 1, wxEXPAND | wxALL, 5);
    bs_vertical->Add(bs_hor_bottom, 0, wxEXPAND| wxALL, 5);

    // Top sizer, creating controls
    wxArrayString wxMacrolist;
    m_MacroList = new wxListBox( this, ID_LISTBOX, wxDefaultPosition, wxSize(-1, 250), wxMacrolist, wxLB_SINGLE );
    bs_hor_top->Add(m_MacroList, 0, wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP | wxALL, 5);

    m_TextCtrl = new wxTextCtrl( this, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(350, 250), wxTE_MULTILINE );
    bs_hor_top->Add(m_TextCtrl, 1, wxEXPAND | wxALL, 5);


    // Bottom sizer, creating buttons
    wxButton* Autofix_GB = new wxButton(this, ID_AUTOFIXGB, _("Autofix(GB)"), wxDefaultPosition, wxDefaultSize, 0);
    bs_hor_bottom->Add(Autofix_GB, 0, wxALIGN_LEFT | wxALL, 5);

    wxButton* Autofix_TSA = new wxButton(this, ID_AUTOFIXTSA, _("Autofix(TSA)"), wxDefaultPosition, wxDefaultSize, 0);
    bs_hor_bottom->Add(Autofix_TSA, 0, wxALIGN_LEFT | wxALL, 5);

    wxButton* Autofix_WGS = new wxButton(this, ID_AUTOFIXWGS, _("Autofix(WGS)"), wxDefaultPosition, wxDefaultSize, 0);
    bs_hor_bottom->Add(Autofix_WGS, 0, wxALIGN_LEFT | wxALL, 5);

    wxButton* Other = new wxButton(this, ID_OTHERMACROS, _("Other Macros"), wxDefaultPosition, wxDefaultSize, 0);
    bs_hor_bottom->Add(Other, 0, wxALIGN_LEFT | wxALL, 5);

    bs_hor_bottom->AddStretchSpacer();

    wxButton* RunButton = new wxButton( this, ID_RUN, _("Run"), wxDefaultPosition, wxDefaultSize, 0 );
    bs_hor_bottom->Add(RunButton, 0, wxALL, 5);

    wxButton* PrintButton = new wxButton(this, ID_PRINT, _("Print"), wxDefaultPosition, wxDefaultSize, 0);
    bs_hor_bottom->Add(PrintButton, 0, wxALL, 5);

    wxButton* PrintParallelButton = new wxButton(this, ID_PRINTPARALLEL, _("Parallel_Print"), wxDefaultPosition, wxDefaultSize, 0);
    bs_hor_bottom->Add(PrintParallelButton, 0, wxALL, 5);

    wxButton* CloseButton = new wxButton( this, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    bs_hor_bottom->Add(CloseButton, 0, wxALL, 5);

    for (auto& it : m_Macros) {
        if (NStr::FindNoCase(it, "Autofix") == NPOS) {
            m_MacroList->Append(ToWxString(it));
        }
    }

////@end CEditMacroDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CEditMacroDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditMacroDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditMacroDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditMacroDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditMacroDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditMacroDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditMacroDlg icon retrieval
}

void CEditMacroDlg::OnRunClick( wxCommandEvent& event )
{ 
    _ASSERT (m_TopSeqEntry && m_CmdProccessor);

    string macro_str = ToStdString(m_TextCtrl->GetValue());

    CMacroEngine macro_engine;
    CRef<CMacroRep> macro_rep(macro_engine.Parse(macro_str));
    if (!macro_engine.GetStatus()) {
        NcbiMessageBox(macro_engine.GetErrorMessage());
        return;
    }

    _ASSERT(macro_rep.GetPointerOrNull());
    NcbiInfoBox("Parsing succeeded.");

    bool bResolved = true;
    if (macro_rep->AreThereGUIResolvableVars()) {
        CVarResolverDlg dlg(*macro_rep, NULL);
        dlg.ShowModal();
        bResolved = dlg.GetStatus();
    }

    if (bResolved) {
        CMacroBioData bio_data(m_TopSeqEntry, m_SeqSubmit);
        CRef<CMacroCmdComposite> cmd(new CMacroCmdComposite("Executing one macro"));
        try {
            macro_engine.Exec(*macro_rep, bio_data, cmd, true);
            // Introduce Undo Manager or undo changes in case of error
            m_CmdProccessor->Execute(cmd);

            const CMacroStat::CMacroLog& report = macro_engine.GetStatistics().GetMacroReport();
            CNcbiOstrstream log;
            log << report.GetLog();
            string msg("Execution succeeded\n");
            string rep = CNcbiOstrstreamToString(log);
            if (NStr::IsBlank(rep)) {
                msg.append("Macro had no effect.");
            }
            else {
                msg.append(rep);
            }
            NcbiInfoBox(msg);
        }
        catch (const CException& err) {
            cmd->Execute(); // this call resets state to let Unexecute be run
            cmd->Unexecute();
            
            ERR_POST("Execution of macro " << macro_rep->GetName() << " has failed:\n" << err.ReportAll());

            string msg = "Execution of macro " + macro_rep->GetName() + " has failed:";

            CMacroErrorDlg errorDlg(this);
            errorDlg.SetException(msg, err);
            errorDlg.ShowModal();
        }
        
    } else {
        NcbiInfoBox("Execution canceled");
    }

    event.Skip();
} 

void CEditMacroDlg::OnPrintClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, wxT("Save CSV file"), wxEmptyString, wxEmptyString, 
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles), 
        wxFD_SAVE |wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;
   
    unique_ptr<CNcbiOstream> ostr;
    wxString path = dlg.GetPath();
    if (!path.IsEmpty()) {
        ostr.reset(new CNcbiOfstream(path.fn_str(), ios::out));
    }

    if (!ostr) {
        NcbiWarningBox("File is not accessible");
        return;
    }

    _ASSERT(m_TopSeqEntry && m_CmdProccessor);

    string macro_str = ToStdString(m_TextCtrl->GetValue());

    auto start = chrono::steady_clock::now();

    CMacroEngine macro_engine;
    CRef<CMacroRep> macro_rep(macro_engine.Parse(macro_str));
    if (!macro_engine.GetStatus()) {
        NcbiMessageBox(macro_engine.GetErrorMessage());
        return;
    }

    CMacroBioData bio_data(m_TopSeqEntry, m_SeqSubmit);
    CRef<CMacroCmdComposite> cmd(new CMacroCmdComposite("Executing one macro"));
    try {
        macro_engine.Exec(*macro_rep, bio_data, cmd, true, ostr.get());
        // Introduce Undo Manager or undo changes in case of error
        m_CmdProccessor->Execute(cmd);

        auto stop = chrono::steady_clock::now();
        chrono::duration<double> elapsed = stop - start;
        LOG_POST(Info << "Time to execute macro (normal): " << elapsed.count());
        NcbiMessageBox("Macro successfully executed");

    }
    catch (const CException& err) {
        cmd->Execute(); // this call resets state to let Unexecute be run
        cmd->Unexecute();

        ERR_POST("Execution of macro " << macro_rep->GetName() << " has failed:\n" << err.ReportAll());

        string msg = "Execution of macro " + macro_rep->GetName() + " has failed:";

        CMacroErrorDlg errorDlg(this);
        errorDlg.SetException(msg, err);
        errorDlg.ShowModal();
    }
}

void CEditMacroDlg::OnPrintParallelClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, wxT("Select file"), wxEmptyString, wxEmptyString,
        CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    unique_ptr<CNcbiOstream> ostr;
    wxString path = dlg.GetPath();
    if (!path.IsEmpty()) {
        ostr.reset(new CNcbiOfstream(path.fn_str(), ios::out));
    }

    if (!ostr) {
        NcbiWarningBox("File is not accessible");
        return;
    }

    _ASSERT(m_TopSeqEntry && m_CmdProccessor);

    string macro_str = ToStdString(m_TextCtrl->GetValue());

    auto start = chrono::steady_clock::now();

    CMacroEngineParallel macro_engine;
    CRef<CMacroRep> macro_rep(macro_engine.Parse(macro_str));
    if (!macro_engine.GetParsingStatus()) {
        NcbiMessageBox(macro_engine.GetParsingErrorMessage());
        return;
    }
   
    CMacroBioData bio_data(m_TopSeqEntry, m_SeqSubmit);
    CRef<CMacroCmdComposite> cmd(new CMacroCmdComposite("Executing one macro"));
    try {
        macro_engine.Exec(*macro_rep, bio_data, cmd, true, ostr.get());
        // Introduce Undo Manager or undo changes in case of error
        m_CmdProccessor->Execute(cmd);

        auto stop = chrono::steady_clock::now();
        chrono::duration<double> elapsed = stop - start;
        LOG_POST(Info << "Time to parallel execute macro: " << elapsed.count());
        NcbiMessageBox("Macro successfully executed");
    }
    catch (const CException& err) {
        cmd->Execute(); // this call resets state to let Unexecute be run
        cmd->Unexecute();

        ERR_POST("Execution of macro " << macro_rep->GetName() << " has failed:\n" << err.ReportAll());

        string msg = "Execution of macro " + macro_rep->GetName() + " has failed:";

        CMacroErrorDlg errorDlg(this);
        errorDlg.SetException(msg, err);
        errorDlg.ShowModal();
    }
}

void CEditMacroDlg::x_LoadMacroNames()
{
    m_Macros.clear();
    CMacroLib::GetInstance().GetMacroNames(m_Macros);
}

void CEditMacroDlg::OnListItemSelected( wxCommandEvent& event)
{
    string name = ToStdString(m_MacroList->GetStringSelection());
    if ( ! NStr::IsBlank(name) ) {
        CMacroRep* mr = CMacroLib::GetInstance().GetMacroRep(name);
        if (mr)
            m_TextCtrl->SetValue(ToWxString( mr->GetSource() ));
    }
}

void CEditMacroDlg::LoadAutofix_GB(wxCommandEvent& event)
{
    x_LoadAutofixMacro("AutofixGB");
}

void CEditMacroDlg::LoadAutofix_TSA(wxCommandEvent& event)
{
    x_LoadAutofixMacro("AutofixTSA");
}

void CEditMacroDlg::LoadAutofix_WGS(wxCommandEvent& event)
{
    x_LoadAutofixMacro("AutofixWGS");
}

void CEditMacroDlg::x_LoadAutofixMacro(const string& name)
{
    m_MacroList->Clear();
    for (auto& it : m_Macros) {
        if (NStr::FindNoCase(it, name) != NPOS) {
            m_MacroList->Append(ToWxString(it));
        }
    }
}

void CEditMacroDlg::LoadOtherMacros(wxCommandEvent& event)
{
    m_MacroList->Clear();
    for (auto& it : m_Macros) {
        if (NStr::FindNoCase(it, "Autofix") == NPOS) {
            m_MacroList->Append(ToWxString(it));
        }
    }
}

END_NCBI_SCOPE
