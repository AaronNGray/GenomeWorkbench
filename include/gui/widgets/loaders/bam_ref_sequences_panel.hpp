#ifndef GUI_WIDGETS___LOADERS___BAM_REF_SEQUENCES_PANEL__HPP
#define GUI_WIDGETS___LOADERS___BAM_REF_SEQUENCES_PANEL__HPP

/*  $Id: bam_ref_sequences_panel.hpp 44710 2020-02-27 18:03:07Z shkeda $
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
 * Authors:  
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <wx/panel.h>
#include <wx/checklst.h>

#include <gui/gui_export.h>

#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/scope.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/srchctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSearchCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CBAMREFSEQUENCESPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBAMREFSEQUENCESPANEL_TITLE _("BAM Referenced Sequences")
#define SYMBOL_CBAMREFSEQUENCESPANEL_IDNAME ID_CBAMREFSEQUENCESPANEL
#define SYMBOL_CBAMREFSEQUENCESPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBAMREFSEQUENCESPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CBamRefSequencesPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CBamRefSequencesPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBamRefSequencesPanel )
    DECLARE_EVENT_TABLE()

public:

    /// Constructors
    CBamRefSequencesPanel();
    CBamRefSequencesPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBAMREFSEQUENCESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBAMREFSEQUENCESPANEL_POSITION, const wxSize& size = SYMBOL_CBAMREFSEQUENCESPANEL_SIZE, long style = SYMBOL_CBAMREFSEQUENCESPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBAMREFSEQUENCESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBAMREFSEQUENCESPANEL_POSITION, const wxSize& size = SYMBOL_CBAMREFSEQUENCESPANEL_SIZE, long style = SYMBOL_CBAMREFSEQUENCESPANEL_STYLE );

    /// Destructor
    ~CBamRefSequencesPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBamRefSequencesPanel event handler declarations

    /// wxEVT_IDLE event handler for ID_CBAMREFSEQUENCESPANEL
    void OnIdle( wxIdleEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_ALL_BTN
    void OnSelectAllBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DESELECT_ALL_BTN
    void OnDeselectAllBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN event handler for ID_SEARCHCTRL
    void OnSearchButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN event handler for ID_SEARCHCTRL
    void OnSearchCancelClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_SEARCHCTRL
    void OnSearchEnter( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SEARCHCTRL
    void OnSearchCtrlUpdate( wxUpdateUIEvent& event );

////@end CBamRefSequencesPanel event handler declarations

////@begin CBamRefSequencesPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBamRefSequencesPanel member function declarations

    void SetReferenceSeqIds(const vector<CConstRef<objects::CSeq_id> >& refSeqIds);
    void GetSelectedReferenceSeqIds(vector<CConstRef<objects::CSeq_id> >& selSeqIds) const;

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBamRefSequencesPanel member variables
    wxCheckListBox* m_RefSeqList;
    wxSearchCtrl* m_SearchCtrl;
    /// Control identifiers
    enum {
        ID_CBAMREFSEQUENCESPANEL = 10000,
        ID_CHECKLISTBOX = 10001,
        ID_SELECT_ALL_BTN = 10002,
        ID_DESELECT_ALL_BTN = 10003,
        ID_SEARCHCTRL = 10024
    };
////@end CBamRefSequencesPanel member variables

private:
    vector<CConstRef<objects::CSeq_id> > m_RefSeqIds;
    list<size_t> m_IdsWithNoDescriptions;

    CRef<objects::CScope> m_Scope;

    void x_FillList(const string& filter);

    string m_CurrentFilter;

};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___BAM_REF_SEQUENCES_PANEL__HPP
