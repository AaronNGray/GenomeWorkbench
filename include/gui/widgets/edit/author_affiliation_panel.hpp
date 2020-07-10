#ifndef GUI_WIDGETS_EDIT___AUTHOR_AFFILIATION_PANEL__HPP
#define GUI_WIDGETS_EDIT___AUTHOR_AFFILIATION_PANEL__HPP

/*  $Id: author_affiliation_panel.hpp 37410 2017-01-06 18:49:23Z filippov $
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
#include <objects/biblio/Affil.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include "wx/valtext.h"

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


/*!
 * Control identifiers
 */


////@begin control identifiers
#define SYMBOL_CAUTHORAFFILIATIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CAUTHORAFFILIATIONPANEL_TITLE _("AuthorAffiliationPanel")
#define SYMBOL_CAUTHORAFFILIATIONPANEL_IDNAME ID_CAUTHORAFFILIATIONPANEL
#define SYMBOL_CAUTHORAFFILIATIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CAUTHORAFFILIATIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAuthorAffiliationPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CAuthorAffiliationPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CAuthorAffiliationPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAuthorAffiliationPanel();
    CAuthorAffiliationPanel( wxWindow* parent, const objects::CAffil& affil, wxWindowID id = SYMBOL_CAUTHORAFFILIATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTHORAFFILIATIONPANEL_POSITION, const wxSize& size = SYMBOL_CAUTHORAFFILIATIONPANEL_SIZE, long style = SYMBOL_CAUTHORAFFILIATIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CAUTHORAFFILIATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CAUTHORAFFILIATIONPANEL_POSITION, const wxSize& size = SYMBOL_CAUTHORAFFILIATIONPANEL_SIZE, long style = SYMBOL_CAUTHORAFFILIATIONPANEL_STYLE );

    /// Destructor
    ~CAuthorAffiliationPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void SetAffil(CRef<objects::CAffil> affil);
    CRef<objects::CAffil> GetAffil();

////@begin CAuthorAffiliationPanel event handler declarations

////@end CAuthorAffiliationPanel event handler declarations

////@begin CAuthorAffiliationPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAuthorAffiliationPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAuthorAffiliationPanel member variables
    wxTextCtrl* m_AffilCtrl;
    wxTextCtrl* m_DepartmentCtrl;
    wxTextCtrl* m_StreetCtrl;
    wxTextCtrl* m_CityCtrl;
    wxTextCtrl* m_SubCtrl;
    wxTextCtrl* m_ZipCtrl;
    wxTextCtrl* m_CountryCtrl;
    /// Control identifiers
    enum {
        ID_CAUTHORAFFILIATIONPANEL = 10002,
        ID_AUTH_AFFIL_INST = 10003,
        ID_AUTH_AFFIL_DEPT = 10004,
        ID_AUTH_AFFIL_ADDRESS = 10005,
        ID_AUTH_AFFIL_CITY = 10006,
        ID_AUTH_AFFIL_STATE = 10007,
        ID_AUTH_AFFIL_CODE = 10008,
        ID_AUTH_AFFIL_COUNTRY = 10009
    };
////@end CAuthorAffiliationPanel member variables
private:
    CRef<objects::CAffil> m_Affil;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___AUTHOR_AFFILIATION_PANEL__HPP
