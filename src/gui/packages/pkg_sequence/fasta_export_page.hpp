#ifndef PKG_SEQUENCE___FASTA_EXPORT_PAGE__HPP
#define PKG_SEQUENCE___FASTA_EXPORT_PAGE__HPP

/*  $Id: fasta_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
#include <gui/packages/pkg_sequence/fasta_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFASTAEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFASTAEXPORTPAGE_TITLE _("FASTA Export Page")
#define SYMBOL_CFASTAEXPORTPAGE_IDNAME ID_FASTA_EXPORT_PAGE
#define SYMBOL_CFASTAEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CFASTAEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CObjectListWidgetSel;

/*!
 * CFastaExportPage class declaration
 */

class CFastaExportPage: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CFastaExportPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFastaExportPage();
    CFastaExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CFASTAEXPORTPAGE_IDNAME, const wxString& caption = SYMBOL_CFASTAEXPORTPAGE_TITLE, const wxPoint& pos = SYMBOL_CFASTAEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CFASTAEXPORTPAGE_SIZE, long style = SYMBOL_CFASTAEXPORTPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFASTAEXPORTPAGE_IDNAME, const wxString& caption = SYMBOL_CFASTAEXPORTPAGE_TITLE, const wxPoint& pos = SYMBOL_CFASTAEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CFASTAEXPORTPAGE_SIZE, long style = SYMBOL_CFASTAEXPORTPAGE_STYLE );

    /// Destructor
    ~CFastaExportPage();

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataFromWindow();

////@begin CFastaExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON3
    void OnSelectFileClick( wxCommandEvent& event );

////@end CFastaExportPage event handler declarations

////@begin CFastaExportPage member function declarations

    /// Data access
    CFastaExportParams& GetData() { return m_data; }
    const CFastaExportParams& GetData() const { return m_data; }
    void SetData(const CFastaExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFastaExportPage member function declarations

    void SetObjects(map<string, TConstScopedObjects>* objects);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFastaExportPage member variables
    CObjectListWidgetSel* m_LocationSel;
    /// The data edited by this window
    CFastaExportParams m_data;
    /// Control identifiers
    enum {
        ID_FASTA_EXPORT_PAGE = 10058,
        ID_PANEL = 10059,
        ID_CHECKBOX = 10060,
        ID_RADIOBOX = 10001,
        ID_TEXTCTRL = 10061,
        ID_BITMAPBUTTON3 = 10016
    };
////@end CFastaExportPage member variables

private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_SEQUENCE___FASTA_EXPORT_PAGE__HPP
