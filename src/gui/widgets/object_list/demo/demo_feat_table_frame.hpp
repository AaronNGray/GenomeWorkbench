/////////////////////////////////////////////////////////////////////////////
// Name:        demo_object_list_frame.hpp
// Purpose:
// Author:      Yury Voronov
// Modified by:
// Created:     03/01/2008 15:26:43
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _DEMO_OBJECT_LIST_FRAME_H_
#define _DEMO_OBJECT_LIST_FRAME_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/listctrl.h"
////@end includes

#include <corelib/ncbiobj.hpp>
#include <objmgr/object_manager.hpp>

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class CFeatTableWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXDEMOFEATTABLERAME 10000
#define wxID_LOAD 10002
#define ID_FEATTABLE 10001
#define SYMBOL_CWXDEMOFEATURETABLEFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CWXDEMOFEATURETABLEFRAME_TITLE _("Feature Table Demo")
#define SYMBOL_CWXDEMOFEATURETABLEFRAME_IDNAME ID_CWXDEMOFEATTABLERAME
#define SYMBOL_CWXDEMOFEATURETABLEFRAME_SIZE wxSize(400, 300)
#define SYMBOL_CWXDEMOFEATURETABLEFRAME_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxDemoFeatureTableFrame class declaration
 */

class CwxDemoFeatureTableFrame: public wxFrame
{
    DECLARE_CLASS( CwxDemoFeatureTableFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxDemoFeatureTableFrame();
    CwxDemoFeatureTableFrame( wxWindow* parent, wxWindowID id = SYMBOL_CWXDEMOFEATURETABLEFRAME_IDNAME, const wxString& caption = SYMBOL_CWXDEMOFEATURETABLEFRAME_TITLE, const wxPoint& pos = SYMBOL_CWXDEMOFEATURETABLEFRAME_POSITION, const wxSize& size = SYMBOL_CWXDEMOFEATURETABLEFRAME_SIZE, long style = SYMBOL_CWXDEMOFEATURETABLEFRAME_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXDEMOFEATURETABLEFRAME_IDNAME, const wxString& caption = SYMBOL_CWXDEMOFEATURETABLEFRAME_TITLE, const wxPoint& pos = SYMBOL_CWXDEMOFEATURETABLEFRAME_POSITION, const wxSize& size = SYMBOL_CWXDEMOFEATURETABLEFRAME_SIZE, long style = SYMBOL_CWXDEMOFEATURETABLEFRAME_STYLE );

    /// Destructor
    ~CwxDemoFeatureTableFrame();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxDemoFeatureTableFrame event handler declarations

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
    void OnOpenClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_LOAD
    void OnLoadClick( wxCommandEvent& event );

////@end CwxDemoFeatureTableFrame event handler declarations

////@begin CwxDemoFeatureTableFrame member function declarations

    wxString GetAccession() const { return m_Accession ; }
    void SetAccession(wxString value) { m_Accession = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxDemoFeatureTableFrame member function declarations

    bool LoadAccession( const wxString& anAccName );
    bool LoadFile( const wxString& aFileName );

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CwxDemoFeatureTableFrame member variables
    CFeatTableWidget* m_FeatTableCtrl;
private:
    wxString m_Accession;
    CRef<objects::CObjectManager> m_ObjectManager;
////@end CwxDemoFeatureTableFrame member variables
};

END_NCBI_SCOPE

#endif
    // _DEMO_OBJECT_LIST_FRAME_H_
