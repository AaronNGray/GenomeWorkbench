#ifndef GUI_WIDGETS_EDIT___PATENT_PANEL__HPP
#define GUI_WIDGETS_EDIT___PATENT_PANEL__HPP

/*  $Id: patent_panel.hpp 34283 2015-12-08 21:44:29Z filippov $
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
////@end forward declarations

BEGIN_NCBI_SCOPE

class CSerialObject;


/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_PATENT 10000
#define ID_PATENT_TITLE 10001
#define ID_PATENT_ABSTRACT 10045
#define ID_PATENT_COUNTRY 10052
#define ID_PATENT_DOCTYPE 10080
#define ID_PATENT_DOCNUM 10081
#define ID_PATENT_APPNUM 10083
#define SYMBOL_CPATENTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPATENTPANEL_TITLE _("Patent")
#define SYMBOL_CPATENTPANEL_IDNAME ID_PATENT
#define SYMBOL_CPATENTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPATENTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPatentPanel class declaration
 */

class CPatentPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CPatentPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPatentPanel();
    CPatentPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CPATENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPATENTPANEL_POSITION, const wxSize& size = SYMBOL_CPATENTPANEL_SIZE, long style = SYMBOL_CPATENTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPATENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPATENTPANEL_POSITION, const wxSize& size = SYMBOL_CPATENTPANEL_SIZE, long style = SYMBOL_CPATENTPANEL_STYLE );

    CRef<objects::CCit_pat> GetCit_pat(void) const;

    /// Destructor
    ~CPatentPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CPatentPanel event handler declarations

////@end CPatentPanel event handler declarations

////@begin CPatentPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPatentPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CPatentPanel member variables
////@end CPatentPanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedPatent;
    wxWindow* m_issuedate_ctrl;
    wxWindow* m_appdate_ctrl;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___PATENT_PANEL__HPP
