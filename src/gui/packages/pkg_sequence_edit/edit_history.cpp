/*  $Id: edit_history.cpp 42186 2019-01-09 19:34:50Z asztalos $
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
#include <objects/seqblock/GB_block.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seq/Seq_hist_rec.hpp>
#include <objects/general/Date.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>   
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>

#include <set>

#include <objmgr/scope.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include "growable_list.hpp"
#include <gui/packages/pkg_sequence_edit/edit_history.hpp>



////@begin includes
////@end includes


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CEditHistory type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditHistory, wxDialog )


/*!
 * CEditHistory event table definition
 */

BEGIN_EVENT_TABLE( CEditHistory, wxDialog )

////@begin CEditHistory event table entries
////@end CEditHistory event table entries

END_EVENT_TABLE()


/*!
 * CEditHistory constructors
 */

CEditHistory::CEditHistory()
{
    Init();
}


CEditHistory::CEditHistory( wxWindow* parent, CBioseq_Handle bsh,
                            wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Bioseq(bsh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CEditHistory creator
 */

bool CEditHistory::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditHistory creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    
////@end CEditHistory creation
    return true;
}


/*!
 * CEditHistory destructor
 */

CEditHistory::~CEditHistory()
{
////@begin CEditHistory destruction
////@end CEditHistory destruction
}


/*!
 * Member initialisation
 */

void CEditHistory::Init()
{
////@begin CEditHistory member initialisation

////@end CEditHistory member initialisation
}


/*!
 * Control creation for CEditHistory
 */

void CEditHistory::CreateControls()
{    
////@begin CEditHistory content construction
    CEditHistory* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxArrayString seq_ids_replaces;
    wxArrayString seq_ids_replaced_by;
    CRef<CDate> replaces_date(new CDate);
    CRef<CDate> replaced_by_date(new CDate);
    bool is_deleted = false;
    CRef<CDate> deleted_date(new CDate);
    if (m_Bioseq && m_Bioseq.IsSetInst_Hist()) 
    { 
        CScope &scope = m_Bioseq.GetScope();
        const CSeq_hist& hist = m_Bioseq.GetInst_Hist();
        if (hist.IsSetReplaces())
        {
            if (hist.GetReplaces().IsSetIds())
            {
                for( CSeq_hist_rec::TIds::const_iterator id_it = hist.GetReplaces().GetIds().begin(); id_it != hist.GetReplaces().GetIds().end(); ++id_it)
                {
                    string str;
                    CLabel::GetLabel(**id_it, &str, CLabel::eContent, &scope);
                    seq_ids_replaces.Add(ToWxString (str));
                }
            }
            if (hist.GetReplaces().IsSetDate())
            {
                replaces_date->Assign(hist.GetReplaces().GetDate());
            }
        }      
        if (hist.IsSetReplaced_by())
        {
            if (hist.GetReplaced_by().IsSetIds())
            {
                for( CSeq_hist_rec::TIds::const_iterator id_it = hist.GetReplaced_by().GetIds().begin(); id_it != hist.GetReplaced_by().GetIds().end(); ++id_it)
                {
                    string str;
                    CLabel::GetLabel(**id_it, &str, CLabel::eContent, &scope);
                    seq_ids_replaced_by.Add(ToWxString (str));
                }
            }
            if (hist.GetReplaced_by().IsSetDate())
            {
                replaced_by_date->Assign(hist.GetReplaced_by().GetDate());
            }
        }    
        if (hist.IsSetDeleted())
        {
            if (hist.GetDeleted().IsBool())
                is_deleted = hist.GetDeleted().GetBool();
            if (hist.GetDeleted().IsDate())
                deleted_date->Assign(hist.GetDeleted().GetDate());
        }
    } 
    if (seq_ids_replaces.IsEmpty())        
        seq_ids_replaces.Add(wxEmptyString);
    if (seq_ids_replaced_by.IsEmpty())        
        seq_ids_replaced_by.Add(wxEmptyString);

// TODO secondary on parts
    wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Replaces Seq-id"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IdListReplaces = new CGrowableListCtrl( itemDialog1,seq_ids_replaces, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_IdListReplaces, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_ReplacesDate = new CFlexibleDatePanel(itemDialog1, replaces_date);
    itemBoxSizer2->Add(m_ReplacesDate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Replaced By Seq-id"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IdListReplacedBy = new CGrowableListCtrl( itemDialog1,seq_ids_replaced_by, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_IdListReplacedBy, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_ReplacedByDate = new CFlexibleDatePanel(itemDialog1, replaced_by_date);
    itemBoxSizer2->Add(m_ReplacedByDate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Status"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Deleted = new wxCheckBox( itemDialog1, wxID_ANY, _("Deleted"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Deleted, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_Deleted->SetValue(is_deleted);

    m_DeletedDate = new CFlexibleDatePanel(itemDialog1, deleted_date);
    itemBoxSizer2->Add(m_DeletedDate, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CEditHistory content construction
    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}


CRef<CCmdComposite> CEditHistory::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Edit History"));   

    CRef<CSeq_inst> new_inst(new CSeq_inst);
    new_inst->Assign(m_Bioseq.GetBioseqCore()->GetInst());    
    new_inst->ResetHist();

    vector<string> replaces_ids;
    m_IdListReplaces->GetItems(replaces_ids);
    vector<string> replaced_ids;
    m_IdListReplacedBy->GetItems(replaced_ids);

    for (size_t i = 0; i < replaces_ids.size(); i++)
    {
        CRef<CSeq_id> id(new CSeq_id(replaces_ids[i]));
        new_inst->SetHist().SetReplaces().SetIds().push_back(id);
    }

    for (size_t i = 0; i < replaced_ids.size(); i++)
    {
        CRef<CSeq_id> id(new CSeq_id(replaced_ids[i]));
        new_inst->SetHist().SetReplaced_by().SetIds().push_back(id);
    }

    if (m_ReplacesDate->GetDate())
        new_inst->SetHist().SetReplaces().SetDate(*m_ReplacesDate->GetDate());
    if (m_ReplacedByDate->GetDate())
        new_inst->SetHist().SetReplaced_by().SetDate(*m_ReplacedByDate->GetDate());
   
    if (m_DeletedDate->GetDate())
        new_inst->SetHist().SetDeleted().SetDate(*m_DeletedDate->GetDate());
    if (m_Deleted->GetValue())
        new_inst->SetHist().SetDeleted().SetBool(true);

    cmd->AddCommand(*CRef<CCmdChangeBioseqInst>(new CCmdChangeBioseqInst(m_Bioseq, *new_inst)));

    CSeq_entry_Handle seh = m_Bioseq.GetSeq_entry_Handle();
    CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
    CSeqdesc_CI desc_iter(seh, CSeqdesc::e_Genbank,1);
    if (desc_iter) 
    {
        new_desc->Assign(*desc_iter);
        set<string> existing;
        for (const auto &str : new_desc->GetGenbank().GetExtra_accessions())
        {
            existing.insert(str);
        }
        for (size_t i = 0; i < replaces_ids.size(); i++)
        {
            string str = replaces_ids[i];
            if (existing.find(str) == existing.end())
            {
                new_desc->SetGenbank().SetExtra_accessions().push_back(str);
                existing.insert(str);
            }
        }
        CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_iter.GetSeq_entry_Handle(),*desc_iter, *new_desc));
        cmd->AddCommand (*ecmd);
    }
    else 
    {
        set<string> existing;
        for (size_t i = 0; i < replaces_ids.size(); i++)
        {
            string str = replaces_ids[i];
            if (existing.find(str) == existing.end())
            {
                new_desc->SetGenbank().SetExtra_accessions().push_back(str);
                existing.insert(str);
            }
        }
        cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(seh, *new_desc)) );
    }

    return cmd;
}


/*!
 * Should we show tooltips?
 */

bool CEditHistory::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditHistory::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditHistory bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditHistory bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditHistory::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditHistory icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditHistory icon retrieval
}

END_NCBI_SCOPE


