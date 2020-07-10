  /*  $Id: withdraw_sequences.cpp 43728 2019-08-23 17:19:12Z filippov $
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

////@begin includes
////@end includes
#include <sstream>

#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/withdraw_sequences.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CWithdrawSequences, wxDialog )


/*!
 * CWithdrawSequences event table definition
 */

BEGIN_EVENT_TABLE( CWithdrawSequences, wxDialog )

END_EVENT_TABLE()


/*!
 * CWithdrawSequences constructors
 */

CWithdrawSequences::CWithdrawSequences()
{
    Init();
}

CWithdrawSequences::CWithdrawSequences( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    this->m_TopSeqEntry = seh;
    m_SetSeq.clear();
    ReadBioseq(*(m_TopSeqEntry.GetCompleteSeq_entry()));
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CWithdrawSequences::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWithdrawSequences creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWithdrawSequences creation

    SetSize(wxSize(280, 550));
    return true;
}


/*!
 * CWithdrawSequences destructor
 */

CWithdrawSequences::~CWithdrawSequences()
{
////@begin CWithdrawSequences destruction
////@end CWithdrawSequences destruction
}


/*!
 * Member initialisation
 */

void CWithdrawSequences::Init()
{
}


/*!
 * Control creation for CWithdrawSequences
 */

void CWithdrawSequences::CreateControls()
{    
    CWithdrawSequences* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
    
    m_IDPanel = new CIDSubpanel( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_IDPanel, 1, wxGROW|wxALL, 5);
      
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}



/*!
 * Should we show tooltips?
 */

bool CWithdrawSequences::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CWithdrawSequences::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CWithdrawSequences::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CWithdrawSequences::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Withdraw Sequences") );
    vector< vector<int> > & subsets = GetSubsets();
    if (this->m_TopSeqEntry && !subsets.empty() && !subsets.front().empty())
    {
        int i = subsets.front().front();
        CBioseq_set_Handle grandparent = this->m_SetSeq[i].grandparent;
        CBioseq_set_Handle parent = this->m_SetSeq[i].parent;

        CRef<objects::CBioseq_set> changed_set(new objects::CBioseq_set());
        if (grandparent != parent)
        {
            changed_set->Assign(*grandparent.GetCompleteBioseq_set());
        }
        else
        {
            CRef<CSeq_entry> new_se(new CSeq_entry());
            CRef<CBioseq_set> bss(new CBioseq_set());
            bss->Assign(*parent.GetCompleteBioseq_set());
            new_se->SetSet(*bss);
            changed_set->SetSeq_set().push_back(new_se);
            changed_set->SetClass(CBioseq_set::eClass_genbank);

            CRef<CSeqdesc> user_object( new CSeqdesc );
            CSeqdesc_Base::TUser& user = user_object->SetUser();
            user.SetType().SetStr("GbenchModification");  
            CRef<CUser_field> method = user.SetFieldRef("method");
            method->SetValue("WithdrawSequences");
            changed_set->SetDescr().Set().push_back( user_object );
        }

        for (unsigned int s = 0; s < subsets.size(); s++)
        {    
            PopulateSet(*changed_set, subsets[s]);
        }
        CCmdChangeBioseqSet *set_cmd = new CCmdChangeBioseqSet(grandparent, *changed_set);
        cmd->AddCommand(*set_cmd);
    }
    return cmd;
}

void CWithdrawSequences::PopulateSet(CBioseq_set& changed_set, const vector<int> &subset)
{
    if (changed_set.CanGetSeq_set())
    {
        for ( unsigned int j = 0; j < subset.size(); j++)
        {
            int i = subset[j];
            CRef<CSeq_entry> se(new CSeq_entry());
            se->Assign(*m_SetSeq[i].entry.GetCompleteSeq_entry());                                                                   
            TakeFromSet(changed_set,se);
            CopyDescrToEntry(*m_SetSeq[i].parent.GetCompleteBioseq_set(), *se);     
            changed_set.SetSeq_set().push_back(se);
        }
    }
}

void CWithdrawSequences::CopyDescrToEntry(const CBioseq_set& source, CSeq_entry& target)
{   
    if (source.IsSetDescr())
    {
        bool title_present = false;
        if (target.IsSetDescr())
        {
            for (CSeq_descr::Tdata::const_iterator desc = target.GetDescr().Get().begin(); desc != target.GetDescr().Get().end(); ++desc)
            {
                if ((*desc)->IsTitle() && !(*desc)->GetTitle().empty())
                {
		    title_present = true;
                    break;
                }
            }
        }
        for (CSeq_descr::Tdata::const_iterator desc = source.GetDescr().Get().begin(); desc != source.GetDescr().Get().end(); ++desc)
        {
            if (title_present && (*desc)->IsTitle())
                continue;
            target.SetDescr().Set().push_back(*desc);            
        }
    }
               
         
}

vector<string> CWithdrawSequences::GetNames()
{
    vector<string> names;
    vector< vector<int> > & subsets = GetSubsets();
    for (unsigned int s = 0; s < subsets.size(); s++)
    {
	for ( unsigned int j = 0; j < subsets[s].size(); j++)
	{
	    int i = subsets[s][j];
	    CConstRef<CSeq_entry> se = m_SetSeq[i].entry.GetCompleteSeq_entry();	   
	    if (se->IsSeq())
	    {
		FOR_EACH_SEQID_ON_BIOSEQ(id, se->GetSeq())
		{
		    string name;
		    (*id)->GetLabel(&name);
		    names.push_back(name);
		}
	    }
	    else if (se->IsSet() && se->GetSet().CanGetClass() && se->GetSet().GetClass() == CBioseq_set::eClass_nuc_prot )
	    {
		FOR_EACH_SEQID_ON_BIOSEQ(id, se->GetSet().GetNucFromNucProtSet())
		{
		    string name;
		    (*id)->GetLabel(&name);
		    names.push_back(name);
		}
	    }
	}
    }
    return names;
}

END_NCBI_SCOPE
