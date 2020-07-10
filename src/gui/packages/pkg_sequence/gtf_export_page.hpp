#ifndef PKG_SEQUENCE___GTF_EXPORT_PAGE__HPP
#define PKG_SEQUENCE___GTF_EXPORT_PAGE__HPP

/*  $Id: gtf_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#include <gui/packages/pkg_sequence/gtf_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGTFEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGTFEXPORTPAGE_TITLE _("GTF Export Page")
#define SYMBOL_CGTFEXPORTPAGE_IDNAME ID_CGTFEXPORTPAGE
#define SYMBOL_CGTFEXPORTPAGE_SIZE wxSize(266, 184)
#define SYMBOL_CGTFEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CObjectListWidgetSel;

/*!
 * CGtfExportPage class declaration
 */

class CGtfExportPage: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CGtfExportPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGtfExportPage();
    CGtfExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CGTFEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CGTFEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CGTFEXPORTPAGE_SIZE, long style = SYMBOL_CGTFEXPORTPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGTFEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CGTFEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CGTFEXPORTPAGE_SIZE, long style = SYMBOL_CGTFEXPORTPAGE_STYLE );

    /// Destructor
    ~CGtfExportPage();

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

////@begin CGtfExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON1
    void OnSelectFileClick( wxCommandEvent& event );

////@end CGtfExportPage event handler declarations

////@begin CGtfExportPage member function declarations

    /// Data access
    CGtfExportParams& GetData() { return m_data; }
    const CGtfExportParams& GetData() const { return m_data; }
    void SetData(const CGtfExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGtfExportPage member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGtfExportPage member variables
    CObjectListWidgetSel* m_LocationSel;
    /// The data edited by this window
    CGtfExportParams m_data;
    /// Control identifiers
    enum {
        ID_CGTFEXPORTPAGE = 10104,
        ID_PANEL7 = 10105,
        ID_TEXTCTRL1 = 10106,
        ID_CHECKBOX1 = 10107,
        ID_CHECKBOX2 = 10108,
        ID_CHECKBOX3 = 10109,
        ID_TEXTCTRL6 = 10110,
        ID_BITMAPBUTTON1 = 10111
    };
////@end CGtfExportPage member variables

private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___GTF_EXPORT_PAGE__HPP
