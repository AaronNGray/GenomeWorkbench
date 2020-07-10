/*  $Id: remove_xrefs.cpp 42313 2019-01-31 16:20:31Z asztalos $
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
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/general/Dbtag.hpp>
#include <gui/packages/pkg_sequence_edit/remove_xrefs.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


void CRemoveDbXrefsFeats::ModifyFeature(CRef<CSeq_feat> feat)
{
   feat->ResetDbxref();
}

void CRemoveDbXrefsFeats::rm_dbxref_from_feat( CSeq_entry_Handle seh, CSeqFeatData::E_Choice type, ICommandProccessor* cmd_processor)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove DBXrefs From Features"));
    FindFeatures(seh,type,cmd);
    cmd_processor->Execute(cmd);
}



void CRemoveDbXrefsBioSource::ModifyBioSource(CBioSource& biosource)
{
    biosource.SetOrg().ResetDb();
}

void CRemoveDbXrefsBioSource::rm_dbxref_from_biosource( CSeq_entry_Handle seh, ICommandProccessor* cmd_processor)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove DBXrefs From BioSource"));
    FindBioSource(seh,cmd);
    cmd_processor->Execute(cmd);
}

void CRemoveDbXrefsBioSourceAndFeats::rm_dbxref_from_biosource_and_feats( CSeq_entry_Handle seh, ICommandProccessor* cmd_processor)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove DBXrefs From BioSource And Features"));
    CRemoveDbXrefsFeats::FindFeatures(seh,CSeqFeatData::e_not_set,cmd);
    CRemoveDbXrefsBioSource::FindBioSource(seh,cmd);
    cmd_processor->Execute(cmd);
}

void CRemoveTaxonFeats::ModifyFeature(CRef<CSeq_feat> feat)
{
    if (!feat->IsSetDbxref())
        return;

    CSeq_feat::TDbxref::iterator dbtag =  feat->SetDbxref().begin();
    while ( dbtag != feat->SetDbxref().end() )
    {
        if ((*dbtag)->IsSetDb() && (*dbtag)->GetDb() == "taxon")
        {
            dbtag = feat->SetDbxref().erase(dbtag);
        }
        else
            ++dbtag;
    }
    if (feat->SetDbxref().empty())
        feat->ResetDbxref();
}

void CRemoveTaxonFeats::rm_taxon_from_feats( CSeq_entry_Handle seh, ICommandProccessor* cmd_processor)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove Taxons From Features"));
    FindFeatures(seh,CSeqFeatData::e_not_set,cmd);
    cmd_processor->Execute(cmd);
}

void CRemoveTaxonBioSource::ModifyBioSource(CBioSource& biosource)
{
    COrg_ref::TDb::iterator dbtag =  biosource.SetOrg().SetDb().begin();
    while ( dbtag != biosource.SetOrg().SetDb().end() )
    {
        if ((*dbtag)->IsSetDb() && (*dbtag)->GetDb() == "taxon")
        {
            dbtag = biosource.SetOrg().SetDb().erase(dbtag);
        }
        else
            ++dbtag;
    }
    if (biosource.GetOrg().GetDb().empty())
        biosource.SetOrg().ResetDb();
}

void CRemoveTaxonFeatsAndBioSource::rm_taxon_from_feats_and_biosource( CSeq_entry_Handle seh, ICommandProccessor* cmd_processor)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove Taxons From Features and BioSource"));
    CRemoveTaxonFeats::FindFeatures(seh,CSeqFeatData::e_not_set,cmd);
    CRemoveTaxonBioSource::FindBioSource(seh,cmd);
    cmd_processor->Execute(cmd);
}

END_NCBI_SCOPE
