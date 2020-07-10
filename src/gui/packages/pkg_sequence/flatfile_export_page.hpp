#ifndef PKG_SEQUENCE___FLATFILE_EXPORT_PAGE__HPP
#define PKG_SEQUENCE___FLATFILE_EXPORT_PAGE__HPP

/*  $Id: flatfile_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#include <gui/packages/pkg_sequence/flatfile_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFLATFILEEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFLATFILEEXPORTPAGE_TITLE _("FlatFile Export Page")
#define SYMBOL_CFLATFILEEXPORTPAGE_IDNAME ID_CFLATFILEEXPORTPAGE
#define SYMBOL_CFLATFILEEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CFLATFILEEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

class wxChoice;

BEGIN_NCBI_SCOPE

class CObjectListWidgetSel;


/*!
 * CFlatFileExportPage class declaration
 */

class CFlatFileExportPage: public wxPanel
    , public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( CFlatFileExportPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFlatFileExportPage();
    CFlatFileExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CFLATFILEEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CFLATFILEEXPORTPAGE_POSITION, long style = SYMBOL_CFLATFILEEXPORTPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFLATFILEEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CFLATFILEEXPORTPAGE_POSITION, long style = SYMBOL_CFLATFILEEXPORTPAGE_STYLE );

    /// Destructor
    ~CFlatFileExportPage();

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

////@begin CFlatFileExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON4
    void OnSelectFileClick( wxCommandEvent& event );

////@end CFlatFileExportPage event handler declarations

////@begin CFlatFileExportPage member function declarations

    /// Data access
    CFlatFileExportParams& GetData() { return m_data; }
    const CFlatFileExportParams& GetData() const { return m_data; }
    void SetData(const CFlatFileExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFlatFileExportPage member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFlatFileExportPage member variables
    CObjectListWidgetSel* m_LocationSel;
    /// The data edited by this window
    CFlatFileExportParams m_data;
    /// Control identifiers
    enum {
        ID_CFLATFILEEXPORTPAGE = 10046,
        ID_PANEL3 = 10047,
        ID_CHOICE3 = 10048,
        ID_TEXTCTRL7 = 10049,
        ID_BITMAPBUTTON4 = 10020
    };
////@end CFlatFileExportPage member variables

private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___FLATFILE_EXPORT_PAGE__HPP
