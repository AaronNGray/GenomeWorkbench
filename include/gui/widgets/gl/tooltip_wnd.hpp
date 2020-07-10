#ifndef GUI_WIDGETS_GL___TOOLTIP_WND__HPP
#define GUI_WIDGETS_GL___TOOLTIP_WND__HPP

/*  $Id: tooltip_wnd.hpp 22678 2010-12-10 17:44:07Z katargir $
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
 * Authors:  
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/frame.h>

class wxStaticText;

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class CTooltipWnd;
////@end forward declarations

class CTooltip;

/*!
 * Control identifiers
 */

////@begin control identifiers
 //wxNO_BORDER|wxTAB_TRAVERSAL
// FRAME_TOOL_WINDOW keeps it from taking focus. Without that calling Raise() on the ur-parent
// of the gl window was the only way to keep focus on MAC (but that causes a flash).
#define SYMBOL_CTOOLTIPWND_STYLE wxFRAME_TOOL_WINDOW|wxFRAME_FLOAT_ON_PARENT|wxFRAME_NO_TASKBAR
#define SYMBOL_CTOOLTIPWND_IDNAME ID_CTOOLTIPWND
#define SYMBOL_CTOOLTIPWND_SIZE wxSize(0, 0)
#define SYMBOL_CTOOLTIPWND_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CTooltipWnd class declaration
 */

class CTooltipWnd: public wxFrame
{    
    DECLARE_DYNAMIC_CLASS( CTooltipWnd )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTooltipWnd();
    CTooltipWnd(CTooltip* tooltip);

    /// Creation
    bool Create(wxWindow* parent, 
                wxWindowID id = ID_CTOOLTIPWND, 
                const wxString& title = wxEmptyString,
                const wxPoint& pos = SYMBOL_CTOOLTIPWND_POSITION, 
                const wxSize& size = SYMBOL_CTOOLTIPWND_SIZE,
                long style = SYMBOL_CTOOLTIPWND_STYLE);

    /// Destructor
    ~CTooltipWnd();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTooltipWnd event handler declarations

    /// All mouse events event handler for ID_CTOOLTIPWND
    void OnMouse( wxMouseEvent& event );

////@end CTooltipWnd event handler declarations

////@begin CTooltipWnd member function declarations

    wxString GetTooltipText() const { return m_TooltipText ; }
    void SetTooltipText(wxString value) { m_TooltipText = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTooltipWnd member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTooltipWnd member variables
    wxStaticText* m_TooltipCtrl;
private:
    wxString m_TooltipText;
    /// Control identifiers
    enum {
        ID_CTOOLTIPWND = 10000,
        ID_PANEL1 = 10002
    };
////@end CTooltipWnd member variables

    CTooltip* m_Tooltip;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL___TOOLTIP_WND__HPP
