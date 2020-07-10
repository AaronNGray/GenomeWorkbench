#ifndef GUI_PKG_SEQ_EDIT___SUBDISCREPANCY_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SUBDISCREPANCY_PANEL__HPP

/*  $Id: subdiscrepancy_panel.hpp 43399 2019-06-24 18:58:25Z filippov $
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
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <gui/packages/pkg_sequence_edit/discrepancy_dlg.hpp>

#include <wx/panel.h>


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBDISCREPANCYPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBDISCREPANCYPANEL_TITLE _("Submission Discrepancy Panel")
#define SYMBOL_CSUBDISCREPANCYPANEL_IDNAME ID_CSUBDISCREPANCYPANEL
#define SYMBOL_CSUBDISCREPANCYPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBDISCREPANCYPANEL_POSITION wxDefaultPosition
////@end control identifiers

class IWorkbench;
class CDiscrepancyPanel;

/*!
 * CSubDiscrepancyPanel class declaration
 */

class CSubDiscrepancyPanel: public wxPanel, public ISubmissionPage, public CDiscrepancyPanelHost
{    
    DECLARE_DYNAMIC_CLASS( CSubDiscrepancyPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubDiscrepancyPanel();
    CSubDiscrepancyPanel( wxWindow* parent, 
                          IWorkbench* wb,
                          const wxString &dir,
                          CGBProjectHandle* ph,
                          ICommandProccessor* proc,
                          wxWindowID id = SYMBOL_CSUBDISCREPANCYPANEL_IDNAME, 
                          const wxPoint& pos = SYMBOL_CSUBDISCREPANCYPANEL_POSITION, 
                          const wxSize& size = SYMBOL_CSUBDISCREPANCYPANEL_SIZE, 
                          long style = SYMBOL_CSUBDISCREPANCYPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBDISCREPANCYPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBDISCREPANCYPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBDISCREPANCYPANEL_SIZE, 
        long style = SYMBOL_CSUBDISCREPANCYPANEL_STYLE );

    /// Destructor
    ~CSubDiscrepancyPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();

////@begin CSubDiscrepancyPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CSUBDISCREPANCY_REFRESH
    void OnRefreshClick( wxCommandEvent& event );

////@end CSubDiscrepancyPanel event handler declarations

////@begin CSubDiscrepancyPanel member function declarations

    virtual bool IsLastPage() const;
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubDiscrepancyPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSubDiscrepancyPanel member variables
    /// Control identifiers
    enum {
        ID_CSUBDISCREPANCYPANEL = 6700,
        ID_CSUBDISCREPANCY_REFRESH
    };
////@end CSubDiscrepancyPanel member variables

    virtual wxString GetAnchor() { return _("valiodation-submitter-report"); }

private:
    CDiscrepancyPanel* m_Panel;
    wxStaticText* m_Modified;
    IWorkbench* m_Workbench;
    wxString m_WorkDir;
    CGBProjectHandle* m_ProjectHandle;
    ICommandProccessor* m_CmdProcessor;
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SUBDISCREPANCY_PANEL__HPP
