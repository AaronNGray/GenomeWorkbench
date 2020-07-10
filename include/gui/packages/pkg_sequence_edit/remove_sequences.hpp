
/*  $Id: remove_sequences.hpp 43609 2019-08-08 16:12:53Z filippov $
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
 * Authors:  Igor Filippov
 */

#ifndef _REMOVE_SEQUENCES_H_
#define _REMOVE_SEQUENCES_H_

#include <corelib/ncbistd.hpp>

#include <gui/utils/command_processor.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

#define ID_REMOVE_SEQ_SELECT 12000
#define ID_REMOVE_SEQ_SELECT_ALL 12001
#define ID_REMOVE_SEQ_UNSELECT_ALL 12002
#define ID_REMOVE_SEQ_LEFT 12003
#define ID_REMOVE_SEQ_RIGHT 12004

class CRemoveSequencesDlg : public CReportEditingDialog
{
 DECLARE_DYNAMIC_CLASS( CRemoveSequencesDlg )
 DECLARE_EVENT_TABLE()

public:
    CRemoveSequencesDlg() {  Init(); }
    CRemoveSequencesDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = wxID_ANY, const wxString& caption = _("Remove Sequences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL);
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = _("Remove Sequences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL );       
    virtual ~CRemoveSequencesDlg() {}
    bool ShowToolTips()  { return true; }
    wxBitmap GetBitmapResource( const wxString& name ) {   wxUnusedVar(name);   return wxNullBitmap; }
    wxIcon GetIconResource( const wxString& name ) {  wxUnusedVar(name);   return wxNullIcon; }
    virtual void Init();
    void CreateControls();    
    void UpdateList();
    void OnButtonMoveLeft( wxCommandEvent& event );
    void OnButtonMoveRight( wxCommandEvent& event );
    void OnSelect( wxCommandEvent& event );
    void OnSelectAll( wxCommandEvent& event );
    void OnUnselectAll( wxCommandEvent& event );
    CSeq_entry_Handle GetSeqEntry(unsigned int i) {return m_Seqs[i].GetSeq_entry_Handle();}
    size_t GetSetSize() {return m_Seqs.size();}
    string GetLabel(CSeq_entry_Handle seh);
    CRef<CCmdComposite> GetCommand();

    void ChooseSequence(CBioseq_Handle bsh);

private:
    void CombineLabels(const CSeq_id &id, vector<string> &labels);
    void CollectLabels(CSeq_entry_Handle seh, list<string> &strs);
    bool MatchConstraint(CRef<objects::edit::CStringConstraint> constraint, CSeq_entry_Handle seh);
    void CombineLabelsAll(const CSeq_id &id, set<string> &labels);
    void CollectLabelsAll(CSeq_entry_Handle seh, set<string> &strs);

    wxTextCtrl *m_Min;
    wxTextCtrl *m_Max;
    vector<CBioseq_Handle> m_Seqs;
    wxListCtrl* m_ListCtrl1;
    wxListCtrl* m_ListCtrl2;
    set<int> m_Selected;
    vector<int> m_Available;  
    CStringConstraintPanel* m_StringConstraintPanel;
};

class CRemoveProteins
{
public:
    static CRef<CCmdComposite> JustRemoveProteins(objects::CSeq_entry_Handle seh);
    static CRef<CCmdComposite> AndRenormalizeNucProtSets(objects::CSeq_entry_Handle seh);
    static CRef<CCmdComposite> OrphanedProteins(objects::CSeq_entry_Handle seh);

};

END_NCBI_SCOPE

#endif
    // _REMOVE_SEQUENCES_H_
