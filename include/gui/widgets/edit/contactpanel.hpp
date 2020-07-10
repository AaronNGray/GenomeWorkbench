#ifndef GUI_WIDGETS_EDIT___CONTACTPANEL__HPP
#define GUI_WIDGETS_EDIT___CONTACTPANEL__HPP

/*  $Id: contactpanel.hpp 42280 2019-01-24 20:12:34Z asztalos $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <serial/serialbase.hpp>

#include <wx/window.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/choice.h>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CContact_info;
END_SCOPE(objects);


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCONTACTPANEL 10053
#define ID_TXT_CONTACT1 10054
#define ID_TXT_CONTACT2 10055
#define ID_TXT_CONTACT3 10056
#define ID_CHOICE 10057
#define ID_TXT_CONTACT4 10062
#define ID_TXT_CONTACT5 10063
#define ID_TXT_CONTACT6 10064
#define ID_BTN_CONTACT7 10065
#define SYMBOL_CCONTACTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCONTACTPANEL_TITLE _("ContactPanel")
#define SYMBOL_CCONTACTPANEL_IDNAME ID_CCONTACTPANEL
#define SYMBOL_CCONTACTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCONTACTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CContactPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CContactPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CContactPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CContactPanel();
    CContactPanel( 
        wxWindow* parent, 
        objects::CContact_info& object, 
        wxWindowID id = SYMBOL_CCONTACTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCONTACTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCONTACTPANEL_SIZE, 
        long style = SYMBOL_CCONTACTPANEL_STYLE );

    /// Creation
    bool Create( 
        wxWindow* parent, 
        wxWindowID id = SYMBOL_CCONTACTPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCONTACTPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCONTACTPANEL_SIZE, 
        long style = SYMBOL_CCONTACTPANEL_STYLE );

    /// Destructor
    ~CContactPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void SetData(const objects::CContact_info& contact);

////@begin CContactPanel event handler declarations

////@end CContactPanel event handler declarations

////@begin CContactPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CContactPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
    wxTextCtrl* m_First;
    wxTextCtrl* m_MI;
    wxTextCtrl* m_Last;
    wxChoice* m_Suffix;
    wxTextCtrl* m_Phone;
    wxTextCtrl* m_Fax;

    CRef<objects::CContact_info> m_Contact;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___CONTACTPANEL__HPP
