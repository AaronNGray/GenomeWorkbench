#ifndef GUI_PKG_SEQ_EDIT___ORGINFO_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___ORGINFO_PANEL__HPP
/*  $Id: orginfo_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <objmgr/seq_entry_handle.hpp>
#include <gui/framework/workbench.hpp>


#include <wx/event.h>
#include <wx/panel.h>
#include <wx/bookctrl.h>

class wxListbook;

BEGIN_NCBI_SCOPE

class COrgGeneralPanel;
class COrgAdvancedPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CORGANISMINFOPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CORGANISMINFOPANEL_TITLE _("Organism Info Panel")
#define SYMBOL_CORGANISMINFOPANEL_IDNAME ID_CORGANISMINFOPANEL
#define SYMBOL_CORGANISMINFOPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CORGANISMINFOPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * COrganismInfoPanel class declaration
 */

class COrganismInfoPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( COrganismInfoPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    COrganismInfoPanel();
    COrganismInfoPanel( wxWindow* parent, 
                        IWorkbench* wb,
                        ICommandProccessor* cmdproc,
                        objects::CSeq_entry_Handle seh,
                        const wxString &dir,
                        wxWindowID id = SYMBOL_CORGANISMINFOPANEL_IDNAME, 
                        const wxPoint& pos = SYMBOL_CORGANISMINFOPANEL_POSITION, 
                        const wxSize& size = SYMBOL_CORGANISMINFOPANEL_SIZE, 
                        long style = SYMBOL_CORGANISMINFOPANEL_STYLE );
    
    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CORGANISMINFOPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CORGANISMINFOPANEL_POSITION, 
        const wxSize& size = SYMBOL_CORGANISMINFOPANEL_SIZE, 
        long style = SYMBOL_CORGANISMINFOPANEL_STYLE );

    /// Destructor
    ~COrganismInfoPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();

////@begin COrganismInfoPanel event handler declarations
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);
    void OnImportSrcTable(wxCommandEvent& event);
////@end COrganismInfoPanel event handler declarations

////@begin COrganismInfoPanel member function declarations
    virtual bool StepForward();
    virtual bool StepBackward();

    virtual bool TransferDataFromWindow();
    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end COrganismInfoPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplyDescriptor(objects::CSeqdesc& desc);
    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh);

private:
    IWorkbench *m_Workbench;
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;
    wxString m_WorkDir;

////@begin COrganismInfoPanel member variables
    wxListbook* m_Listbook;
    COrgGeneralPanel* m_GeneralPanel;
    COrgAdvancedPanel* m_AdvancedPanel;
public:
    /// Control identifiers
    enum {
        ID_CORGANISMINFOPANEL = 6570,
        ID_ORGINFOBTN = 6571,
        ID_ORGINFONTBK = 6572
    };
////@end COrganismInfoPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___ORGINFO_PANEL__HPP
