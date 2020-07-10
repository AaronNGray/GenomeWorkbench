/*  $Id: restore_rna_edit.cpp 41749 2018-09-19 20:25:42Z asztalos $
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


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <objtools/cleanup/cleanup.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/packages/pkg_sequence_edit/restore_rna_edit.hpp>

////@begin includes
////@end includes


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CRestoreRNAediting type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRestoreRNAediting, wxDialog )


/*
 * CRestoreRNAediting event table definition
 */

BEGIN_EVENT_TABLE( CRestoreRNAediting, wxDialog )

////@begin CRestoreRNAediting event table entries
EVT_BUTTON( ID_RESTORE_RNA_EDIT_APPLY, CRestoreRNAediting::OnApply )
////@end CRestoreRNAediting event table entries

END_EVENT_TABLE()


/*
 * CRestoreRNAediting constructors
 */

CRestoreRNAediting::CRestoreRNAediting()
{
    Init();
}

CRestoreRNAediting::CRestoreRNAediting( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor,
					wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_CmdProcessor(cmdProcessor)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CRestoreRNAediting creator
 */

bool CRestoreRNAediting::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRestoreRNAediting creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRestoreRNAediting creation
    return true;
}


/*
 * CRestoreRNAediting destructor
 */

CRestoreRNAediting::~CRestoreRNAediting()
{
////@begin CRestoreRNAediting destruction
////@end CRestoreRNAediting destruction
}


/*
 * Member initialisation
 */

void CRestoreRNAediting::Init()
{
////@begin CRestoreRNAediting member initialisation
    m_Name = NULL;
    m_LeaveUp = NULL;
////@end CRestoreRNAediting member initialisation
}


/*
 * Control creation for CRestoreRNAediting
 */

void CRestoreRNAediting::CreateControls()
{    
////@begin CRestoreRNAediting content construction
    CRestoreRNAediting* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Protein name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Name = new wxTextCtrl( itemDialog1, ID_RESTORE_PROTEINNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Name, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_RESTORE_RNA_EDIT_APPLY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LeaveUp = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Leave dialog up"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveUp->SetValue(false);
    itemBoxSizer6->Add(m_LeaveUp, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRestoreRNAediting content construction
}


/*
 * Should we show tooltips?
 */

bool CRestoreRNAediting::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CRestoreRNAediting::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRestoreRNAediting bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRestoreRNAediting bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CRestoreRNAediting::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRestoreRNAediting icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRestoreRNAediting icon retrieval
}

void CRestoreRNAediting::OnApply(wxCommandEvent& event)
{
    string name = m_Name->GetValue().ToStdString();
    NStr::TruncateSpacesInPlace(name);
    CRef<CCmdComposite> cmd(new CCmdComposite("Restore RNA Editing"));
    for (CFeat_CI cds(m_TopSeqEntry, CSeqFeatData::eSubtype_cdregion); cds; ++cds)
    {
	if (!cds->IsSetProduct()) 
	    continue;

	const CSeq_id *id = cds->GetProduct().GetId();
        if (!id) 
	    continue;

	CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*id);
	if (!bsh) 
	    continue;

	string prot_seq;
	CSeqVector prot_vec(*(bsh.GetCompleteBioseq()), &m_TopSeqEntry.GetScope());
	prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
	prot_vec.GetSeqData(0, prot_vec.size(), prot_seq);

	if (prot_seq.empty() || prot_seq.front() == 'M')
	    continue;

	CFeat_CI prot_feat_ci(bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
	if( !prot_feat_ci)
	    continue;

	CSeq_feat_Handle prot_fh = prot_feat_ci->GetSeq_feat_Handle();
	CConstRef<CSeq_feat> feat = prot_fh.GetSeq_feat();
	if (!feat->IsSetData() || !feat->GetData().IsProt())
	    continue;

	bool found = false;
	FOR_EACH_NAME_ON_PROTREF(name_it, feat->GetData().GetProt())
	{
	    if (name == *name_it)
	    {
		found = true;
		break;
	    }	    
	}
	if (!found)
	    continue;

	CRef<CSeq_feat> new_cds(new CSeq_feat);
	new_cds->Assign(*cds->GetOriginalSeq_feat());
	if (CCleanup::FixRNAEditingCodingRegion(*new_cds))
        {
            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(cds->GetSeq_feat_Handle(), *new_cds));
            cmd->AddCommand(*chgFeat);
        }
	CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
	new_inst->Assign(bsh.GetInst());
	if (new_inst->IsSetSeq_data())
	{
	    new_inst->ResetSeq_data();
	}
	prot_seq[0] = 'M';
	new_inst->ResetExt();
	new_inst->SetRepr(objects::CSeq_inst::eRepr_raw); 
	new_inst->SetSeq_data().SetNcbieaa().Set(prot_seq);
	new_inst->SetLength(TSeqPos(prot_seq.length()));
	new_inst->SetMol(CSeq_inst::eMol_aa);
        CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(bsh, *new_inst));
        cmd->AddCommand(*chgInst);

    }  
    m_CmdProcessor->Execute(cmd);
    if (!m_LeaveUp->GetValue())
	Close();
}
END_NCBI_SCOPE
