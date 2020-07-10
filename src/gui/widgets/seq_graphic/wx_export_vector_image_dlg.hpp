/*  $Id: wx_export_vector_image_dlg.hpp 43842 2019-09-09 21:57:24Z evgeniev $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */
#ifndef _WX_EXPORT_VECTOR_IMAGE_DLG_H_
#define _WX_EXPORT_VECTOR_IMAGE_DLG_H_


/*!
 * Includes
 */

////@begin includes
////@end includes

#include <corelib/ncbiobj.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/timer.h>
#include <gui/opengl/glpane.hpp>
#include <gui/print/print_options.hpp>


////@begin forward declarations
class wxBoxSizer;
////@end forward declarations
class wxPanel;
class wxCheckBox;

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

class CSeqGraphicPane;
class CIndProgressBar;


////@begin control identifiers
#define SYMBOL_CWXEXPORTVECTORIMAGEDLG_STYLE wxDEFAULT_DIALOG_STYLE|wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXEXPORTVECTORIMAGEDLG_TITLE _("Save Image As ")
#define SYMBOL_CWXEXPORTVECTORIMAGEDLG_IDNAME ID_CWXEXPORTVECTORIMAGEDLG
#define SYMBOL_CWXEXPORTVECTORIMAGEDLG_SIZE wxSize(440, 210)
#define SYMBOL_CWXEXPORTVECTORIMAGEDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CwxExportVectorImageDlg class declaration
 */

class CwxExportVectorImageDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxExportVectorImageDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxExportVectorImageDlg();
    CwxExportVectorImageDlg( CPrintOptions::EOutputFormat format, CSeqGraphicPane* pane, wxWindow* parent);

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXEXPORTVECTORIMAGEDLG_IDNAME, const wxString& caption = SYMBOL_CWXEXPORTVECTORIMAGEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXEXPORTVECTORIMAGEDLG_POSITION, const wxSize& size = SYMBOL_CWXEXPORTVECTORIMAGEDLG_SIZE, long style = SYMBOL_CWXEXPORTVECTORIMAGEDLG_STYLE );

    ~CwxExportVectorImageDlg();

    void Init();

    void CreateControls();

    void SetSeqRange(TSeqPos from, TSeqPos to);
    void SetViewport(const TVPRect& vp);

    void OnTimer(wxTimerEvent&);

    void LoadSettings();
    void SaveSettings() const;

////@begin CwxExportVectorImageDlg event handler declarations

    void OnTextctrl5TextUpdated( wxCommandEvent& event );

    void OnFilepathClick( wxCommandEvent& event );

    void OnSaveClick( wxCommandEvent& event );

    void OnOpenClick( wxCommandEvent& event );

    void OnCancelClick( wxCommandEvent& event );

////@end CwxExportVectorImageDlg event handler declarations

////@begin CwxExportVectorImageDlg member function declarations

    wxString GetSeqRange() const { return m_SeqRange ; }
    void SetSeqRange(wxString value) { m_SeqRange = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxExportVectorImageDlg member function declarations

    static bool ShowToolTips();

////@begin CwxExportVectorImageDlg member variables
    wxPanel* m_Panel;
    wxTextCtrl* m_FileNameBox;
    wxTextCtrl* m_SeqRangeBox;
    wxCheckBox* m_SaveSimplified;
    wxCheckBox* m_DisplayTitle;
    wxBoxSizer* m_InfoSizer;
    wxStaticText* m_Info;
    wxButton* m_SaveButton;
    wxButton* m_OpenButton;
private:
    wxString m_SeqRange;
    enum {
        ID_CWXEXPORTVECTORIMAGEDLG = 10048,
        ID_PANEL3 = 10052,
        ID_TEXTCTRL5 = 10049,
        ID_BITMAPBUTTON = 10021,
        ID_TEXTCTRL6 = 10051,
        ID_CHECKBOX6 = 10043,
        ID_CHECKBOX7 = 10062,
        ID_SAVE = 10054,
        ID_OPEN = 10053,
        ID_CANCEL = 10055
    };
////@end CwxExportVectorImageDlg member variables
private:
    bool x_GetSeqRange(TSeqPos& from, TSeqPos& to) const;

    enum EState 
    {
        eInitial,
        eCreatingTracks,
        eLoadingData,
        eReadyToSave
    };

    wxTimer m_Timer;
    CSeqGraphicPane* m_Pane;
    CIndProgressBar* m_ProgressBar;
    string           m_Path;
    string           m_FileName;
    string           m_FileExtension;
    TSeqRange        m_OrgRange;
    TVPRect          m_VP;
    TSeqPos          m_NewFrom;
    TSeqPos          m_NewTo;
    TVPUnit          m_NewImgWidth;

    EState           m_State;
    /// view PDF file after finished.
    bool             m_OpenDocument;
    bool             m_UserSelectedFilename;
    CPrintOptions::EOutputFormat    m_OutputFormat = CPrintOptions::ePdf;
};

END_NCBI_SCOPE

#endif
    // _WX_EXPORT_VECTOR_IMAGE_DLG_H_
