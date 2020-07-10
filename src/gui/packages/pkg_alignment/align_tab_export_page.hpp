#ifndef PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PAGE__HPP
#define PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PAGE__HPP

/*  $Id: align_tab_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <gui/packages/pkg_alignment/align_tab_export_params.hpp>
#include <gui/widgets/wx/save_file_helper.hpp>

#include <gui/objutils/objects.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CALIGNTABEXPORTPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CALIGNTABEXPORTPAGE_TITLE _("Align Tab Export Page")
#define SYMBOL_CALIGNTABEXPORTPAGE_IDNAME ID_CALIGNTABEXPORTPAGE
#define SYMBOL_CALIGNTABEXPORTPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CALIGNTABEXPORTPAGE_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CObjectListWidgetSel;

/*!
 * CAlignTabExportPage class declaration
 */

class CAlignTabExportPage: public wxPanel
    , public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CAlignTabExportPage )
    DECLARE_EVENT_TABLE()

public:
    CAlignTabExportPage();
    CAlignTabExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNTABEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CALIGNTABEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CALIGNTABEXPORTPAGE_SIZE, long style = SYMBOL_CALIGNTABEXPORTPAGE_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNTABEXPORTPAGE_IDNAME, const wxPoint& pos = SYMBOL_CALIGNTABEXPORTPAGE_POSITION, const wxSize& size = SYMBOL_CALIGNTABEXPORTPAGE_SIZE, long style = SYMBOL_CALIGNTABEXPORTPAGE_STYLE );

    ~CAlignTabExportPage();

    void Init();

    void SetObjects(map<string, TConstScopedObjects>* objects);

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

////@begin CAlignTabExportPage event handler declarations

    void OnSelectFileClick( wxCommandEvent& event );

////@end CAlignTabExportPage event handler declarations

////@begin CAlignTabExportPage member function declarations

    CAlignTabExportParams& GetData() { return m_data; }
    const CAlignTabExportParams& GetData() const { return m_data; }
    void SetData(const CAlignTabExportParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAlignTabExportPage member function declarations

    static bool ShowToolTips();

////@begin CAlignTabExportPage member variables
    CObjectListWidgetSel* m_LocationSel;
    CAlignTabExportParams m_data;
    enum {
        ID_CALIGNTABEXPORTPAGE = 10025,
        ID_PANEL4 = 10077,
        ID_TEXTCTRL14 = 10078,
        ID_BITMAPBUTTON = 10079
    };
////@end CAlignTabExportPage member variables

private:
    string m_RegPath;
    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___ALIGN_TAB_EXPORT_PAGE__HPP
