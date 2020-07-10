#ifndef GUI_PKG_SEQ_EDIT___SEQSUB_MOLINFO_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SEQSUB_MOLINFO_PANEL__HPP

/*  $Id: seqsub_molinfo_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/submission_page_interface.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <wx/panel.h>
#include <wx/bookctrl.h>

class wxListbook;

BEGIN_NCBI_SCOPE

class CSeqSubPlasmidPanel;
class CSeqSubChromoPanel;
class CSeqSubOrganellePanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSUBMOLINFOPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSUBMOLINFOPANEL_TITLE _("Submission Molinfo Panel")
#define SYMBOL_CSUBMOLINFOPANEL_IDNAME ID_CSUBMOLINFOPANEL
#define SYMBOL_CSUBMOLINFOPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSUBMOLINFOPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSubMolinfoPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSubMolinfoPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CSubMolinfoPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSubMolinfoPanel();
    CSubMolinfoPanel( wxWindow* parent, 
        ICommandProccessor* cmdproc,
        objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CSUBMOLINFOPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBMOLINFOPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBMOLINFOPANEL_SIZE, 
        long style = SYMBOL_CSUBMOLINFOPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSUBMOLINFOPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSUBMOLINFOPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSUBMOLINFOPANEL_SIZE, 
        long style = SYMBOL_CSUBMOLINFOPANEL_STYLE );

    /// Destructor
    ~CSubMolinfoPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();

////@begin CSubMolinfoPanel event handler declarations
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnPageChanging(wxBookCtrlEvent& event);
////@end CSubMolinfoPanel event handler declarations

////@begin CSubMolinfoPanel member function declarations
    virtual bool StepForward();
    virtual bool StepBackward();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSubMolinfoPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor();

    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh);

private:
    ICommandProccessor* m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;

////@begin CSubMolinfoPanel member variables
    wxListbook* m_Listbook;
    CSeqSubPlasmidPanel* m_PlasmidPanel;
    CSeqSubChromoPanel* m_ChromosomePanel;
    CSeqSubOrganellePanel* m_OrganellePanel;
public:
    /// Control identifiers
    enum {
        ID_CSUBMOLINFOPANEL = 6630,
        ID_MOLINFONOTEBOOK = 6631
    };
////@end CSubMolinfoPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SEQSUB_MOLINFO_PANEL__HPP
