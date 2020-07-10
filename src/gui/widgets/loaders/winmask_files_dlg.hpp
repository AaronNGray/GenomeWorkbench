#ifndef GUI_WIDGETS___LOADERS___WINMASK_FILES_DLG__HPP
#define GUI_WIDGETS___LOADERS___WINMASK_FILES_DLG__HPP

/*  $Id: winmask_files_dlg.hpp 43936 2019-09-20 20:26:42Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dialog.hpp>
#include <wx/dialog.h>

#include <gui/utils/job_future.hpp>


/*!
 * Forward declarations
 */

class wxTextCtrl;
class wxButton;
class wxCheckListBox;
class wxStaticText;
class wxCheckBox;
class wxBitmapButton;
class wxRadioButton;

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CWINMASKFILESDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWINMASKFILESDLG_TITLE _("WindowMasker files information")
#define SYMBOL_CWINMASKFILESDLG_IDNAME ID_CWIN_MASK_FILES_DLG
#define SYMBOL_CWINMASKFILESDLG_SIZE wxDefaultSize
#define SYMBOL_CWINMASKFILESDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CWinMaskFilesDlg class declaration
 */

class CWinMaskFilesDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CWinMaskFilesDlg )
    DECLARE_EVENT_TABLE()

public:
    CWinMaskFilesDlg();
    CWinMaskFilesDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWINMASKFILESDLG_IDNAME, const wxString& caption = SYMBOL_CWINMASKFILESDLG_TITLE, const wxPoint& pos = SYMBOL_CWINMASKFILESDLG_POSITION, const wxSize& size = SYMBOL_CWINMASKFILESDLG_SIZE, long style = SYMBOL_CWINMASKFILESDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWINMASKFILESDLG_IDNAME, const wxString& caption = SYMBOL_CWINMASKFILESDLG_TITLE, const wxPoint& pos = SYMBOL_CWINMASKFILESDLG_POSITION, const wxSize& size = SYMBOL_CWINMASKFILESDLG_SIZE, long style = SYMBOL_CWINMASKFILESDLG_STYLE );

    ~CWinMaskFilesDlg();

    void Init();

    void CreateControls();

    virtual bool TransferDataFromWindow();

////@begin CWinMaskFilesDlg event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
    void OnUseWMPATHSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON1
    void OnUseDownloadedSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnResetDownloads( wxCommandEvent& event );

////@end CWinMaskFilesDlg event handler declarations

////@begin CWinMaskFilesDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWinMaskFilesDlg member function declarations

    static bool ShowToolTips();

////@begin CWinMaskFilesDlg member variables
    wxRadioButton* m_UseWMPATHCtrl;
    wxStaticText* m_WMPATHNotAvailableCtrl;
    wxStaticText* m_WMPATHValueCtrl;
    wxRadioButton* m_UseDownloadedCtrl;
    wxBoxSizer* m_FileListTitleSizer;
    wxStaticText* m_FileListTitle;
    wxCheckListBox* m_FileList;
    wxStaticText* m_HelpLine;
    /// Control identifiers
    enum {
        ID_CWIN_MASK_FILES_DLG = 10023,
        ID_RADIOBUTTON = 10029,
        ID_RADIOBUTTON1 = 10030,
        ID_BUTTON2 = 10031,
        ID_CHECKLISTBOX1 = 10027
    };
////@end CWinMaskFilesDlg member variables

    enum {
        ID_LOADING_TEXT = 9998,
        ID_LOADING_PROGRESS = 9999
    };

private:
    void x_FillFileList();
    void x_ShowLoadingFiles(bool show);

    bool m_ResetCache;
    std::unique_ptr<async_job> m_FutureJob;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___WINMASK_FILES_DLG__HPP
