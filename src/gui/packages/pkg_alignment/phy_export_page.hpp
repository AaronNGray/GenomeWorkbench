#ifndef PKG_ALIGNMENT___PHY_EXPORT_PAGE__HPP
#define PKG_ALIGNMENT___PHY_EXPORT_PAGE__HPP

/*  $Id: phy_export_page.hpp 39318 2017-09-12 16:00:18Z evgeniev $
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
 * Authors:  Roman Katargin, Vladimir Tereshkov
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <gui/widgets/wx/save_file_helper.hpp>
#include <gui/packages/pkg_alignment/phy_export_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPhyExportPage_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPhyExportPage_TITLE _("Tree Export Page")
#define SYMBOL_CPhyExportPage_IDNAME ID_CAGPPEXPORTPAGE
#define SYMBOL_CPhyExportPage_SIZE wxSize(400, 300)
#define SYMBOL_CPhyExportPage_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

class CObjectListWidget;

/*!
 * CPhyExportPage class declaration
 */

class CPhyExportPage: public wxPanel
    , public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( CPhyExportPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPhyExportPage();
    CPhyExportPage( wxWindow* parent, wxWindowID id = SYMBOL_CPhyExportPage_IDNAME, const wxPoint& pos = SYMBOL_CPhyExportPage_POSITION, const wxSize& size = SYMBOL_CPhyExportPage_SIZE, long style = SYMBOL_CPhyExportPage_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPhyExportPage_IDNAME, const wxPoint& pos = SYMBOL_CPhyExportPage_POSITION, const wxSize& size = SYMBOL_CPhyExportPage_SIZE, long style = SYMBOL_CPhyExportPage_STYLE );

    /// Destructor
    ~CPhyExportPage();

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path);
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

////@begin CPhyExportPage event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnButton2Click( wxCommandEvent& event );

////@end CPhyExportPage event handler declarations

////@begin CPhyExportPage member function declarations

    /// Data access
    CPhyExportParams& GetData() { return m_data; }
    const CPhyExportParams& GetData() const { return m_data; }
    void SetData(const CPhyExportParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPhyExportPage member function declarations

    void SetObjects(TConstScopedObjects* objects);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CPhyExportPage member variables
    CObjectListWidget* m_LocationSel;
    /// The data edited by this window
    CPhyExportParams m_data;
    /// Control identifiers
    enum {
        ID_CAGPPEXPORTPAGE = 10025,
        ID_PANEL2 = 10027,
        ID_CHOICE2 = 10029,
        ID_TEXTCTRL4 = 10030,
        ID_CHECKBOX15 = 10028,
        ID_TEXTCTRL5 = 10031,
        ID_BITMAPBUTTON1 = 10032
    };
////@end CPhyExportPage member variables

private:
    string m_RegPath;

    std::unique_ptr<CSaveFileHelper>    m_SaveFile;
};

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___PHY_EXPORT_PAGE__HPP
