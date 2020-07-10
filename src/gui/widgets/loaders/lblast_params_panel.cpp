/*  $Id: lblast_params_panel.cpp 42112 2018-12-21 14:33:31Z asztalos $
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
 * Authors:  
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/lblast_params_panel.hpp>
#include <gui/widgets/loaders/lblast_object_loader.hpp>

#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/bmpbuttn.h>

#include <wx/msgdlg.h> 
#include <wx/filedlg.h>
#include <wx/filefn.h> 
#include <wx/filename.h> 
#include <wx/dir.h>
#include <wx/artprov.h>

BEGIN_NCBI_SCOPE

/*!
 * CLBLASTParamsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLBLASTParamsPanel, wxPanel )


/*!
 * CLBLASTParamsPanel event table definition
 */

BEGIN_EVENT_TABLE( CLBLASTParamsPanel, wxPanel )

////@begin CLBLASTParamsPanel event table entries
    EVT_RADIOBOX( ID_RADIOBOX1, CLBLASTParamsPanel::OnSeqTypeSelected )

    EVT_TEXT( ID_TEXTCTRL2, CLBLASTParamsPanel::OnDBTextUpdated )

    EVT_BUTTON( ID_BITMAPBUTTON, CLBLASTParamsPanel::OnSelectDir )

////@end CLBLASTParamsPanel event table entries

    EVT_TIMER(-1, CLBLASTParamsPanel::OnTimer)

END_EVENT_TABLE()


/*!
 * CLBLASTParamsPanel constructors
 */

CLBLASTParamsPanel::CLBLASTParamsPanel()
    : m_Timer(this)
{
    Init();
}

CLBLASTParamsPanel::CLBLASTParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Timer(this)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CLBLASTParamsPanel creator
 */

bool CLBLASTParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLBLASTParamsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLBLASTParamsPanel creation
    return true;
}


/*!
 * CLBLASTParamsPanel destructor
 */

CLBLASTParamsPanel::~CLBLASTParamsPanel()
{
////@begin CLBLASTParamsPanel destruction
////@end CLBLASTParamsPanel destruction
}


/*!
 * Member initialisation
 */

void CLBLASTParamsPanel::Init()
{
////@begin CLBLASTParamsPanel member initialisation
    m_SeqTypeCtrl = NULL;
    m_DBCtrl = NULL;
    m_SeqCountSizer = NULL;
    m_CreateProjectItemsCtrl = NULL;
////@end CLBLASTParamsPanel member initialisation
}


/*!
 * Control creation for CLBLASTParamsPanel
 */

void CLBLASTParamsPanel::CreateControls()
{    
////@begin CLBLASTParamsPanel content construction
    CLBLASTParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_SeqTypeCtrlStrings;
    m_SeqTypeCtrlStrings.Add(_("&Nucleotides"));
    m_SeqTypeCtrlStrings.Add(_("&Proteins"));
    m_SeqTypeCtrl = new wxRadioBox( itemPanel1, ID_RADIOBOX1, _("Sequence Types"), wxDefaultPosition, wxDefaultSize, m_SeqTypeCtrlStrings, 1, wxRA_SPECIFY_ROWS );
    m_SeqTypeCtrl->SetSelection(0);
    itemBoxSizer2->Add(m_SeqTypeCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Local BLAST DB:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DBCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_DBCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton7 = new wxBitmapButton( itemPanel1, ID_BITMAPBUTTON, itemPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton7->SetHelpText(_("Select Local DB"));
    if (CLBLASTParamsPanel::ShowToolTips())
        itemBitmapButton7->SetToolTip(_("Select Local DB"));
    itemBoxSizer4->Add(itemBitmapButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SeqCountSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_SeqCountSizer, 0, wxGROW|wxALL, 5);

    m_CreateProjectItemsCtrl = new wxCheckBox( itemPanel1, ID_CHECKBOX7, _("Create project items for sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CreateProjectItemsCtrl->SetValue(false);
    m_SeqCountSizer->Add(m_CreateProjectItemsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_CreateProjectItemsCtrl->SetValidator( wxGenericValidator(& GetData().m_CreateSeqs) );
////@end CLBLASTParamsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CLBLASTParamsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLBLASTParamsPanel::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CLBLASTParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLBLASTParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLBLASTParamsPanel icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

bool CLBLASTParamsPanel::SelectDatabase(wxWindow* parent, wxString& db, bool proteins)
{
#ifndef __WXMAC__    
    wxString filter = proteins ?
        wxT("Protein db files (*.p\?\?)|*.p\?\?") :
        wxT("Nucleotide db files (*.n\?\?)|*.n\?\?");
#else
    wxString filter;
#endif

    wxFileDialog dlg(parent, wxT("Select a Local BLAST DB"), wxEmptyString, db,
        filter,
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
        wxFileName fn(dlg.GetPath());
        wxString db_prefix = proteins ? wxT("p") : wxT("n");
        if (!fn.GetExt().StartsWith(db_prefix)) {
            wxString msg("Please select any of the local BLAST DB files whose extensions start with ");
            if (proteins) {
                msg += wxT("'p'");
            }
            else {
                msg += wxT("'n'");
            }
            wxMessageBox(msg, wxT("Error"), wxOK | wxICON_ERROR);
            return false;
        }
        fn.SetExt(wxEmptyString);
        db = fn.GetFullPath();
        return true;
    }
    return false;
}


void CLBLASTParamsPanel::OnSelectDir( wxCommandEvent& event )
{
    wxString db = m_DBCtrl->GetValue();
    bool proteins = (m_SeqTypeCtrl->GetSelection() == 1);

    if (SelectDatabase(this, db, proteins)) {
        if (proteins)
            GetData().SetProtDB(db);
        else
            GetData().SetNucDB(db);

        m_DBCtrl->SetValue(db);
    }
}

/*!
 * Transfer data to the window
 */

bool CLBLASTParamsPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    bool proteins = GetData().GetProtSeqs();

    m_SeqTypeCtrl->SetSelection(proteins ? 1 : 0);
    wxString db = proteins ? GetData().GetProtDB() : GetData().GetNucDB();
    m_DBCtrl->SetValue(db);

    return wxPanel::TransferDataToWindow();
}

/*!
 * Transfer data from the window
 */

bool CLBLASTParamsPanel::ValidateLocalDatabase(const wxString& db, bool proteins)
{
    wxFileName fn(db);
    wxDir dir(fn.GetPath());
    if (!dir.IsOpened())
        return false;
    wxString ext = proteins ? wxT(".p??") : wxT(".n??");

    wxString tmp;
    bool cont = dir.GetFirst(&tmp, fn.GetFullName() + ext, wxDIR_FILES);
    while (cont) {
        wxFileName file(tmp);
        if (file.GetExt() != wxT("prt"))
            return true;
        cont = dir.GetNext(&tmp);
    }

    return false;
}

bool CLBLASTParamsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    wxString db = m_DBCtrl->GetValue();

    if (db.empty()) {
        wxMessageBox(wxT("Please, select a Local BLAST DB."), wxT("Error"),
            wxOK | wxICON_ERROR, this);
        m_DBCtrl->SetFocus();
        return false;
    }

    bool proteins = GetData().GetProtSeqs();

    if (!ValidateLocalDatabase(db, proteins)) {
        wxMessageBox(wxT("Invalid Local BLAST DB."), wxT("Error"),
            wxOK | wxICON_ERROR, this);
        m_DBCtrl->SetFocus();
        return false;
    }

    if (proteins)
        GetData().SetProtDB(db);
    else
        GetData().SetNucDB(db);

    return true;
}

void CLBLASTParamsPanel::OnSeqTypeSelected( wxCommandEvent& event )
{
    bool proteins = (event.GetInt() == 1);
    GetData().SetProtSeqs(proteins);

    wxString db = proteins ? GetData().GetProtDB() : GetData().GetNucDB();
    m_DBCtrl->SetValue(db);
}

void CLBLASTParamsPanel::x_UpdateSeqCount()
{
    bool proteins = GetData().GetProtSeqs();
    wxString dbText = m_DBCtrl->GetValue();
    wxBusyCursor wait;

    int seqCount = 0;

    if (!dbText.empty() && ValidateLocalDatabase(dbText, proteins)) {
        string db(dbText.ToUTF8());
        seqCount = CLBLASTObjectLoader::CountSeqs(db, proteins);
    }

    wxString text;
    if (seqCount > 0)
        text = wxString::Format(wxT("Create project items for %d sequences"), seqCount);
    else
        text = wxT("Create project items for sequences");

    m_CreateProjectItemsCtrl->SetLabelText(text);
    m_SeqCountSizer->Layout();
}

void CLBLASTParamsPanel::OnDBTextUpdated( wxCommandEvent& )
{
    m_Timer.StartOnce(500);
}

void CLBLASTParamsPanel::OnTimer(wxTimerEvent& event)
{
    x_UpdateSeqCount();
}

END_NCBI_SCOPE
