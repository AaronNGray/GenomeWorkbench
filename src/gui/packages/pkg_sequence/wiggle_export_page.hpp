#ifndef GUI_CORE___WIGGLE_EXPORT_PAGE__HPP
#define GUI_CORE___WIGGLE_EXPORT_PAGE__HPP

/*  $Id: wiggle_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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

/*!
 * Includes
 */

#include <gui/widgets/wx/save_file_helper.hpp>
#include <gui/packages/pkg_sequence/wiggle_export_params.hpp>

////@begin includes
#include "wx/valgen.h"
#include "wx/spinctrl.h"
#include "wx/valtext.h"
////@end includes

#include "wx/panel.h"
#include <wx/statbox.h>
#include <wx/stattext.h>

class wxTextCtrl;

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CWIGGLEEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CWIGGLEEXPORTPAGE_TITLE _("Wiggle Export Page")
#define SYMBOL_CWIGGLEEXPORTPAGE_IDNAME ID_CWIGGLEEXPORTPAGE
#define SYMBOL_CWIGGLEEXPORTPAGE_SIZE wxSize(267, 185)
#define SYMBOL_CWIGGLEEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

////@begin forward declarations
class CObjectListWidget;
class wxSpinCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CObjectListWidget;


/*!
 * CWiggleExportPage class declaration
 */

class CWiggleExportPage: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CWiggleExportPage )
    DECLARE_EVENT_TABLE()

public:
    CWiggleExportPage();
    CWiggleExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CWIGGLEEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CWIGGLEEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CWIGGLEEXPORTPAGE_SIZE, long style = SYMBOL_CWIGGLEEXPORTPAGE_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIGGLEEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CWIGGLEEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CWIGGLEEXPORTPAGE_SIZE, long style = SYMBOL_CWIGGLEEXPORTPAGE_STYLE );

    ~CWiggleExportPage();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

////@begin IRegSettings implementation
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();
////@end IRegSettings implementation

////@begin CWiggleExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
    void OnSelectFileClick( wxCommandEvent& event );

////@end CWiggleExportPage event handler declarations

////@begin CWiggleExportPage member function declarations

    /// Data access
    CWiggleExportParams& GetData() { return m_data; }
    const CWiggleExportParams& GetData() const { return m_data; }
    void SetData(const CWiggleExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWiggleExportPage member function declarations

    static bool ShowToolTips();

////@begin CWiggleExportPage member variables
    CObjectListWidget* m_ObjectSel;
    wxStaticBox* m_BinSizePanel;
    wxStaticText* m_BinSizeLabel;
    wxSpinCtrl* m_BinSizeCtrl;
    wxTextCtrl* m_FromCtrl;
    wxTextCtrl* m_ToCtrl;
    wxTextCtrl* m_FileNameCtrl;
    /// The data edited by this window
    CWiggleExportParams m_data;
    /// Control identifiers
    enum {
        ID_CWIGGLEEXPORTPAGE = 10000,
        ID_PANEL2 = 10037,
        ID_SPINCTRL = 10024,
        ID_TEXTCTRL = 10001,
        ID_TEXTCTRL1 = 10002,
        ID_TEXTCTRL6 = 10054,
        ID_BITMAPBUTTON = 10055
    };
////@end CWiggleExportPage member variables
private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif  /// GUI_CORE___WIGGLE_EXPORT_PAGE__HPP
