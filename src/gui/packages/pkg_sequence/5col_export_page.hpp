#ifndef PKG_SEQUENCE___5COL_EXPORT_PAGE__HPP
#define PKG_SEQUENCE___5COL_EXPORT_PAGE__HPP

/*  $Id: 5col_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <gui/widgets/wx/save_file_helper.hpp>
#include <gui/packages/pkg_sequence/5col_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_C5COLEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_C5COLEXPORTPAGE_TITLE _("5Col Export Page")
#define SYMBOL_C5COLEXPORTPAGE_IDNAME ID_C5COLEXPORTPAGE
#define SYMBOL_C5COLEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_C5COLEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

class wxTextCtrl;

BEGIN_NCBI_SCOPE

class CObjectListWidgetSel;

/*!
 * C5ColExportPage class declaration
 */

class C5ColExportPage: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( C5ColExportPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    C5ColExportPage();
    C5ColExportPage( wxWindow* parent, wxWindowID id = SYMBOL_C5COLEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_C5COLEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_C5COLEXPORTPAGE_SIZE, long style = SYMBOL_C5COLEXPORTPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_C5COLEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_C5COLEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_C5COLEXPORTPAGE_SIZE, long style = SYMBOL_C5COLEXPORTPAGE_STYLE );

    /// Destructor
    ~C5ColExportPage();

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin C5ColExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON2
    void OnSelectFileClcik( wxCommandEvent& event );

////@end C5ColExportPage event handler declarations

////@begin C5ColExportPage member function declarations

    wxString GetFileName() const { return m_FileName ; }
    void SetFileName(wxString value) { m_FileName = value ; }

    /// Data access
    C5ColExportParams& GetData() { return m_data; }
    const C5ColExportParams& GetData() const { return m_data; }
    void SetData(const C5ColExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end C5ColExportPage member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);
    SConstScopedObject GetSeqLoc() const { return m_SeqLoc; }

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin C5ColExportPage member variables
    CObjectListWidgetSel* m_LocationSel;
    wxTextCtrl* m_FileNameCtrl;
private:
    wxString m_FileName;
    /// The data edited by this window
    C5ColExportParams m_data;
    /// Control identifiers
    enum {
        ID_C5COLEXPORTPAGE = 10068,
        ID_OBJECT_LIST = 10069,
        ID_TEXTCTRL10 = 10087,
        ID_CHECKBOX17 = 10088,
        ID_TEXTCTRL11 = 10089,
        ID_BITMAPBUTTON2 = 10014
    };
////@end C5ColExportPage member variables

private:
    SConstScopedObject m_SeqLoc;
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___5COL_EXPORT_PAGE__HPP
