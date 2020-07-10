#ifndef GUI_PKG_SEQ_EDIT___VALID_DISC_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___VALID_DISC_PANEL__HPP
/*  $Id: sub_validate_disc_tab.hpp 43399 2019-06-24 18:58:25Z filippov $
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

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <gui/utils/command_processor.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <wx/event.h>
#include <wx/panel.h>
#include <wx/bookctrl.h>

class wxListbook;

BEGIN_NCBI_SCOPE

class CSubValidatePanel;
class CSubDiscrepancyPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_VALID_DISC_PANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_VALID_DISC_PANEL_TITLE _("Validate and Discrepancy Panel")
#define SYMBOL_VALID_DISC_PANEL_IDNAME ID_VALID_DISC_PANEL
#define SYMBOL_VALID_DISC_PANEL_SIZE wxSize(400, 300)
#define SYMBOL_VALID_DISC_PANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubValidateDiscPanel class declaration
 */

class CSubValidateDiscPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CSubValidateDiscPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubValidateDiscPanel();
    CSubValidateDiscPanel( wxWindow* parent, 
                           IWorkbench* wb,
                           const wxString &dir,
                           objects::CGBProjectHandle* ph,
                           objects::CProjectItem* pi,
                           ICommandProccessor* proc,
                           wxWindowID id = SYMBOL_VALID_DISC_PANEL_IDNAME, 
                           const wxPoint& pos = SYMBOL_VALID_DISC_PANEL_POSITION, 
                           const wxSize& size = SYMBOL_VALID_DISC_PANEL_SIZE, 
                           long style = SYMBOL_VALID_DISC_PANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_VALID_DISC_PANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_VALID_DISC_PANEL_POSITION, 
        const wxSize& size = SYMBOL_VALID_DISC_PANEL_SIZE, 
        long style = SYMBOL_VALID_DISC_PANEL_STYLE );

    /// Destructor
    ~CSubValidateDiscPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();

////@begin CSubValidateDiscPanel event handler declarations
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);
////@end CSubValidateDiscPanel event handler declarations

////@begin CSubValidateDiscPanel member function declarations
    virtual bool StepForward();
    virtual bool StepBackward();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubValidateDiscPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplyCommand();
    virtual bool IsLastPage() const;
    virtual wxString GetAnchor();

    void SetProjectItem(objects::CProjectItem* pi);

private:
    IWorkbench* m_Workbench;
    wxString m_WorkDir;
    objects::CGBProjectHandle* m_ProjectHandle;
    objects::CProjectItem* m_ProjectItem;
    ICommandProccessor* m_CmdProcessor;

////@begin CSubValidateDiscPanel member variables
    wxListbook* m_Listbook;
    CSubValidatePanel* m_ValidatePanel;
    CSubDiscrepancyPanel* m_DiscrepancyPanel;

public:
    /// Control identifiers
    enum {
        ID_VALID_DISC_PANEL = 6670,
        ID_VALID_DISC_LISTBOOK
    };
////@end CSubValidateDiscPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___VALID_DISC_PANEL__HPP
