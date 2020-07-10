#ifndef GUI_WIDGETS_EDIT___FEAT_COMMENT_PANEL__HPP
#define GUI_WIDGETS_EDIT___FEAT_COMMENT_PANEL__HPP

/*  $Id: feat_comment.hpp 36964 2016-11-18 14:19:00Z filippov $
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

#include <corelib/ncbiobj.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <wx/checkbox.h>


////@begin includes
////@end includes

BEGIN_NCBI_SCOPE


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFEATCOMMENTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFEATCOMMENTPANEL_TITLE _("Comment Feature panel")
#define SYMBOL_CFEATCOMMENTPANEL_IDNAME wxID_ANY
#define SYMBOL_CFEATCOMMENTPANEL_SIZE wxDefaultSize
#define SYMBOL_CFEATCOMMENTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFeatCommentPanel class declaration
 */

class CFeatCommentPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CFeatCommentPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFeatCommentPanel();
    CFeatCommentPanel( wxWindow* parent, const wxString& comment,
                   wxWindowID id = SYMBOL_CFEATCOMMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFEATCOMMENTPANEL_POSITION, const wxSize& size = SYMBOL_CFEATCOMMENTPANEL_SIZE, long style = SYMBOL_CFEATCOMMENTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEATCOMMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFEATCOMMENTPANEL_POSITION, const wxSize& size = SYMBOL_CFEATCOMMENTPANEL_SIZE, long style = SYMBOL_CFEATCOMMENTPANEL_STYLE );

    /// Destructor
    ~CFeatCommentPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFeatCommentPanel event handler declarations

////@end CFeatCommentPanel event handler declarations

////@begin CFeatCommentPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeatCommentPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    bool GetConvertToMiscFeat(void) {return m_ConvertToMiscFeat && m_ConvertToMiscFeat->GetValue();}
private:
////@begin CFeatCommentPanel member variables
    wxCheckBox* m_ConvertToMiscFeat;
    wxString m_comment;
////@end CFeatCommentPanel member variables
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___FEAT_COMMENT_PANEL__HPP
