/*  $Id: unverified_panel.hpp 40461 2018-02-20 18:48:16Z filippov $
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
#ifndef _UNVERIFIED_PANEL_H_
#define _UNVERIFIED_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <gui/widgets/edit/utilities.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/checkbox.h>

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
#define SYMBOL_CUNVERIFIEDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CUNVERIFIEDPANEL_TITLE _("UnverifiedPanel")
#define SYMBOL_CUNVERIFIEDPANEL_IDNAME ID_CUNVERIFIEDPANEL
#define SYMBOL_CUNVERIFIEDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CUNVERIFIEDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CUnverifiedPanel class declaration
 */

class CUnverifiedPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CUnverifiedPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CUnverifiedPanel();
    CUnverifiedPanel( wxWindow* parent, wxWindowID id = SYMBOL_CUNVERIFIEDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CUNVERIFIEDPANEL_POSITION, const wxSize& size = SYMBOL_CUNVERIFIEDPANEL_SIZE, long style = SYMBOL_CUNVERIFIEDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CUNVERIFIEDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CUNVERIFIEDPANEL_POSITION, const wxSize& size = SYMBOL_CUNVERIFIEDPANEL_SIZE, long style = SYMBOL_CUNVERIFIEDPANEL_STYLE );

    /// Destructor
    ~CUnverifiedPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CUnverifiedPanel event handler declarations

////@end CUnverifiedPanel event handler declarations

////@begin CUnverifiedPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CUnverifiedPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CUnverifiedPanel member variables
    wxCheckBox* m_Organism;
    wxCheckBox* m_Features;
    wxCheckBox* m_Misassembled;
    wxCheckBox* m_Contamination;
    /// Control identifiers
    enum {
        ID_CUNVERIFIEDPANEL = 10133,
        ID_CHECKBOX14 = 10134,
        ID_CHECKBOX15 = 10135,
        ID_CHECKBOX16 = 10136,
	ID_CHECKBOX17 = 10137
    };
////@end CUnverifiedPanel member variables
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
// _UNVERIFIED_PANEL_H_

