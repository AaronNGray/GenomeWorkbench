#ifndef GUI_PKG_SEQ_EDIT___SEQSUB_ORGANELLE_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SEQSUB_ORGANELLE_PANEL__HPP

/*  $Id: seqsub_organelle_panel.hpp 43209 2019-05-29 14:01:26Z bollin $
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
#include <objects/seqfeat/BioSource.hpp>
#include <gui/utils/command_processor.hpp>
#include <wx/panel.h>
#include <wx/hyperlink.h>
#include <wx/radiobut.h>

class wxScrolledWindow;
class wxFlexGridSizer;
class wxBoxSizer;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSEQSUBORGANELLEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSEQSUBORGANELLEPANEL_TITLE _("Submission Organelle Panel")
#define SYMBOL_CSEQSUBORGANELLEPANEL_IDNAME ID_CSEQSUBORGANELLEPANEL
#define SYMBOL_CSEQSUBORGANELLEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSEQSUBORGANELLEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqSubOrganellePanel class declaration
 */

class CSeqSubOrganellePanel: public wxPanel, public ISubmissionPage
{    
    DECLARE_DYNAMIC_CLASS( CSeqSubOrganellePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqSubOrganellePanel();
    CSeqSubOrganellePanel( wxWindow* parent, ICommandProccessor *cmdproc, objects::CSeq_entry_Handle seh,
        wxWindowID id = SYMBOL_CSEQSUBORGANELLEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQSUBORGANELLEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQSUBORGANELLEPANEL_SIZE, 
        long style = SYMBOL_CSEQSUBORGANELLEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSEQSUBORGANELLEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSEQSUBORGANELLEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSEQSUBORGANELLEPANEL_SIZE, 
        long style = SYMBOL_CSEQSUBORGANELLEPANEL_STYLE );

    /// Destructor
    ~CSeqSubOrganellePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();

////@begin CSeqSubOrganellePanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLORGANELLEYES
    void OnOrganelleYesSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLORGANELLENO
    void OnOrganelleNoSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDORGANELLE
    void OnAddOrganelleClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELORGANELLE
    void OnDeleteOrganelleClicked( wxHyperlinkEvent& event );

    // wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELONEORGANELLE
    void OnDeleteOneOrganelleClicked(wxHyperlinkEvent& event);

////@end CSeqSubOrganellePanel event handler declarations

////@begin CSeqSubOrganellePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqSubOrganellePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void ApplyCommand();
    virtual void SetSeqEntryHandle(objects::CSeq_entry_Handle seh) { m_Seh = seh; x_DeleteAllSubWindows(); TransferDataToWindow();}
    virtual void ReportMissingFields(string &text);
    virtual wxString GetAnchor() { return _("molecule-info-organelle"); }

private:

    void x_AddEmptyRow();
    void x_AddRowToWindow(wxWindow* row);
    void x_EnableControls(bool value);
    void x_GetOrganelleTypes();
    void x_DeleteAllSubWindows();

    size_t m_Rows{ 1 };
    int m_RowHeight{ 0 };
#ifndef __WXMSW__
    int m_TotalHeight{ 0 };
    int	m_TotalWidth{ 0 };
    int	m_ScrollRate{ 0 };
    int m_MaxRows{ 4 };
#endif
    ICommandProccessor *m_CmdProcessor;
    objects::CSeq_entry_Handle m_Seh;
    vector<objects::CBioSource::EGenome> m_organelle_types;

    ////@begin CSeqSubOrganellePanel member variables
    wxRadioButton* m_YesOrganelle;
    wxRadioButton* m_NoOrganelle;
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    wxBoxSizer* m_LabelSizer;
    wxHyperlinkCtrl* m_AddOrganelle;
    wxHyperlinkCtrl* m_DelOrganelle;
public:
    /// Control identifiers
    enum {
        ID_CSEQSUBORGANELLEPANEL = 6610,
        ID_MOLORGANELLEYES = 6611,
        ID_MOLORGANELLENO = 6612,
        ID_MOLORGANELLESCROLLEDWND = 6613,
        ID_MOLADDORGANELLE = 6614,
        ID_MOLDELORGANELLE = 6615,
        ID_MOLDELONEORGANELLE = 6616
    };
////@end CSeqSubOrganellePanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SEQSUB_ORGANELLE_PANEL__HPP
