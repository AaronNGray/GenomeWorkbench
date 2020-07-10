#ifndef GUI_CORE___ASN_EXPORT_PAGE__HPP
#define GUI_CORE___ASN_EXPORT_PAGE__HPP

/*  $Id: asn_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

/*!
 * Includes
 */

#include "wx/panel.h"

#include <gui/widgets/wx/save_file_helper.hpp>
#include <gui/core/asn_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CASNEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CASNEXPORTPAGE_TITLE _("Asn Export Page")
#define SYMBOL_CASNEXPORTPAGE_IDNAME ID_CASNEXPORTPAGE
#define SYMBOL_CASNEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CASNEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

class wxTextCtrl;

BEGIN_NCBI_SCOPE

class CObjectListWidget;

/*!
 * CAsnExportPage class declaration
 */

class CAsnExportPage: public wxPanel
    , public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( CAsnExportPage )
    DECLARE_EVENT_TABLE()

public:
    CAsnExportPage();
    CAsnExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CASNEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CASNEXPORTPAGE_POSITION, long style = SYMBOL_CASNEXPORTPAGE_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASNEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CASNEXPORTPAGE_POSITION, long style = SYMBOL_CASNEXPORTPAGE_STYLE );

    ~CAsnExportPage();

    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

////@begin CAsnExportPage event handler declarations

    void OnSelectFileClick( wxCommandEvent& event );

////@end CAsnExportPage event handler declarations

////@begin CAsnExportPage member function declarations

    CAsnExportParams& GetData() { return m_data; }
    const CAsnExportParams& GetData() const { return m_data; }
    void SetData(const CAsnExportParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAsnExportPage member function declarations

    static bool ShowToolTips();

////@begin CAsnExportPage member variables
    CObjectListWidget* m_ObjectSel;
    wxTextCtrl* m_FileNameCtrl;
    CAsnExportParams m_data;
    enum {
        ID_CASNEXPORTPAGE = 10036,
        ID_PANEL2 = 10037,
        ID_RADIOBOX3 = 10039,
        ID_TEXTCTRL6 = 10054,
        ID_BITMAPBUTTON = 10055
    };
////@end CAsnExportPage member variables

private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif  /// GUI_CORE___ASN_EXPORT_PAGE__HPP
