#ifndef GUI_WIDGETS_EDIT___SINGLEAUTHOR_PANEL__HPP
#define GUI_WIDGETS_EDIT___SINGLEAUTHOR_PANEL__HPP

/*  $Id: singleauthor_panel.hpp 43131 2019-05-20 16:34:35Z bollin $
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
#include <gui/gui_export.h>
// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <gui/widgets/edit/large_spin_ctrl.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/hyperlink.h>

class wxChoice;

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CAuthor;
    class CName_std;
END_SCOPE(objects)

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_SINGLEAUTHOR 10025
#define ID_AUTH_FIRST 10026
#define ID_AUTH_MI 10027
#define ID_AUTH_LAST 10028
#define ID_AUTH_SUFFIX 10029
#define ID_AUTH_SWAP_NAME 10030
#define SYMBOL_CSINGLEAUTHORPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEAUTHORPANEL_TITLE _("SingleAuthor")
#define SYMBOL_CSINGLEAUTHORPANEL_IDNAME ID_SINGLEAUTHOR
#define SYMBOL_CSINGLEAUTHORPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEAUTHORPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleAuthorPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSingleAuthorPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleAuthorPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleAuthorPanel();
    CSingleAuthorPanel( wxWindow* parent, objects::CAuthor& author, 
        wxWindowID id = SYMBOL_CSINGLEAUTHORPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLEAUTHORPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSINGLEAUTHORPANEL_SIZE, 
        long style = SYMBOL_CSINGLEAUTHORPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSINGLEAUTHORPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLEAUTHORPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSINGLEAUTHORPANEL_SIZE, 
        long style = SYMBOL_CSINGLEAUTHORPANEL_STYLE );

    /// Destructor
    ~CSingleAuthorPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

    CRef<objects::CAuthor> GetAuthor() const;
    void SetAuthor(const objects::CAuthor &author);

////@begin CSingleAuthorPanel event handler declarations

    void OnFirstNameKillFocus(wxFocusEvent& event);

////@end CSingleAuthorPanel event handler declarations

////@begin CSingleAuthorPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleAuthorPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleAuthorPanel member variables
    wxTextCtrl* m_FirstNameCtrl;
    wxTextCtrl* m_MiddleInitial;
    wxTextCtrl* m_LastNameCtrl;
    wxChoice* m_Suffix;
////@end CSingleAuthorPanel member variables

    void OnMoveAuthorUpdated ( wxCommandEvent& evt );
    void OnSwapName(  wxCommandEvent& evt );
    void HideNonTextCtrls();

    static string s_ParseInitialsFromName(const objects::CName_std& name);
    static bool IsPlaceholder(const objects::CAuthor& author);

private:
    string x_ParseInitialsFromAuthor();

    CRef<objects::CAuthor> m_Author;
    wxString m_FirstName;
    wxString m_LastName;
    CLargeNoTabSpinControl* m_InsertSpinCtrl;
    CLargeNoTabSpinControl* m_MoveSpinCtrl;
    CNoTabBitmapButton* m_SwapButton;
    wxArrayString m_SuffixStrings;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SINGLEAUTHOR_PANEL__HPP
