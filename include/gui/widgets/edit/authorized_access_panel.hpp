/*  $Id: authorized_access_panel.hpp 31212 2014-09-12 12:45:01Z bollin $
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
#ifndef _AUTHORIZED_ACCESS_PANEL_H_
#define _AUTHORIZED_ACCESS_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <objects/general/User_object.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <wx/panel.h>
#include <wx/textctrl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CAUTHORIZEDACCESSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CAUTHORIZEDACCESSPANEL_TITLE _("AuthorizedAccessPanel")
#define SYMBOL_CAUTHORIZEDACCESSPANEL_IDNAME ID_CAUTHORIZEDACCESSPANEL
#define SYMBOL_CAUTHORIZEDACCESSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CAUTHORIZEDACCESSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAuthorizedAccessPanel class declaration
 */

class CAuthorizedAccessPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CAuthorizedAccessPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAuthorizedAccessPanel();
    CAuthorizedAccessPanel( wxWindow* parent, wxWindowID id = SYMBOL_CAUTHORIZEDACCESSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTHORIZEDACCESSPANEL_POSITION, const wxSize& size = SYMBOL_CAUTHORIZEDACCESSPANEL_SIZE, long style = SYMBOL_CAUTHORIZEDACCESSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAUTHORIZEDACCESSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTHORIZEDACCESSPANEL_POSITION, const wxSize& size = SYMBOL_CAUTHORIZEDACCESSPANEL_SIZE, long style = SYMBOL_CAUTHORIZEDACCESSPANEL_STYLE );

    /// Destructor
    ~CAuthorizedAccessPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAuthorizedAccessPanel event handler declarations

////@end CAuthorizedAccessPanel event handler declarations

////@begin CAuthorizedAccessPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAuthorizedAccessPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAuthorizedAccessPanel member variables
    wxTextCtrl* m_StudyTxt;
    /// Control identifiers
    enum {
        ID_CAUTHORIZEDACCESSPANEL = 10166,
        ID_TEXTCTRL22 = 10167
    };
////@end CAuthorizedAccessPanel member variables
    void SetUser_object(CRef<objects::CUser_object> user);
    CRef<objects::CUser_object> GetUser_object();

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

private:
    CRef<objects::CUser_object> m_User;
};

END_NCBI_SCOPE

#endif
    // _AUTHORIZED_ACCESS_PANEL_H_
