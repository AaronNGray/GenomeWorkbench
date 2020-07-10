#ifndef GUI_WIDGETS_EDIT___SUBMIT_REMARKS_PANEL__HPP
#define GUI_WIDGETS_EDIT___SUBMIT_REMARKS_PANEL__HPP

/*  $Id: submit_remarks_panel.hpp 39708 2017-10-27 14:03:13Z filippov $
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
 * Authors:  Igor Filippov
 */

#include <corelib/ncbistd.hpp>
#include <objects/submit/Submit_block.hpp>
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
#define SYMBOL_SUBMIT_REMARKS_PANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_SUBMIT_REMARKS_PANEL_TITLE _("Remarks")
#define SYMBOL_SUBMIT_REMARKS_PANEL_IDNAME ID_SUBMIT_REMARKS_PANEL
#define SYMBOL_SUBMIT_REMARKS_PANEL_SIZE wxSize(400, 300)
#define SYMBOL_SUBMIT_REMARKS_PANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubmitRemarksPanel class declaration
 */

class CSubmitRemarksPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSubmitRemarksPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitRemarksPanel();
    CSubmitRemarksPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_SUBMIT_REMARKS_PANEL_IDNAME, const wxPoint& pos = SYMBOL_SUBMIT_REMARKS_PANEL_POSITION, const wxSize& size = SYMBOL_SUBMIT_REMARKS_PANEL_SIZE, long style = SYMBOL_SUBMIT_REMARKS_PANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SUBMIT_REMARKS_PANEL_IDNAME, const wxPoint& pos = SYMBOL_SUBMIT_REMARKS_PANEL_POSITION, const wxSize& size = SYMBOL_SUBMIT_REMARKS_PANEL_SIZE, long style = SYMBOL_SUBMIT_REMARKS_PANEL_STYLE );

    /// Destructor
    ~CSubmitRemarksPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRemarksAndSerialNumberPanel event handler declarations

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
    /// Control identifiers
    enum {
        ID_SUBMIT_REMARKS_PANEL = 10075,
        ID_REMARK = 10076,
    };
////@end CRemarksAndSerialNumberPanel member variables
private:
    objects::CSubmit_block* m_Object;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SUBMIT_REMARKS_PANEL__HPP
