/*  $Id: table_xform_panel.hpp 43091 2019-05-14 15:48:52Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#ifndef _TABLE_XFORM_PANEL_H_
#define _TABLE_XFORM_PANEL_H_


/*!
 * Includes
 */

////@begin includes
////@end includes

#include "wx/listctrl.h"
#include "gui/widgets/loaders/table_data_listctrl.hpp"

#include <gui/widgets/loaders/table_import_data_source.hpp>
#include <gui/widgets/loaders/table_annot_data_source.hpp>

#include <objmgr/scope.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CTableDataListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTABLEXFORMPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTABLEXFORMPANEL_TITLE _("Table Data Transform")
#define SYMBOL_CTABLEXFORMPANEL_IDNAME ID_CTABLEXFORMPANEL
#define SYMBOL_CTABLEXFORMPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTABLEXFORMPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CTableXformPanel class declaration
 */

class CTableXformPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTableXformPanel )
    DECLARE_EVENT_TABLE()

public:
    enum eTransformType { eRegionFeatures=0, eAddLocs, eNoTransform };

public:
    CTableXformPanel();
    CTableXformPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEXFORMPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEXFORMPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEXFORMPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEXFORMPANEL_SIZE, long style = SYMBOL_CTABLEXFORMPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTABLEXFORMPANEL_IDNAME, const wxString& caption = SYMBOL_CTABLEXFORMPANEL_TITLE, const wxPoint& pos = SYMBOL_CTABLEXFORMPANEL_POSITION, const wxSize& size = SYMBOL_CTABLEXFORMPANEL_SIZE, long style = SYMBOL_CTABLEXFORMPANEL_STYLE );

    ~CTableXformPanel();

    void Init();

    void CreateControls();

    /// Override Show() to allow initialization each time window is displayed
    bool Show(bool show);

    void SetMainTitle(const wxString& title);

    void SetImportDataSource(CRef<CTableImportDataSource> ds) { m_ImportedTableData = ds; }
    void SetAnnotDataSource(CRef<CTableAnnotDataSource> ad) { m_AnnotTableData = ad; }

    bool IsInputValid();
    bool GetLastTransform() const { return m_LastXform; }

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

////@begin CTableXformPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SAVEIMPORTPARMS
    void OnSaveImportParmsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_CONVERTTOFEATURES
    void OnConvertToFeaturesSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_CREATELOCATIONS
    void OnCreateLocationsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_CONVERTTOSNP
    void OnConvertToSnpSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_DONOTCONVERT
    void OnDoNotConvertSelected( wxCommandEvent& event );

////@end CTableXformPanel event handler declarations

////@begin CTableXformPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTableXformPanel member function declarations

    static bool ShowToolTips();

////@begin CTableXformPanel member variables
    wxRadioButton* m_ConvertToFeatures;
    wxRadioButton* m_CreateLocations;
    wxRadioButton* m_ConvertToSnips;
    wxRadioButton* m_KeepTableUnchanged;
    wxStaticText* m_XformInfo;
    CTableDataListCtrl* m_TableDataListCtrl;
    /// Control identifiers
    enum {
        ID_CTABLEXFORMPANEL = 10069,
        ID_SAVEIMPORTPARMS = 10105,
        ID_CONVERTTOFEATURES = 10109,
        ID_CREATELOCATIONS = 10107,
        ID_CONVERTTOSNP = 10008,
        ID_DONOTCONVERT = 10110,
        ID_TABLEXFORMPANEL = 10082,
        ID_CTABLEDATALISTCTRL = 10106
    };
////@end CTableXformPanel member variables

protected:

    /// Set text to describe consequenses of chosen transformation option
    void    x_SetHelpText();

    string         m_RegPath;

    /// Contents of tabular file
    CRef<CTableImportDataSource> m_ImportedTableData; 

    /// Tabular file converted to a seq-annot
    CRef<CTableAnnotDataSource> m_AnnotTableData;

    /// Records type of most recent transform (needed to allow undo)
    eTransformType  m_LastXform;

    CRef<objects::CScope> m_Scope;

private:
    bool x_AddLocations(string& errMsg, ICanceled& canceled);
    bool x_CreateFeatures(bool create_snps, string& errMsg, ICanceled& canceled);
};

END_NCBI_SCOPE

#endif  // _TABLE_XFORM_PANEL_H_
