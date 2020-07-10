#ifndef GUI_WIDGETS_EDIT___CITSUB_PANEL__HPP
#define GUI_WIDGETS_EDIT___CITSUB_PANEL__HPP

/*  $Id: citsub_panel.hpp 34283 2015-12-08 21:44:29Z filippov $
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

/*!
 * Includes
 */

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class CSerialObject;

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_CITSUB 10088
#define ID_CITSUB_DESC 10089
#define ID_CITSUB_ADD_STANDARD_REMARK 10090
#define SYMBOL_CCITSUBPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCITSUBPANEL_TITLE _("CitSub")
#define SYMBOL_CCITSUBPANEL_IDNAME ID_CITSUB
#define SYMBOL_CCITSUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCITSUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCitSubPanel class declaration
 */

class CCitSubPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CCitSubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCitSubPanel();
    CCitSubPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CCITSUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCITSUBPANEL_POSITION, const wxSize& size = SYMBOL_CCITSUBPANEL_SIZE, long style = SYMBOL_CCITSUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCITSUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCITSUBPANEL_POSITION, const wxSize& size = SYMBOL_CCITSUBPANEL_SIZE, long style = SYMBOL_CCITSUBPANEL_STYLE );
    CRef<objects::CCit_sub> GetCit_sub() const;

    /// Destructor
    ~CCitSubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCitSubPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CITSUB_ADD_STANDARD_REMARK
    void OnCitsubAddStandardRemarkSelected( wxCommandEvent& event );

////@end CCitSubPanel event handler declarations

////@begin CCitSubPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCitSubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
////@begin CCitSubPanel member variables
    wxTextCtrl* m_Remark;
    wxChoice* m_StandardRemark;
////@end CCitSubPanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedCitSub;
    wxWindow* m_date_ctrl;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___CITSUB_PANEL__HPP
