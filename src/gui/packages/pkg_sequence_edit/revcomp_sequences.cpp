/*  $Id: revcomp_sequences.cpp 41600 2018-08-29 15:45:58Z filippov $
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


#include <ncbi_pch.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/revcomp_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>

#include <objects/seq/MolInfo.hpp>

#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/align_ci.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


IMPLEMENT_DYNAMIC_CLASS( CRevCompSequencesDlg, wxDialog )


/*!
 * CRevCompSequencesDlg event table definition
 */

BEGIN_EVENT_TABLE( CRevCompSequencesDlg, wxDialog )

EVT_BUTTON( ID_SELECTALL, CRevCompSequencesDlg::OnSelectAll)
EVT_BUTTON( ID_UNSELECTALL, CRevCompSequencesDlg::OnUnselectAll)
EVT_BUTTON( ID_SELECT, CRevCompSequencesDlg::OnSelect)

END_EVENT_TABLE()


/*!
 * CRevCompSequencesDlg constructors
 */

CRevCompSequencesDlg::CRevCompSequencesDlg()
{
    Init();
}

CRevCompSequencesDlg::CRevCompSequencesDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CRevCompSequencesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRevCompSequencesDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CRevCompSequencesDlg creation

    SetSize(wxSize(280, 750));
    return true;
}


/*!
 * CRevCompSequencesDlg destructor
 */

CRevCompSequencesDlg::~CRevCompSequencesDlg()
{
////@begin CRevCompSequencesDlg destruction
////@end CRevCompSequencesDlg destruction
}


/*!
 * Member initialisation
 */

void CRevCompSequencesDlg::Init()
{
    m_ListCtrl = NULL;
}


/*!
 * Control creation for CRevCompSequencesDlg
 */

void CRevCompSequencesDlg::CreateControls()
{    
    CRevCompSequencesDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
    itemBoxSizer3->Add(m_ListCtrl, 1, wxGROW|wxALL, 5);

    m_ListCtrl->InsertColumn(0,wxEmptyString,wxLIST_FORMAT_LEFT);
    if (m_TopSeqEntry)
    {
        int i = 0;
        objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
        for ( ; b_iter ; ++b_iter ) 
        {
            string id_str;
            CSeq_id_Handle best = sequence::GetId(*b_iter, sequence::eGetId_Best);
            best.GetSeqId()->GetLabel(&id_str, CSeq_id::eContent);
            if (!id_str.empty())
            {
                m_ListCtrl->InsertItem(i,ToWxString(id_str));
                m_ListCtrl->SetItemData(i,i);
                m_Entries.push_back(*b_iter);
                i++;
            }
        }            
    }
    m_ListCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE);

    m_CheckSequence = new wxCheckBox( itemDialog1, wxID_ANY, _("Reverse complement sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckSequence->SetValue(true);
    itemBoxSizer2->Add(m_CheckSequence, 0, wxALIGN_LEFT|wxALL, 5);

    m_CheckFeat = new wxCheckBox( itemDialog1, wxID_ANY, _("Reverse features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckFeat->SetValue(true);
    itemBoxSizer2->Add(m_CheckFeat, 0, wxALIGN_LEFT|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);        

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemDialog1, ID_SELECT, _("Select"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton2 = new wxButton( itemDialog1, ID_SELECTALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton3 = new wxButton( itemDialog1, ID_UNSELECTALL, _("Unselect All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/*!
 * Should we show tooltips?
 */

bool CRevCompSequencesDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRevCompSequencesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CRevCompSequencesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CRevCompSequencesDlg::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("RevComp Sequences") );
    
    CRef<CCmdComposite> cmd_null;
    if (!m_ListCtrl) return cmd_null;
    
    bool modified = false;
    bool found_alignments = false;
    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )  break;
        int i = static_cast<int>(m_ListCtrl->GetItemData(item)); 

        CBioseq_Handle bsh = m_Entries[i];
        found_alignments |= RevCompBioSeq(bsh,cmd,m_TopSeqEntry,m_CheckSequence->GetValue(), m_CheckFeat->GetValue(), true);
        modified = true;
    }  
    if (!modified) return cmd_null;
    
    if (found_alignments)
        wxMessageBox(_("Alingments will be broken after this operation"), wxT("Warning"), wxOK | wxICON_ERROR, NULL);
    return cmd;
}


void CRevCompSequencesDlg::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, bool update_seq, bool update_feat, bool update_graph)
{
    CRef<CCmdComposite> cmd( new CCmdComposite(title) );    
    bool found_alignments = false;
    if (tse)
    {
        objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
        for ( ; b_iter ; ++b_iter ) 
        {
            CBioseq_Handle bsh = *b_iter;
            found_alignments |= RevCompBioSeq(bsh, cmd, tse, update_seq, update_feat, update_graph);
        }
    }  
    
    if (found_alignments)
        wxMessageBox(_("Alingments will be broken after this operation"), wxT("Warning"), wxOK | wxICON_ERROR, NULL);

    cmdProcessor->Execute(cmd.GetPointer());
}

bool CRevCompSequencesDlg::RevCompBioSeq(CBioseq_Handle bsh, CRef<CCmdComposite> cmd, CSeq_entry_Handle tse, bool update_seq, bool update_feat, bool update_graph)
{
    bool found_alignments = false;
    CScope& scope = bsh.GetScope();
    if ( update_seq )
    {
        CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
        new_inst->Assign(bsh.GetInst());
        ReverseComplement(*new_inst, &scope);
        CRef<CCmdChangeBioseqInst> cmd_inst(new CCmdChangeBioseqInst(bsh, *new_inst));
        cmd->AddCommand(*cmd_inst);
    }
    
    if (  update_feat )
    {
        CFeat_CI feat_ci(bsh, CSeq_annot::C_Data::e_Ftable);
        for (; feat_ci; ++feat_ci)
        {       
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(feat_ci->GetOriginalFeature());
            edit::ReverseComplementFeature(*new_feat,scope);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(*feat_ci, *new_feat)));
        }
    }

    if (update_graph)
    {
        CGraph_CI graph_ci(bsh, CSeq_annot::C_Data::e_Graph);
        for (; graph_ci; ++graph_ci)
        {
            const CMappedGraph& graph = *graph_ci;
            CRef<CSeq_graph> new_graph(new CSeq_graph());
            new_graph->Assign(graph.GetOriginalGraph());
            edit::ReverseComplementLocation(new_graph->SetLoc(),scope);
            CSeq_graph::TGraph& data = new_graph->SetGraph();
            switch ( data.Which() ) 
            {
            case CSeq_graph::TGraph::e_Byte:  reverse( data.SetByte().SetValues().begin(), data.SetByte().SetValues().end() );   break;
            case CSeq_graph::TGraph::e_Int:  reverse( data.SetInt().SetValues().begin(), data.SetInt().SetValues().end() );   break;
            case CSeq_graph::TGraph::e_Real:  reverse( data.SetReal().SetValues().begin(), data.SetReal().SetValues().end() );   break;
            default: break;
            }
            CIRef<IEditCommand> chgGraph( new CCmdChangeGraph( graph.GetSeq_graph_Handle(), *new_graph ));
            cmd->AddCommand(*chgGraph);
        }
    }
    
    CAlign_CI align_ci(tse, CSeq_annot::C_Data::e_Align);
    for (; align_ci; ++align_ci)
    {
        const CSeq_align& align = *align_ci;
        int num_rows = align.CheckNumRows();
        for (int row = 0; row < num_rows; row++)
        {
            const CSeq_id&  align_id = align.GetSeq_id(row);
            if (bsh.IsSynonym(align_id))
            {
                found_alignments = true;
                break;
            }
        }
        if (found_alignments)
        {
            break;
        }
    }
    
    return found_alignments;
}

void CRevCompSequencesDlg::OnSelect( wxCommandEvent& event )
{
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    if (!constraint) return;

    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
        if ( item == -1 )  break;
        int i = static_cast<int>(m_ListCtrl->GetItemData(item)); 
        CBioseq_Handle bsh = m_Entries[i];
        string id_str;
        CSeq_id_Handle best = sequence::GetId(bsh, sequence::eGetId_Best);
        best.GetSeqId()->GetLabel(&id_str, CSeq_id::eContent);
        string accession;
        int version;
        best.GetSeqId()->GetLabel(&accession, &version, CSeq_id::eContent);
        if ((!id_str.empty() && constraint->DoesTextMatch(id_str)) ||
            (!accession.empty() && constraint->DoesTextMatch(accession)))
        {
            m_ListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    }

}
 
void CRevCompSequencesDlg::OnSelectAll( wxCommandEvent& event )
{
    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
        if ( item == -1 )  break;
        m_ListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void CRevCompSequencesDlg::OnUnselectAll( wxCommandEvent& event )
{
    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )  break;
        m_ListCtrl->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

END_NCBI_SCOPE
