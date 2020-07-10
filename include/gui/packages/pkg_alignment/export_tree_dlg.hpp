/*  $Id: export_tree_dlg.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#ifndef _EXPORT_TREE_DLG_H_
#define _EXPORT_TREE_DLG_H_


/*!
 * Includes
 */

#include <gui/packages/pkg_alignment/phy_export_params.hpp>
#include <wx/dialog.h>
////@begin includes
#include <wx/valtext.h>
////@end includes

#include <gui/widgets/wx/save_file_helper.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CEXPORTTREEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEXPORTTREEDLG_TITLE _("Export Tree")
#define SYMBOL_CEXPORTTREEDLG_IDNAME ID_CEXPORTTREEDLG
#define SYMBOL_CEXPORTTREEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CEXPORTTREEDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CExportTreeDlg class declaration
 */

class CExportTreeDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CExportTreeDlg )
    DECLARE_EVENT_TABLE()

public:
    CExportTreeDlg();
    CExportTreeDlg( wxWindow* parent, wxWindowID id = SYMBOL_CEXPORTTREEDLG_IDNAME, const wxString& caption = SYMBOL_CEXPORTTREEDLG_TITLE, const wxPoint& pos = SYMBOL_CEXPORTTREEDLG_POSITION, const wxSize& size = SYMBOL_CEXPORTTREEDLG_SIZE, long style = SYMBOL_CEXPORTTREEDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEXPORTTREEDLG_IDNAME, const wxString& caption = SYMBOL_CEXPORTTREEDLG_TITLE, const wxPoint& pos = SYMBOL_CEXPORTTREEDLG_POSITION, const wxSize& size = SYMBOL_CEXPORTTREEDLG_SIZE, long style = SYMBOL_CEXPORTTREEDLG_STYLE );

    ~CExportTreeDlg();

    void Init();

    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

    /// Data access
    CPhyExportParams& GetData() { return m_data; }
    const CPhyExportParams& GetData() const { return m_data; }
    void SetData(const CPhyExportParams& data) { m_data = data; }

////@begin CExportTreeDlg event handler declarations

    void OnSelectFileClick( wxCommandEvent& event );

////@end CExportTreeDlg event handler declarations

////@begin CExportTreeDlg member function declarations

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CExportTreeDlg member function declarations

    static bool ShowToolTips();

////@begin CExportTreeDlg member variables
    wxTextCtrl* m_txtFileName;
    wxChoice* m_FormatChoice;
private:
    wxString m_FileName;
    enum {
        ID_CEXPORTTREEDLG = 10000,
        ID_TEXTCTRL = 10008,
        ID_FILEOPEN_BTN = 10009,
        ID_FORMAT_CHOICE = 10001
    };
////@end CExportTreeDlg member variables
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
    /// The data edited by this window
    CPhyExportParams m_data;
};

END_NCBI_SCOPE

#endif
    // _EXPORT_TREE_DLG_H_
