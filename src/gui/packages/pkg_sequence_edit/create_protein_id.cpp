/*  $Id: create_protein_id.cpp 40575 2018-03-14 14:55:43Z bollin $
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
#include <objects/general/Dbtag.hpp>
#include <objects/seq/seq_macros.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/cmd_create_feat.hpp>


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/textdlg.h> 

#include <gui/packages/pkg_sequence_edit/create_protein_id.hpp>



BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCreateProteinId::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, wxWindow *parent)
{
    if (!tse)
        return;

    string existing_db;
    for (CBioseq_CI b_iter(tse, CSeq_inst::eMol_aa); b_iter; ++b_iter)
    {
        FOR_EACH_SEQID_ON_BIOSEQ(id_it, *b_iter->GetBioseqCore())
        {
            if ((*id_it)->IsGeneral() && (*id_it)->GetGeneral().IsSetDb() && 
                (*id_it)->GetGeneral().GetDb() != "TMSMART" && (*id_it)->GetGeneral().GetDb() != "BankIt" && (*id_it)->GetGeneral().GetDb() != "NCBIFILE")
            {
                existing_db = (*id_it)->GetGeneral().GetDb();
                break;
            }
        }
        if (!existing_db.empty())
            break;
    }

    string dbname = wxGetTextFromUser(_("Database"),_("Create Protein IDs"), wxString(existing_db), parent).ToStdString();

    if (dbname.empty())
        return;

    CRef<CCmdComposite> cmd(new CCmdComposite("Create Protein Ids"));

    for (CFeat_CI feat_ci(tse, CSeqFeatData::e_Cdregion); feat_ci; ++feat_ci)
    {

        CConstRef<CSeq_feat> gene = sequence::GetBestGeneForCds(*feat_ci->GetSeq_feat(), tse.GetScope());

        if (gene && gene->GetData().GetGene().IsSetLocus_tag())
        {
            string locus_tag = gene->GetData().GetGene().GetLocus_tag();
            if (locus_tag.empty())
                continue;
            create_protein_ids(feat_ci->GetSeq_feat_Handle(), dbname, locus_tag, feat_ci->GetScope(), cmd);           
        }
    }
    
    cmdProcessor->Execute(cmd);
}

void CCreateProteinId::create_protein_ids(CSeq_feat_Handle fh, const string &dbname, const string &locus_tag, CScope &scope, CRef<CCmdComposite> cmd)
{
    const CSeq_feat &orig_feat = *fh.GetSeq_feat();
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(orig_feat);
    CSeq_loc &loc = new_feat->SetProduct();

    CRef<CSeq_id> id(new CSeq_id);
    id->SetGeneral().SetDb(dbname);
    id->SetGeneral().SetTag().SetStr(locus_tag);

    if (!fh.IsSetProduct())
        return;
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(fh.GetProduct());
    if (!prot_bsh || !prot_bsh.IsProtein()) 
        return;

    CRef<CSeq_entry> new_prot(new CSeq_entry);
    CRef<CBioseq> new_seq(new CBioseq);
    new_seq->Assign(*prot_bsh.GetBioseqCore());
    bool skip = false;
    FOR_EACH_SEQID_ON_BIOSEQ(id_it, *new_seq)
    {
        if ((*id_it)->IsGeneral() && (*id_it)->GetGeneral().IsSetDb() && 
            (*id_it)->GetGeneral().GetDb() != "TMSMART" && (*id_it)->GetGeneral().GetDb() != "BankIt" && (*id_it)->GetGeneral().GetDb() != "NCBIFILE")
        {
            skip = true;
            break;
        }
    }
    if (skip)
        return;
    CRef<CSeq_id> old_id;
    EDIT_EACH_SEQID_ON_BIOSEQ(id_it, *new_seq)
    {
        if ((*id_it)->IsLocal())
        {
            old_id = *id_it;
            *id_it = id;
            break;
        }
    }
    
    if (!old_id)
        new_seq->SetId().push_back(id);
    else
    {
        EDIT_EACH_SEQANNOT_ON_BIOSEQ(annot, *new_seq)
        {
            EDIT_EACH_SEQFEAT_ON_SEQANNOT(prot_feat, **annot)
            {
                if ((*prot_feat)->IsSetData() && (*prot_feat)->GetData().IsProt() && (*prot_feat)->GetLocation().GetId()->IsLocal() && (*prot_feat)->GetLocation().GetId()->Match(*old_id))
                {
                    (*prot_feat)->SetLocation().SetId(*id);
                }
            }
        }

        if (loc.GetId()->IsLocal() && loc.GetId()->Match(*old_id))
        {
            loc.SetId(*id);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
        }
    }
    new_prot->SetSeq(*new_seq);
    cmd->AddCommand(*CRef<CCmdChangeSeqEntry>(new CCmdChangeSeqEntry(prot_bsh.GetSeq_entry_Handle(), new_prot)));           
}

void CCreateLocusTagGene::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, wxWindow *parent)
{
    if (!tse)
        return;

//    string prefix = wxGetTextFromUser(_("Prefix"),_("Create locus-tag genes"), wxEmptyString, parent).ToStdString();
    CCreateLocusTagGenesDlg dlg(parent);
    if (dlg.ShowModal() != wxID_OK)
        return;
    string prefix = dlg.GetPrefix();
    
    if (prefix.empty())
        return;
    bool create_protein_ids = dlg.GetCreateProtIds();
    string dbname = dlg.GetDatabase();
    if (dbname.empty())
        create_protein_ids = false;

    CRef<CCmdComposite> cmd(new CCmdComposite("Create locus-tag genes"));
    int count = 5;
    for (CFeat_CI feat_ci(tse, CSeqFeatData::e_Cdregion); feat_ci; ++feat_ci)
    {

        CNcbiOstrstream ss;
        ss << prefix  << "_" << std::setw(4) << std::setfill('0') << count;
        string locus_tag = CNcbiOstrstreamToString(ss);

        CConstRef<CSeq_feat> gene = sequence::GetBestGeneForCds(*feat_ci->GetSeq_feat(), tse.GetScope());
        if (gene)
        {
            if (gene->GetData().GetGene().IsSetLocus_tag())
                continue;          

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(*gene);
            new_feat->SetData().SetGene().SetLocus_tag(locus_tag);
            CSeq_feat_Handle fh = tse.GetScope().GetSeq_featHandle(*gene);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
            count += 5;
        }
        else
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->SetData().SetGene().SetLocus_tag(locus_tag);
            CRef<CSeq_loc> loc(new CSeq_loc);
            loc->Assign(feat_ci->GetLocation());
            new_feat->SetLocation(*loc);
            if (feat_ci->IsSetPartial() && feat_ci->GetPartial()) {
                new_feat->SetPartial(true);
            }
            CBioseq_Handle bsh = tse.GetScope().GetBioseqHandle(feat_ci->GetLocation());
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
            count += 5;
        }

        if (create_protein_ids)
            CCreateProteinId::create_protein_ids(feat_ci->GetSeq_feat_Handle(), dbname, locus_tag, feat_ci->GetScope(), cmd);      
    }
    
    cmdProcessor->Execute(cmd);
}


/*
 * CCreateLocusTagGenesDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCreateLocusTagGenesDlg, wxDialog )


/*
 * CCreateLocusTagGenesDlg event table definition
 */

BEGIN_EVENT_TABLE( CCreateLocusTagGenesDlg, wxDialog )

////@begin CCreateLocusTagGenesDlg event table entries
////@end CCreateLocusTagGenesDlg event table entries

END_EVENT_TABLE()


/*
 * CCreateLocusTagGenesDlg constructors
 */

CCreateLocusTagGenesDlg::CCreateLocusTagGenesDlg()
{
    Init();
}

CCreateLocusTagGenesDlg::CCreateLocusTagGenesDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CCreateLocusTagGenesDlg creator
 */

bool CCreateLocusTagGenesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCreateLocusTagGenesDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCreateLocusTagGenesDlg creation
    return true;
}


/*
 * CCreateLocusTagGenesDlg destructor
 */

CCreateLocusTagGenesDlg::~CCreateLocusTagGenesDlg()
{
////@begin CCreateLocusTagGenesDlg destruction
////@end CCreateLocusTagGenesDlg destruction
}


/*
 * Member initialisation
 */

void CCreateLocusTagGenesDlg::Init()
{
////@begin CCreateLocusTagGenesDlg member initialisation
    m_Prefix = NULL;
    m_CreateProtIds = NULL;
    m_Database = NULL;
////@end CCreateLocusTagGenesDlg member initialisation
}


/*
 * Control creation for CCreateLocusTagGenesDlg
 */

void CCreateLocusTagGenesDlg::CreateControls()
{    
////@begin CCreateLocusTagGenesDlg content construction
    // Generated by DialogBlocks, 02/09/2015 10:52:05 (unregistered)

    CCreateLocusTagGenesDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Prefix"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Prefix = new wxTextCtrl( itemDialog1, ID_CREATE_LOCUSTAG_GENE_PREFIX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Prefix, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 5);

    m_CreateProtIds = new wxCheckBox( itemDialog1, ID_CREATE_LOCUSTAG_GENES_PROT_ID, _("Also create protein IDs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CreateProtIds->SetValue(false);
    itemBoxSizer6->Add(m_CreateProtIds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Database"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Database = new wxTextCtrl( itemDialog1, ID_CREATE_LOCUSTAG_GENE_DATABASE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_Database, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CCreateLocusTagGenesDlg content construction
}


/*
 * Should we show tooltips?
 */

bool CCreateLocusTagGenesDlg::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CCreateLocusTagGenesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCreateLocusTagGenesDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCreateLocusTagGenesDlg bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CCreateLocusTagGenesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCreateLocusTagGenesDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCreateLocusTagGenesDlg icon retrieval
}


END_NCBI_SCOPE
