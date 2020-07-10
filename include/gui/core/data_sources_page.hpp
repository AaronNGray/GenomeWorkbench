#ifndef GUI_CORE___DATA_SOURCES_PAGE__HPP
#define GUI_CORE___DATA_SOURCES_PAGE__HPP

/*  $Id: data_sources_page.hpp 25477 2012-03-27 14:53:29Z kuznets $
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

#include <gui/framework/options_dlg_extension.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/panel.h>

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE


////@begin control identifiers
#define SYMBOL_CDATASOURCESPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CDATASOURCESPAGE_TITLE wxT("Data Sources")
#define SYMBOL_CDATASOURCESPAGE_IDNAME ID_CDATASOURCESPAGE
#define SYMBOL_CDATASOURCESPAGE_SIZE wxSize(400, 300)
#define SYMBOL_CDATASOURCESPAGE_POSITION wxDefaultPosition
////@end control identifiers


class CUIDataSourceService;


class CDataSourcesPage: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CDataSourcesPage )
    DECLARE_EVENT_TABLE()

public:
    CDataSourcesPage();
    CDataSourcesPage( wxWindow* parent, wxWindowID id = SYMBOL_CDATASOURCESPAGE_IDNAME, const wxPoint& pos = SYMBOL_CDATASOURCESPAGE_POSITION, const wxSize& size = SYMBOL_CDATASOURCESPAGE_SIZE, long style = SYMBOL_CDATASOURCESPAGE_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDATASOURCESPAGE_IDNAME, const wxPoint& pos = SYMBOL_CDATASOURCESPAGE_POSITION, const wxSize& size = SYMBOL_CDATASOURCESPAGE_SIZE, long style = SYMBOL_CDATASOURCESPAGE_STYLE );

    ~CDataSourcesPage();

    void SetService (CUIDataSourceService* service) { m_Service = service; }

    void Init();

    void CreateControls();

////@begin CDataSourcesPage event handler declarations

////@end CDataSourcesPage event handler declarations

////@begin CDataSourcesPage member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CDataSourcesPage member function declarations

    static bool ShowToolTips();

////@begin CDataSourcesPage member variables
    enum {
        ID_CDATASOURCESPAGE = 10044,
        ID_CHOICE2 = 10045
    };
////@end CDataSourcesPage member variables

private:
    CUIDataSourceService* m_Service;
};


class CDataSourcesOptionsDlgExtension :
        public CObject,
        public IExtension,
        public IOptionsDlgExtension
{
public:
    CDataSourcesOptionsDlgExtension(CUIDataSourceService* service) : m_Service(service) {}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const
        { return "options_dlg_page_data_sources"; }
    virtual string  GetExtensionLabel() const
        { return "Data Sources Page"; }
    /// @}

    virtual wxWindow* CreateSettingsPage(wxWindow* parent)
    {
        CDataSourcesPage* page = new CDataSourcesPage();
        page->SetService(m_Service);
        page->Create(parent);
        return page;
    }

    virtual size_t GetPriority() const { return 3; }
    virtual string GetPageLabel() const { return ToStdString(SYMBOL_CDATASOURCESPAGE_TITLE); }

private:
    CUIDataSourceService* m_Service;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___DATA_SOURCES_PAGE__HPP
