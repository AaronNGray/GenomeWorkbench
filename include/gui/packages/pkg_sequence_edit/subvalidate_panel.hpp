#ifndef GUI_PKG_SEQ_EDIT___SUBVALIDATE_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SUBVALIDATE_PANEL__HPP

/*  $Id: subvalidate_panel.hpp 43399 2019-06-24 18:58:25Z filippov $
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
#include <gui/objects/GBProjectHandle.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <wx/panel.h>


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBVALIDATEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBVALIDATEPANEL_TITLE _("Submission Validate Panel")
#define SYMBOL_CSUBVALIDATEPANEL_IDNAME ID_CSUBVALIDATEPANEL
#define SYMBOL_CSUBVALIDATEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBVALIDATEPANEL_POSITION wxDefaultPosition
////@end control identifiers

class IWorkbench;
class CValidatePanel;

/*!
 * CSubValidatePanel class declaration
 */

class CSubValidatePanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CSubValidatePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubValidatePanel();
    CSubValidatePanel( wxWindow* parent, IWorkbench* wb,
                       objects::CGBProjectHandle* ph,
                       objects::CProjectItem* pi,
                       wxWindowID id = SYMBOL_CSUBVALIDATEPANEL_IDNAME, 
                       const wxPoint& pos = SYMBOL_CSUBVALIDATEPANEL_POSITION, 
                       const wxSize& size = SYMBOL_CSUBVALIDATEPANEL_SIZE, 
                       long style = SYMBOL_CSUBVALIDATEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBVALIDATEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBVALIDATEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBVALIDATEPANEL_SIZE, 
        long style = SYMBOL_CSUBVALIDATEPANEL_STYLE );

    /// Destructor
    ~CSubValidatePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();

////@begin CSubValidatePanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBVALIDATE_REFRESH
    void OnValidateClick( wxCommandEvent& event );

////@end CSubValidatePanel event handler declarations

////@begin CSubValidatePanel member function declarations

    virtual bool IsLastPage() const;
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubValidatePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubValidatePanel member variables
    /// Control identifiers
    enum {
        ID_CSUBVALIDATEPANEL = 6700,
        ID_CSUBVALIDATE_REFRESH
    };
////@end CSubValidatePanel member variables

    void SetProjectItem(objects::CProjectItem* pi) {m_ProjectItem = pi;}

    virtual wxString GetAnchor() { return _("validation-validate"); }

private:
    IWorkbench* m_Workbench;
    objects::CGBProjectHandle* m_ProjectHandle;
    objects::CProjectItem* m_ProjectItem;
    CValidatePanel* m_Panel;
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SUBVALIDATE_PANEL__HPP
