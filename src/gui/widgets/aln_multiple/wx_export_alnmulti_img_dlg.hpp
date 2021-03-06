/*  $Id: wx_export_alnmulti_img_dlg.hpp 44954 2020-04-27 17:57:36Z evgeniev $
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description:
 *
 */
// Generated by DialogBlocks (unregistered), 24/04/2020 15:10:21

#ifndef _WX_EXPORT_ALNMULTI_IMG_DLG_H_
#define _WX_EXPORT_ALNMULTI_IMG_DLG_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes

#include <corelib/ncbiobj.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/timer.h>
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
class CAlnMultiWidget;

////@begin control identifiers
#define SYMBOL_CWXEXPORTALNMULTIIMGDLG_STYLE wxDEFAULT_DIALOG_STYLE|wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXEXPORTALNMULTIIMGDLG_TITLE _("Save Image As ")
#define SYMBOL_CWXEXPORTALNMULTIIMGDLG_IDNAME ID_CWXSAVEMULTIALNIMGDLG
#define SYMBOL_CWXEXPORTALNMULTIIMGDLG_SIZE wxSize(440, 210)
#define SYMBOL_CWXEXPORTALNMULTIIMGDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxExportAlnmultiImgDlg class declaration
 */

class CwxExportAlnmultiImgDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxExportAlnmultiImgDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxExportAlnmultiImgDlg();
    CwxExportAlnmultiImgDlg( CPrintOptions::EOutputFormat format, CAlnMultiWidget* widget, wxWindow* parent );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXEXPORTALNMULTIIMGDLG_IDNAME, const wxString& caption = SYMBOL_CWXEXPORTALNMULTIIMGDLG_TITLE, const wxPoint& pos = SYMBOL_CWXEXPORTALNMULTIIMGDLG_POSITION, const wxSize& size = SYMBOL_CWXEXPORTALNMULTIIMGDLG_SIZE, long style = SYMBOL_CWXEXPORTALNMULTIIMGDLG_STYLE);

    ~CwxExportAlnmultiImgDlg();

    void Init();

    void CreateControls();

    void LoadSettings();
    void SaveSettings() const;

    const string& GetPath() const { return m_Path; };
    const string& GetFileName() const { return m_FileName; };

////@begin CwxExportVectorImageDlg event handler declarations

    void OnTextctrl5TextUpdated( wxCommandEvent& event );

    void OnFilepathClick( wxCommandEvent& event );

    void OnSaveClick( wxCommandEvent& event );

    void OnOpenClick( wxCommandEvent& event );

    void OnCancelClick( wxCommandEvent& event );

////@end CwxExportAlnmultiImgDlg event handler declarations

////@begin CwxExportAlnmultiImgDlg member function declarations

    bool GetShowHeader() const { return m_ShowHeader ; }
    void SetShowHeader(bool value) { m_ShowHeader = value ; }

    bool GetSimplifiedGraphics() const { return m_SimplifiedGraphics ; }
    void SetSimplifiedGraphics(bool value) { m_SimplifiedGraphics = value ; }

    bool GetOpenDocument() const { return m_OpenDocument ; }
    void SetOpenDocument(bool value) { m_OpenDocument = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxExportAlnmultiImgDlg member function declarations

    static bool ShowToolTips();

////@begin CwxExportAlnmultiImgDlg member variables
    wxPanel* m_Panel;
    wxTextCtrl* m_FileNameBox;
    wxCheckBox* m_DisplayHeaderRow;
    wxCheckBox* m_SaveSimplified;
    wxBoxSizer* m_InfoSizer;
    wxStaticText* m_Info;
    wxButton* m_SaveButton;
    wxButton* m_OpenButton;
private:
    bool m_ShowHeader;
    bool m_SimplifiedGraphics;
    bool m_OpenDocument;
    enum {
        ID_CWXSAVEMULTIALNIMGDLG = 10048,
        ID_PANEL3 = 10052,
        ID_TEXTCTRL5 = 10049,
        ID_BITMAPBUTTON = 10021,
        ID_CHECKBOX7 = 10062,
        ID_CHECKBOX6 = 10043,
        ID_SAVE = 10054,
        ID_OPEN = 10053,
        ID_CANCEL = 10055
    };
////@end CwxExportAlnmultiImgDlg member variables
private:
    wxTimer m_Timer;
    CAlnMultiWidget  *m_AlnMultiWidget;
    string           m_Path;
    string           m_FileName;
    string           m_FileExtension;

    /// view file after finished.
    bool             m_UserSelectedFilename;
    CPrintOptions::EOutputFormat    m_OutputFormat = CPrintOptions::ePdf;
};

END_NCBI_SCOPE

#endif
    // _WX_EXPORT_ALNMULTI_IMG_DLG_H_
