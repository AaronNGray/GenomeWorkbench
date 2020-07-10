#ifndef GUI_WIDGETS_EDIT___PUBLISHER_PANEL__HPP
#define GUI_WIDGETS_EDIT___PUBLISHER_PANEL__HPP

/*  $Id: publisher_panel.hpp 25034 2012-01-13 16:30:01Z falkrb $
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
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>

#include <objmgr/scope.hpp>

#include <wx/panel.h>
#include <wx/textctrl.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSerialObject;

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_PUBLISHER 10002
#define ID_PUBLISHER_NAME 10003
#define ID_PUBLISHER_ADDRESS 10005
#define ID_PUBLISHER_CITY 10006
#define ID_PUBLISHER_STATE 10007
#define ID_PUBLISHER_CODE 10008
#define ID_PUBLISHER_COUNTRY 10009
#define ID_PUBLISHER_PHONE 10059
#define ID_PUBLISHER_FAX 10060
#define ID_PUBLISHER_EMAIL 10061
#define ID_TEXTCTRL2 10021
#define ID_TEXTCTRL3 10024
#define SYMBOL_CPUBLISHERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPUBLISHERPANEL_TITLE _("PablisherPanel")
#define SYMBOL_CPUBLISHERPANEL_IDNAME ID_PUBLISHER
#define SYMBOL_CPUBLISHERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPUBLISHERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPublisherPanel class declaration
 */

class CPublisherPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CPublisherPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPublisherPanel();
    CPublisherPanel( wxWindow* parent, CSerialObject& object, bool show_pages = false, wxWindowID id = SYMBOL_CPUBLISHERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPUBLISHERPANEL_POSITION, const wxSize& size = SYMBOL_CPUBLISHERPANEL_SIZE, long style = SYMBOL_CPUBLISHERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPUBLISHERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPUBLISHERPANEL_POSITION, const wxSize& size = SYMBOL_CPUBLISHERPANEL_SIZE, long style = SYMBOL_CPUBLISHERPANEL_STYLE );

    /// Destructor
    ~CPublisherPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void PopulatePublisherFields (objects::CImprint& imprint);

////@begin CPublisherPanel event handler declarations

////@end CPublisherPanel event handler declarations

////@begin CPublisherPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPublisherPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CPublisherPanel member variables
    wxFlexGridSizer* m_GridSizer;
    wxTextCtrl* m_AffilCtrl;
    wxTextCtrl* m_StreetCtrl;
    wxTextCtrl* m_CityCtrl;
    wxTextCtrl* m_SubCtrl;
    wxTextCtrl* m_ZipCtrl;
    wxTextCtrl* m_CountryCtrl;
    wxTextCtrl* m_PhoneCtrl;
    wxTextCtrl* m_FaxCtrl;
    wxTextCtrl* m_EmailCtrl;
    wxTextCtrl* m_PubYearCtrl;
    wxTextCtrl* m_CpyYearCtrl;
////@end CPublisherPanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedImprint;
    long m_PubYear;
    long m_CopyrightYear;
    bool m_ShowPages;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___PUBLISHER_PANEL__HPP
