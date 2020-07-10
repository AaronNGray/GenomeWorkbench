#ifndef _GUI_PACKAGES__MATCH_FEAT_TABLE_DLG_H_
#define _GUI_PACKAGES__MATCH_FEAT_TABLE_DLG_H_
/*  $Id: match_feat_table_dlg.hpp 40491 2018-02-27 18:15:04Z filippov $
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
 *
 *   Dialog to match feature table IDs with sequence IDs from the TSE
 */

#include <corelib/ncbistd.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <wx/dialog.h>
#include <wx/choice.h>

class wxButton;
class wxListCtrl;
class wxListView;
class wxCheckListBox;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMATCHFEATURETABLEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMATCHFEATURETABLEDLG_TITLE _("Match Feature Table IDs")
#define SYMBOL_CMATCHFEATURETABLEDLG_IDNAME ID_CMATCHFEATURETABLEDLG
#define SYMBOL_CMATCHFEATURETABLEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CMATCHFEATURETABLEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMatchFeatureTableDlg class declaration
 */

class CMatchFeatureTableDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CMatchFeatureTableDlg )
    DECLARE_EVENT_TABLE()

public:
    typedef set<objects::CSeq_id_Handle> TUniqueFeatTblIDs;
    typedef map<objects::CSeq_id_Handle, objects::CSeq_id_Handle> TFeatSeqIDMap;
    

    /// Constructors
    CMatchFeatureTableDlg();
    CMatchFeatureTableDlg( wxWindow* parent, const objects::CSeq_entry_Handle& seh,
        const TUniqueFeatTblIDs& ftblids,
        wxWindowID id = SYMBOL_CMATCHFEATURETABLEDLG_IDNAME, 
        const wxString& caption = SYMBOL_CMATCHFEATURETABLEDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CMATCHFEATURETABLEDLG_POSITION, 
        const wxSize& size = SYMBOL_CMATCHFEATURETABLEDLG_SIZE, 
        long style = SYMBOL_CMATCHFEATURETABLEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMATCHFEATURETABLEDLG_IDNAME, 
        const wxString& caption = SYMBOL_CMATCHFEATURETABLEDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CMATCHFEATURETABLEDLG_POSITION, 
        const wxSize& size = SYMBOL_CMATCHFEATURETABLEDLG_SIZE, 
        long style = SYMBOL_CMATCHFEATURETABLEDLG_STYLE );

    /// Destructor
    ~CMatchFeatureTableDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMatchFeatureTableDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FEATTBLID_BTN
    void OnMapSelectedBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_AUTOMATCH_TABLE_ID
    void OnAutomatchTableIdClick( wxCommandEvent& event );

////@end CMatchFeatureTableDlg event handler declarations

////@begin CMatchFeatureTableDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMatchFeatureTableDlg member function declarations

    bool TransferDataToWindow();
    void GetFtableIDToSeqIDMap(TFeatSeqIDMap& ftbl_seqid_map);

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMatchFeatureTableDlg member variables
    wxListView* m_FeatTableIDList;
    wxListView* m_SequenceIDList;
    wxButton* m_Map;
    wxChoice* m_AutoMatchType;
    wxCheckListBox* m_MappedIDs;
    /// Control identifiers
    enum {
        ID_CMATCHFEATURETABLEDLG = 10447,
        ID_FEATTBLID_LSTCTRLA = 10448,
        ID_FEATTBLID_LSTCTRLB = 10449,
        ID_FEATTBLID_BTN = 10450,
        ID_AUTOMATCH_TABLE_ID = 10109,
        ID_AUTO_MATCH_TYPE = 10123,
        ID_FEATTBLID_CHKLSTBX = 10451
    };
////@end CMatchFeatureTableDlg member variables
private:
    objects::CSeq_entry_Handle m_Seh;
    const set<objects::CSeq_id_Handle> m_FtblIDs;

    typedef map<string, objects::CSeq_id_Handle> TStrToIDMap;
    TStrToIDMap m_FtblID_map;
    TStrToIDMap m_SeqID_map;

    bool x_TokenMatch(const string& ft, const string& rec);
    void x_AddMatch(long item_ftbl, long item_seqid);
    void LinearAutomatch(wxArrayString &matched_ids);
    void QuadraticAutomatch(wxArrayString &matched_ids);
};

END_NCBI_SCOPE

#endif
    // _GUI_PACKAGES__MATCH_FEAT_TABLE_DLG_H_
