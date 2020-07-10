/*  $Id: winmask_files_dlg.cpp 43938 2019-09-20 20:38:22Z katargir $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "winmask_files_dlg.hpp"
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>

#include <gui/widgets/loaders/winmask_files.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>
#include <gui/utils/ftp_utils.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checklst.h>
#include <wx/treectrl.h>
#include <wx/wupdlock.h>
#include <wx/dirdlg.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/checkbox.h>
#include <wx/statline.h>
#include <wx/radiobut.h>
#include <wx/artprov.h>

#include <wx/filename.h>
//#include <wx/dir.h>
#include <wx/msgdlg.h>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CWinMaskFilesDlg, CDialog )

BEGIN_EVENT_TABLE( CWinMaskFilesDlg, CDialog )

////@begin CWinMaskFilesDlg event table entries
    EVT_RADIOBUTTON( ID_RADIOBUTTON, CWinMaskFilesDlg::OnUseWMPATHSelected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON1, CWinMaskFilesDlg::OnUseDownloadedSelected )

    EVT_BUTTON( ID_BUTTON2, CWinMaskFilesDlg::OnResetDownloads )

////@end CWinMaskFilesDlg event table entries

END_EVENT_TABLE()

CWinMaskFilesDlg::CWinMaskFilesDlg() : m_ResetCache(false)
{
    Init();
}

CWinMaskFilesDlg::CWinMaskFilesDlg(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_ResetCache(false)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CWinMaskFilesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWinMaskFilesDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWinMaskFilesDlg creation
    return true;
}

CWinMaskFilesDlg::~CWinMaskFilesDlg()
{
    if (m_ResetCache)
        CTaxIdHelper::GetInstance().Reset(true);
}

void CWinMaskFilesDlg::Init()
{
////@begin CWinMaskFilesDlg member initialisation
    m_UseWMPATHCtrl = NULL;
    m_WMPATHNotAvailableCtrl = NULL;
    m_WMPATHValueCtrl = NULL;
    m_UseDownloadedCtrl = NULL;
    m_FileListTitleSizer = NULL;
    m_FileListTitle = NULL;
    m_FileList = NULL;
    m_HelpLine = NULL;
////@end CWinMaskFilesDlg member initialisation
}

static vector<string> sFtpFileList;

void CWinMaskFilesDlg::CreateControls()
{    
////@begin CWinMaskFilesDlg content construction
    CWinMaskFilesDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("WindowMasker data files for BLAST"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 10);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALL, 0);

    m_UseWMPATHCtrl = new wxRadioButton( itemCDialog1, ID_RADIOBUTTON, _("Use WINDOW_MASKER_PATH value :"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseWMPATHCtrl->SetValue(false);
    itemBoxSizer4->Add(m_UseWMPATHCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer4->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMPATHNotAvailableCtrl = new wxStaticText( itemCDialog1, wxID_STATIC, _("(Not accessible)"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMPATHNotAvailableCtrl->SetForegroundColour(wxColour(255, 0, 0));
    m_WMPATHNotAvailableCtrl->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    m_WMPATHNotAvailableCtrl->Show(false);
    itemBoxSizer4->Add(m_WMPATHNotAvailableCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer8, 0, wxGROW|wxALL, 0);

    m_WMPATHValueCtrl = new wxStaticText( itemCDialog1, wxID_STATIC, _("Empty"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_WMPATHValueCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine10 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer8->Add(itemStaticLine10, 0, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine11 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemStaticBoxSizer3->Add(itemStaticLine11, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer12, 0, wxGROW|wxALL, 0);

    m_UseDownloadedCtrl = new wxRadioButton( itemCDialog1, ID_RADIOBUTTON1, _("Use downloaded files"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseDownloadedCtrl->SetValue(false);
    itemBoxSizer12->Add(m_UseDownloadedCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer12->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemCDialog1, ID_BUTTON2, _("Reset"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileListTitleSizer = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(m_FileListTitleSizer, 0, wxGROW|wxTOP|wxBOTTOM, 0);

    m_FileListTitle = new wxStaticText( itemCDialog1, wxID_STATIC, _("Select Window Masker files to download:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FileListTitleSizer->Add(m_FileListTitle, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxArrayString m_FileListStrings;
    m_FileList = new wxCheckListBox( itemCDialog1, ID_CHECKLISTBOX1, wxDefaultPosition, wxSize(-1, itemCDialog1->ConvertDialogToPixels(wxSize(-1, 80)).y), m_FileListStrings, wxLB_NEEDED_SB );
    itemStaticBoxSizer3->Add(m_FileList, 1, wxGROW|wxALL, 5);

    m_HelpLine = new wxStaticText( itemCDialog1, wxID_STATIC, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(m_HelpLine, 0, wxALIGN_LEFT|wxLEFT, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer20 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer20, 0, wxALIGN_RIGHT|wxALL, 10);
    wxButton* itemButton21 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer20->AddButton(itemButton21);

    itemStdDialogButtonSizer20->Realize();

////@end CWinMaskFilesDlg content construction

    int height = m_FileListTitle->GetSize().GetHeight();

    wxStaticText* loadingText = new wxStaticText(itemCDialog1, ID_LOADING_TEXT, wxT("Loading file list from FTP..."), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    loadingText->SetFont(bold);
    loadingText->SetForegroundColour(*wxBLACK);
    m_FileListTitleSizer->Add(loadingText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    height= max(height, loadingText->GetSize().GetHeight());

    CIndProgressBar* progress = new CIndProgressBar(itemCDialog1, ID_LOADING_PROGRESS, wxDefaultPosition, 100);
    m_FileListTitleSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    height = max(height, progress->GetSize().GetHeight());

    m_FileListTitleSizer->Insert(m_FileListTitleSizer->GetItemCount() - 1, 1, height, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_FileListTitleSizer->Hide(loadingText);
    m_FileListTitleSizer->Hide(progress);

    CWinMaskerFileStorage& wm = CWinMaskerFileStorage::GetInstance();

    wxString envPath = wm.GetEnvPath();
    if (envPath.empty() || !wxFileName::DirExists(envPath))
        m_WMPATHNotAvailableCtrl->Show();
    if (envPath.empty())
        m_WMPATHValueCtrl->SetLabel(wxT("Empty"));
    else
        m_WMPATHValueCtrl->SetLabel(wxT("\"") + envPath + wxT("\""));

    m_HelpLine->SetLabel(wxT("\"") + wm.GeFtpUrl() + wxT("\""));

    bool useEnvPath = wm.GetUseEnvPath();
    m_UseWMPATHCtrl->SetValue(useEnvPath);
    m_UseDownloadedCtrl->SetValue(!useEnvPath);

    if (sFtpFileList.empty()) {
        x_ShowLoadingFiles(true);
        m_FutureJob.reset(job_async(
            [url = string(wm.GeFtpUrl().ToUTF8())](ICanceled&)
        {
            vector<string> files;
            CFtpUtils::GetDirContent(files, url);
            return files;
        },
            [this](job_future<vector<string> >& future)
        {
            try {
                sFtpFileList = future();
            } NCBI_CATCH("Retrieving FTP file list.");

            x_FillFileList();
        },
            "Get FTP file list"));
    }
    else
        x_FillFileList();

    m_FileList->Enable(!useEnvPath);
}

void CWinMaskFilesDlg::x_ShowLoadingFiles(bool show)
{
    m_FileListTitleSizer->Show(FindWindow(ID_LOADING_TEXT), show);
    m_FileListTitleSizer->Show(FindWindow(ID_LOADING_PROGRESS), show);
    m_FileListTitleSizer->Show(m_FileListTitle, !show);
    GetSizer()->Layout();
}

void CWinMaskFilesDlg::x_FillFileList()
{
    for (const auto& f : sFtpFileList)
        m_FileList->Append(ToWxString(f));

    x_ShowLoadingFiles(false);
}

bool CWinMaskFilesDlg::TransferDataFromWindow()
{
    if (!wxWindow::TransferDataFromWindow())
        return false;

    CWinMaskerFileStorage& wm = CWinMaskerFileStorage::GetInstance();

    bool useEnvPath = m_UseWMPATHCtrl->GetValue();
    if (useEnvPath != wm.GetUseEnvPath()) {
        m_ResetCache = true;
        wm.SetUseEnvPath(useEnvPath);
    }

    if (!useEnvPath) {
        vector<string> files_to_load;
        for (int i = 0; i < sFtpFileList.size(); i++) {
            if (m_FileList->IsChecked(i))
                files_to_load.push_back(sFtpFileList[i]);
        }

        if (!files_to_load.empty()) {
            m_ResetCache = true;
            wm.x_DownloadFiles(files_to_load);
        }
    }

    return true;
}

void CWinMaskFilesDlg::OnUseWMPATHSelected(wxCommandEvent& event)
{
    m_UseDownloadedCtrl->SetValue(false);
    m_FileList->Enable(false);
}

void CWinMaskFilesDlg::OnUseDownloadedSelected(wxCommandEvent& event)
{
    m_UseWMPATHCtrl->SetValue(false);
    m_FileList->Enable(true);
}

void CWinMaskFilesDlg::OnResetDownloads(wxCommandEvent& event)
{
    CWinMaskerFileStorage& wm = CWinMaskerFileStorage::GetInstance();
    if (wm.IsDownloading()) {
        wxMessageBox(wxT("Downloading in progress. Can't delete files."), wxT("Error"), wxOK|wxICON_ERROR);
        return;
    }

    if (wxYES == wxMessageBox("You are about to delete all downloaded Window Masker files. Proceed?", "Confirm", wxYES_NO)) {
        m_ResetCache = true;
        if (!wm.x_ClearDownloads())
            wxMessageBox(wxT("Failed to delete downloaded files"), wxT("Error"), wxOK|wxICON_ERROR);
    }
}

bool CWinMaskFilesDlg::ShowToolTips()
{
    return true;
}

wxBitmap CWinMaskFilesDlg::GetBitmapResource(const wxString& name)
{
    return wxArtProvider::GetBitmap(name);
}

wxIcon CWinMaskFilesDlg::GetIconResource(const wxString& name)
{
    // Icon retrieval
    ////@begin CWinMaskFilesDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
    ////@end CWinMaskFilesDlg icon retrieval
}

END_NCBI_SCOPE
