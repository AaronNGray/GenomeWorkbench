/*  $Id: wx_save_pdf_base_dlg.cpp 44349 2019-12-03 20:58:38Z katargir $
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
 * Authors: Liangshou Wu, Bob Falk
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/wx/wx_save_pdf_base_dlg.hpp>

#include <gui/widgets/wx/regexp_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/objutils/registry.hpp>
#include <corelib/ncbifile.hpp>

#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/panel.h>
#include <wx/artprov.h>

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE
/*!
 * CwxSavePdfBaseDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CwxSavePdfBaseDlg, wxDialog )


/*!
 * CwxSavePdfBaseDlg event table definition
 */

BEGIN_EVENT_TABLE( CwxSavePdfBaseDlg, wxDialog )

////@begin CwxSavePdfBaseDlg event table entries
    EVT_TEXT( ID_TEXTCTRL5, CwxSavePdfBaseDlg::OnTextctrl5TextUpdated )

    EVT_BUTTON( ID_BITMAPBUTTON1, CwxSavePdfBaseDlg::OnFilepathClick )

    EVT_CHECKBOX( ID_TOOLTIP_CHECKBOX, CwxSavePdfBaseDlg::OnTooltipCheckboxClick )

    EVT_BUTTON( ID_SAVE, CwxSavePdfBaseDlg::OnSaveClick )

    EVT_BUTTON( ID_OPEN, CwxSavePdfBaseDlg::OnOpenClick )

    EVT_BUTTON( ID_CANCEL, CwxSavePdfBaseDlg::OnCancelClick )

////@end CwxSavePdfBaseDlg event table entries

END_EVENT_TABLE()


/*!
 * CwxSavePdfBaseDlg constructors
 */

CwxSavePdfBaseDlg::CwxSavePdfBaseDlg()
{
    Init();
}

CwxSavePdfBaseDlg::CwxSavePdfBaseDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CwxSavePdfBaseDlg creator
 */

bool CwxSavePdfBaseDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxSavePdfBaseDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxSavePdfBaseDlg creation
    return true;
}


/*!
 * CwxSavePdfBaseDlg destructor
 */

CwxSavePdfBaseDlg::~CwxSavePdfBaseDlg()
{
////@begin CwxSavePdfBaseDlg destruction
////@end CwxSavePdfBaseDlg destruction
}


/*!
 * Member initialisation
 */

void CwxSavePdfBaseDlg::Init()
{
////@begin CwxSavePdfBaseDlg member initialisation
    m_Panel = NULL;
    m_FileNameBox = NULL;
    m_SaveOptionsSizer = NULL;
    m_IncludeToolTips = NULL;
    m_InfoSizer = NULL;
    m_Info = NULL;
    m_SaveButton = NULL;
    m_OpenButton = NULL;
////@end CwxSavePdfBaseDlg member initialisation
    m_ReadyToSave = false;
    m_OpenPdf = false;
    m_UserSelectedFilename = false;
}


/*!
 * Control creation for CwxSavePdfBaseDlg
 */

void CwxSavePdfBaseDlg::CreateControls()
{    
////@begin CwxSavePdfBaseDlg content construction
    CwxSavePdfBaseDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Panel = new wxPanel( itemDialog1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_Panel, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    m_Panel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( m_Panel, wxID_STATIC, _("Save As:"), wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileNameBox = new wxTextCtrl( m_Panel, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer5->Add(m_FileNameBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton8 = new wxBitmapButton( m_Panel, ID_BITMAPBUTTON1, itemDialog1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton8->SetHelpText(_("Select File"));
    if (CwxSavePdfBaseDlg::ShowToolTips())
        itemBitmapButton8->SetToolTip(_("Select File"));
    itemBoxSizer5->Add(itemBitmapButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SaveOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(m_SaveOptionsSizer, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( m_Panel, wxID_STATIC, _("                "), wxDefaultPosition, wxDefaultSize, 0 );
    m_SaveOptionsSizer->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IncludeToolTips = new wxCheckBox( m_Panel, ID_TOOLTIP_CHECKBOX, _("Include Tool Tips"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IncludeToolTips->SetValue(false);
    m_SaveOptionsSizer->Add(m_IncludeToolTips, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_InfoSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_InfoSizer, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_Info = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_InfoSizer->Add(m_Info, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);

    itemBoxSizer2->Add(5, 1, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_SaveButton = new wxButton( itemDialog1, ID_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_SaveButton, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

    m_OpenButton = new wxButton( itemDialog1, ID_OPEN, _("Save && Open"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_OpenButton, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

    wxButton* itemButton18 = new wxButton( itemDialog1, ID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton18, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

////@end CwxSavePdfBaseDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CwxSavePdfBaseDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxSavePdfBaseDlg::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CwxSavePdfBaseDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxSavePdfBaseDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxSavePdfBaseDlg icon retrieval
}

static const string kPdfPathKey = "PdfPath";

void CwxSavePdfBaseDlg::LoadSettings(const string& pdf_base_key)
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(pdf_base_key);
    string file_path = view.GetString(kPdfPathKey, "");

    wxFileName file(ToWxString(file_path));
    wxString filename = file.GetFullName();
    wxString path = file.GetPath();

    if ( !path.empty()  && wxFileName::DirExists(path) ) {
        m_Path = file_path;
    } else {
        // create a default path under user folder
        m_Path = ToStdString(wxGetHomeDir());
    }
}


void CwxSavePdfBaseDlg::SaveSettings(const string& pdf_base_key) const
{
    if ( !m_Path.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(pdf_base_key);
        view.Set(kPdfPathKey, m_Path);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FILEPATH
 */

void CwxSavePdfBaseDlg::OnFilepathClick( wxCommandEvent& event )
{
    string full_name = ToAsciiStdString(m_FileNameBox->GetValue());

    // If full_name is just a directory
    string path = full_name;
    string fname;    

    CFile f(ToAsciiStdString(m_FileNameBox->GetValue()));
    // If full_name is a valid directory and (existing) file
    if (f.IsFile()) {
        path = f.GetDir();
        fname = f.GetName();
    }
    else if (!f.IsDir()) {        
        string d = f.GetDir();
        CDir dir(d);
        // if file_name is valid direcotry and not-yet-created file:
        if (dir.IsDir()) {
            path = f.GetDir();
            fname = f.GetName();
        }
    }

    wxFileDialog dlg(this, wxT("Set File Name"),
        ToWxString(path), ToWxString(fname),
        wxT("*.pdf"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString filename = dlg.GetPath();
    m_FileNameBox->SetValue(filename);
    m_FileName = dlg.GetFilename();
    m_Path = dlg.GetDirectory();
    m_UserSelectedFilename = true;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SAVE
 */

void CwxSavePdfBaseDlg::OnSaveClick( wxCommandEvent& event )
{
    // validate file path
    wxFileName file(m_FileNameBox->GetValue());
    wxString filename = file.GetFullName();
    wxString n = filename.Lower();
    size_t pos = n.rfind(wxT(".pdf"));
    if (pos != string::npos) {
        filename = filename.SubString(0, pos-1);
    }
    wxString path = file.GetPath();

    if (path.IsEmpty()) {
        NcbiMessageBox("A directory must be specified!");
        return;
    }
    else if (!wxFileName::DirExists(path) ) {
        NcbiMessageBox("The given directory doesn't exist!");
        return;
    }  
    else if (filename.IsEmpty()) {
        NcbiMessageBox("A file name must be specified!");
        return;
    }


    m_Path = path.ToUTF8();
    m_FileName = filename.ToUTF8();
    string full_path = CDir::ConcatPath(m_Path, m_FileName + ".pdf");

    // Make sure directory is writable:
    CDir dir(m_Path);
    if (!dir.CheckAccess(CDirEntry::fWrite)) {
        NcbiMessageBox("Error - You do not have write permission to the directory: " + m_Path);
        return;
    }

   if (!m_UserSelectedFilename) {
        // Warn user if file exists.  If the file was selected from the file selection
        // dialog we do not need to do this (if not the file came from the registry and probably
        // does already exist)
        CFile f(full_path);
        if (f.Exists()) {
            if (!f.CheckAccess(CDirEntry::fWrite)) {
                NcbiMessageBox("Error - You do not have write permission to the file: " + full_path);
                return;
            }

            int choice = wxMessageBox(wxT("Selected file already exists.  Are you sure you want to  overwrite it?"), 
                                      wxT("Warning"), wxYES_NO  | wxICON_ERROR, this);
            if (choice != wxYES) {
                return;
            }               
        }
    }

    x_SavePdf();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_OPEN
 */

void CwxSavePdfBaseDlg::OnOpenClick( wxCommandEvent& event )
{
    m_OpenPdf = true;
    OnSaveClick(event);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CANCEL
 */

void CwxSavePdfBaseDlg::OnCancelClick( wxCommandEvent& event )
{
    // Cancel all jobs
    //m_Timer.Stop();
    EndModal(wxID_CANCEL);
}

void CwxSavePdfBaseDlg::OnTextctrl5TextUpdated( wxCommandEvent& event )
{
    m_UserSelectedFilename = false;
    event.Skip();
}

void CwxSavePdfBaseDlg::OnTooltipCheckboxClick( wxCommandEvent& event )
{
    event.Skip();
}

END_NCBI_SCOPE

