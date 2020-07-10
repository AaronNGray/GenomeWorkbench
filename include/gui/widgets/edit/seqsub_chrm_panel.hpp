#ifndef GUI_PKG_SEQ_EDIT___SEQSUB_CHRM_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SEQSUB_CHRM_PANEL__HPP

/*  $Id: seqsub_chrm_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <objmgr/seq_entry_handle.hpp>
#include <gui/utils/command_processor.hpp>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <wx/radiobut.h>

class wxScrolledWindow;
class wxFlexGridSizer;
class wxBoxSizer;
class wxStaticText;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEQSUBCHROMOPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSEQSUBCHROMOPANEL_TITLE _("Submission Chromosome Panel")
#define SYMBOL_CSEQSUBCHROMOPANEL_IDNAME ID_CSEQSUBCHROMOPANEL
#define SYMBOL_CSEQSUBCHROMOPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSEQSUBCHROMOPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqSubChromoPanel class declaration
 */

class CSeqSubChromoPanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CSeqSubChromoPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqSubChromoPanel();
    CSeqSubChromoPanel( wxWindow* parent, ICommandProccessor *cmdproc, objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CSEQSUBCHROMOPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQSUBCHROMOPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQSUBCHROMOPANEL_SIZE, 
        long style = SYMBOL_CSEQSUBCHROMOPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSEQSUBCHROMOPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQSUBCHROMOPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQSUBCHROMOPANEL_SIZE, 
        long style = SYMBOL_CSEQSUBCHROMOPANEL_STYLE );

    /// Destructor
    ~CSeqSubChromoPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();

////@begin CSeqSubChromoPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLCHROMOSOMEYES
    void OnChromosomeYesSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLCHROMOSOMENO
    void OnChromosomeNoSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDCHROMOSOME
    void OnAddchromosomeClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELCHROMOSOME
    void OnDeletechromosomeClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELONECHROMOSOME
    void OnDeleteOneChromosomeClicked(wxHyperlinkEvent& event);

////@end CSeqSubChromoPanel event handler declarations

////@begin CSeqSubChromoPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqSubChromoPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh);

    virtual void ApplyCommand();
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor() { return _("molecule-info-chromosome"); }

private:
    void x_AddFirstRow();
    void x_AddEmptyRow(bool hide_link = false);
    void x_AddRowToWindow(wxWindow* row, bool hide_link = false);
    void x_EnableControls(bool value);
    void x_DeleteAllSubWindows();

    ICommandProccessor *m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;

////@begin CSeqSubChromoPanel member variables
    wxRadioButton* m_YesChrom;
    wxRadioButton* m_NoChrom;

    int m_RowHeight{ 0 };
#ifndef __WXMSW__
    int m_TotalHeight{ 0 };
    int	m_TotalWidth{ 0 };
    int	m_ScrollRate{ 0 };
    int m_MaxRows{ 4 };
#endif
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    wxBoxSizer* m_LabelSizer;
    wxStaticText* m_ChromoLabel;
    wxHyperlinkCtrl* m_AddChromosome;
    wxHyperlinkCtrl* m_DelChromosome;
public:
    /// Control identifiers
    enum {
        ID_CSEQSUBCHROMOPANEL = 6620,
        ID_MOLCHROMOSOMEYES = 6621,
        ID_MOLCHROMOSOMENO = 6622,
        ID_MOLCHROMOSCROLLEDWND = 6623,
        ID_MOLADDCHROMOSOME = 6624,
        ID_MOLDELCHROMOSOME = 6625,
        ID_MOLDELONECHROMOSOME = 6626
    };
////@end CSeqSubChromoPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SEQSUB_CHRM_PANEL__HPP
