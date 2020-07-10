/////////////////////////////////////////////////////////////////////////////
// Name:        demo_align_table_frame.hpp
// Purpose:
// Author:      Yury Voronov
// Modified by:
// Created:     17/01/2008 21:42:34
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _DEMO_ALIGN_TABLE_FRAME_H_
#define _DEMO_ALIGN_TABLE_FRAME_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/listctrl.h"
////@end includes

#include <corelib/ncbiobj.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXDEMOALIGNTABLEFRAME 10000
#define wxID_LOAD 10002
#define ID_SEGMENTS 10005
#define ID_SPANS 10006
#define ID_ALIGNTABLE 10001
#define SYMBOL_CWXDEMOALIGNTABLEFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CWXDEMOALIGNTABLEFRAME_TITLE _("Align Table Demo")
#define SYMBOL_CWXDEMOALIGNTABLEFRAME_IDNAME ID_CWXDEMOALIGNTABLEFRAME
#define SYMBOL_CWXDEMOALIGNTABLEFRAME_SIZE wxSize(400, 300)
#define SYMBOL_CWXDEMOALIGNTABLEFRAME_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CwxDemoAlignTableFrame class declaration
 */

class CwxDemoAlignTableFrame: public wxFrame
{
    DECLARE_CLASS( CwxDemoAlignTableFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxDemoAlignTableFrame();
    CwxDemoAlignTableFrame( wxWindow* parent, wxWindowID id = SYMBOL_CWXDEMOALIGNTABLEFRAME_IDNAME, const wxString& caption = SYMBOL_CWXDEMOALIGNTABLEFRAME_TITLE, const wxPoint& pos = SYMBOL_CWXDEMOALIGNTABLEFRAME_POSITION, const wxSize& size = SYMBOL_CWXDEMOALIGNTABLEFRAME_SIZE, long style = SYMBOL_CWXDEMOALIGNTABLEFRAME_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXDEMOALIGNTABLEFRAME_IDNAME, const wxString& caption = SYMBOL_CWXDEMOALIGNTABLEFRAME_TITLE, const wxPoint& pos = SYMBOL_CWXDEMOALIGNTABLEFRAME_POSITION, const wxSize& size = SYMBOL_CWXDEMOALIGNTABLEFRAME_SIZE, long style = SYMBOL_CWXDEMOALIGNTABLEFRAME_STYLE );

    /// Destructor
    ~CwxDemoAlignTableFrame();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CwxDemoAlignTableFrame event handler declarations

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
    void OnOpenClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_LOAD
    void OnLoadClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_SEGMENTS
    void OnSegmentsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_SPANS
    void OnSpansClick( wxCommandEvent& event );

////@end CwxDemoAlignTableFrame event handler declarations

////@begin CwxDemoAlignTableFrame member function declarations

    wxString GetAccession() const { return m_Accession ; }
    void SetAccession(wxString value) { m_Accession = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxDemoAlignTableFrame member function declarations

    bool LoadAccession( const wxString& anAccName );
    bool LoadFile( const wxString& aFileName );

    void InitData();

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CwxDemoAlignTableFrame member variables
    wxListCtrl* m_AlignTableCtrl;
private:
    wxString m_Accession;
    CRef<objects::CObjectManager> m_ObjectManager;
    bool m_Segments;
    CRef<CScope> m_Scope;
    CRef<CSeq_id> m_Seq_id;
////@end CwxDemoAlignTableFrame member variables
};

END_NCBI_SCOPE

#endif // _DEMO_ALIGN_TABLE_FRAME_H_
