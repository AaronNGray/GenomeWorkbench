#ifndef GUI_WIDGETS_EDIT___SUBMITTER_PANEL__HPP
#define GUI_WIDGETS_EDIT___SUBMITTER_PANEL__HPP

/*  $Id: submitter_panel_depr.hpp 42777 2019-04-11 15:54:35Z asztalos $
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
 * Authors:  Andrea Asztalos
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
#define SYMBOL_CSubmitterPanel_Depr_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSubmitterPanel_Depr_TITLE _("SubmitterPanel")
#define SYMBOL_CSubmitterPanel_Depr_IDNAME ID_CSubmitterPanel_Depr
#define SYMBOL_CSubmitterPanel_Depr_SIZE wxSize(400, 300)
#define SYMBOL_CSubmitterPanel_Depr_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CContactPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSubmitterPanel_Depr: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS(CSubmitterPanel_Depr)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubmitterPanel_Depr();
    CSubmitterPanel_Depr(
        wxWindow* parent, 
        objects::CContact_info& object, 
        wxWindowID id = SYMBOL_CSubmitterPanel_Depr_IDNAME,
        const wxPoint& pos = SYMBOL_CSubmitterPanel_Depr_POSITION,
        const wxSize& size = SYMBOL_CSubmitterPanel_Depr_SIZE,
        long style = SYMBOL_CSubmitterPanel_Depr_STYLE);

    /// Creation
    bool Create( 
        wxWindow* parent, 
        wxWindowID id = SYMBOL_CSubmitterPanel_Depr_IDNAME,
        const wxPoint& pos = SYMBOL_CSubmitterPanel_Depr_POSITION,
        const wxSize& size = SYMBOL_CSubmitterPanel_Depr_SIZE,
        long style = SYMBOL_CSubmitterPanel_Depr_STYLE);

    /// Destructor
    ~CSubmitterPanel_Depr();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    //bool IsContactNameSet() const;
    //bool IsOneEmailSet() const;

    void SetData(const objects::CContact_info& contact);

    void SetFocusOnFirstName();
    void SetFocusOnLastName();
    void SetFocusOnFirstEmail();

////@begin CSubmitterPanel_Depr event handler declarations

////@end CSubmitterPanel_Depr event handler declarations

////@begin CSubmitterPanel_Depr member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubmitterPanel_Depr member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    enum {
        ID_CSubmitterPanel_Depr = 10053,
        ID_TXT_SUBMITTER1,
        ID_TXT_SUBMITTER2,
        ID_TXT_SUBMITTER3,
        ID_SUBMITTER_CHOICE,
        ID_TXT_SUBMITTER4,
        ID_TXT_SUBMITTER5
    };
private:
    wxTextCtrl* m_First;
    wxTextCtrl* m_MI;
    wxTextCtrl* m_Last;
    wxChoice* m_Suffix;
    wxTextCtrl* m_EmailPrimary;
    wxTextCtrl* m_EmailSecondary;

    CRef<objects::CContact_info> m_Contact;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___SUBMITTER_PANEL__HPP
