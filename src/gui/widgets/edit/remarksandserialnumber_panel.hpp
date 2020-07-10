#ifndef GUI_WIDGETS_EDIT___REMARKSANDSERIALNUMBER_PANEL__HPP
#define GUI_WIDGETS_EDIT___REMARKSANDSERIALNUMBER_PANEL__HPP

/*  $Id: remarksandserialnumber_panel.hpp 38871 2017-06-28 14:47:03Z bollin $
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

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSerialObject;


/*!
 * Control identifiers
 */


////@begin control identifiers
#define SYMBOL_CREMARKSANDSERIALNUMBERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CREMARKSANDSERIALNUMBERPANEL_TITLE _("RemarksAndSerialNumber")
#define SYMBOL_CREMARKSANDSERIALNUMBERPANEL_IDNAME ID_REMARKSANDSERIALNUMBER
#define SYMBOL_CREMARKSANDSERIALNUMBERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CREMARKSANDSERIALNUMBERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRemarksAndSerialNumberPanel class declaration
 */

class CRemarksAndSerialNumberPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CRemarksAndSerialNumberPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemarksAndSerialNumberPanel();
    CRemarksAndSerialNumberPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_POSITION, const wxSize& size = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_SIZE, long style = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_POSITION, const wxSize& size = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_SIZE, long style = SYMBOL_CREMARKSANDSERIALNUMBERPANEL_STYLE );

    /// Destructor
    ~CRemarksAndSerialNumberPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRemarksAndSerialNumberPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonClick( wxCommandEvent& event );

////@end CRemarksAndSerialNumberPanel event handler declarations

////@begin CRemarksAndSerialNumberPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRemarksAndSerialNumberPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();


////@begin CRemarksAndSerialNumberPanel member variables
    wxTextCtrl* m_Comment;
    wxBoxSizer* m_DOISizer;
    wxTextCtrl* m_DOI;
    /// Control identifiers
    enum {
        ID_REMARKSANDSERIALNUMBER = 10075,
        ID_REMARK = 10076,
        ID_TEXTCTRL4 = 10010,
        ID_BUTTON = 10012
    };
////@end CRemarksAndSerialNumberPanel member variables
private:
    CSerialObject* m_Object;
    CRef<objects::CScope> m_Scope;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___REMARKSANDSERIALNUMBER_PANEL__HPP
