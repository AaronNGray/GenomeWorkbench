#ifndef GUI_WIDGETS_EDIT___COMMENT_PANEL__HPP
#define GUI_WIDGETS_EDIT___COMMENT_PANEL__HPP

/*  $Id: comment_panel.hpp 39095 2017-07-27 19:56:04Z filippov $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbiobj.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <wx/textctrl.h>

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CScope;
END_SCOPE(objects)

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCOMMENTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCOMMENTPANEL_TITLE _("Comment panel")
#define SYMBOL_CCOMMENTPANEL_IDNAME ID_COMMENTPANEL
#define SYMBOL_CCOMMENTPANEL_SIZE wxDefaultSize
#define SYMBOL_CCOMMENTPANEL_POSITION wxDefaultPosition
////@end control identifiers

class CSerialObject;

/*!
 * CCommentPanel class declaration
 */

class CCommentPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CCommentPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCommentPanel();
    CCommentPanel( wxWindow* parent, CSerialObject& object, 
                   wxWindowID id = SYMBOL_CCOMMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCOMMENTPANEL_POSITION, const wxSize& size = SYMBOL_CCOMMENTPANEL_SIZE, long style = SYMBOL_CCOMMENTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCOMMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCOMMENTPANEL_POSITION, const wxSize& size = SYMBOL_CCOMMENTPANEL_SIZE, long style = SYMBOL_CCOMMENTPANEL_STYLE );

    /// Destructor
    ~CCommentPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCommentPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_COMMENT_BTN
    void OnClearCommentBtnClick( wxCommandEvent& event );

////@end CCommentPanel event handler declarations

////@begin CCommentPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCommentPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCommentPanel member variables
    wxTextCtrl* m_CommentCtrl;
    /// Control identifiers
    enum {
        ID_COMMENTPANEL = 10012,
        ID_TEXTCTRL7 = 10013,
        ID_CLEAR_COMMENT_BTN = 10178
    };
////@end CCommentPanel member variables
private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___COMMENT_PANEL__HPP
