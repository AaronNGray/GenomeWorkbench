#ifndef PKG_SEQUENCE___GFF_EXPORT_PAGE__HPP
#define PKG_SEQUENCE___GFF_EXPORT_PAGE__HPP

/*  $Id: gff_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#include <gui/packages/pkg_sequence/gff_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGFFEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGFFEXPORTPAGE_TITLE _("GFF Export Page")
#define SYMBOL_CGFFEXPORTPAGE_IDNAME ID_CGFFEXPORTPAGE
#define SYMBOL_CGFFEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CGFFEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

class wxChoice;

BEGIN_NCBI_SCOPE

class CObjectListWidgetSel;

/*!
 * CGffExportPage class declaration
 */

class CGffExportPage: public wxPanel
    , public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( CGffExportPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGffExportPage();
    CGffExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CGFFEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CGFFEXPORTPAGE_POSITION, long style = SYMBOL_CGFFEXPORTPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGFFEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CGFFEXPORTPAGE_POSITION, long style = SYMBOL_CGFFEXPORTPAGE_STYLE );

    /// Destructor
    ~CGffExportPage();

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

////@begin CGffExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON
    void OnSelectFileClick( wxCommandEvent& event );

////@end CGffExportPage event handler declarations

////@begin CGffExportPage member function declarations

    /// Data access
    CGffExportParams& GetData() { return m_data; }
    const CGffExportParams& GetData() const { return m_data; }
    void SetData(const CGffExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGffExportPage member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGffExportPage member variables
    CObjectListWidgetSel* m_LocationSel;
    /// The data edited by this window
    CGffExportParams m_data;
    /// Control identifiers
    enum {
        ID_CGFFEXPORTPAGE = 10015,
        ID_PANEL1 = 10026,
        ID_TEXTCTRL2 = 10019,
        ID_CHECKBOX14 = 10021,
        ID_CHECKBOX4 = 10012,
        ID_TEXTCTRL3 = 10023,
        ID_BITMAPBUTTON = 10013
    };
////@end CGffExportPage member variables

private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___GFF_EXPORT_PAGE__HPP
