/*  $Id: remove_unindexed_features.cpp 38984 2017-07-14 19:48:54Z filippov $
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
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <objtools/validator/utilities.hpp>
#include <objtools/validator/validatorp.hpp>
#include <gui/packages/pkg_sequence_edit/remove_unindexed_features.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

 
// feature must have location on at least one sequence in this record
// feature location must not extend past end of sequence
// copied from CValidError_annot where it was made hard to use outside of the whole validator run.
bool CRemoveUnindexedFeatures::IsLocationUnindexed(const CSeq_loc& loc, CSeq_entry_Handle tse, CScope &scope)
{
    bool found_one = false;
    for (CSeq_loc_CI loc_it(loc); loc_it; ++loc_it) {
        try
        {
            const CSeq_id& id = loc_it.GetSeq_id();       
            CBioseq_Handle in_record = scope.GetBioseqHandleFromTSE(id, tse);
            if (in_record) 
            {
                found_one = true;
                if (!loc_it.IsWhole() && loc_it.GetRange().GetFrom() > in_record.GetBioseqLength() - 1) 
                {
                    return true;
                }
            }
        }
        catch(exception &e)
        {
            return true;
        }
    }
    return !found_one;
}

CRef<CCmdComposite> CRemoveUnindexedFeatures::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    cmd.Reset(new CCmdComposite("Remove Unindexed Features"));
    CScope& scope = tse.GetScope();
    bool modified = false;

    for (CFeat_CI fi(tse); fi; ++fi)
    {         
        const CSeq_loc& feat_loc = fi->GetLocation();
        bool is_nc = false, is_emb = false;
        CBioseq_Handle bsh;
        try
        {
            bsh = scope.GetBioseqHandle(feat_loc);
        }
        catch(CObjMgrException&) {}
        
        if (bsh)
        {
            CConstRef<CBioseq> b = bsh.GetCompleteBioseq();
            FOR_EACH_SEQID_ON_BIOSEQ (seq_it, *b) 
            {
                if ((*seq_it)->IsEmbl()) 
                {
                    is_emb = true;
                } 
                else if ((*seq_it)->IsOther()) 
                {
                    if ((*seq_it)->GetOther().IsSetAccession() && NStr::StartsWith((*seq_it)->GetOther().GetAccession(), "NT_")) 
                    {
                        is_nc = true;
                    }
                }
            }
        }

        if ( IsLocationUnindexed(feat_loc, tse, scope)
             || (!is_nc  &&  !is_emb && validator::IsFarLocation(feat_loc, tse))  
             || validator::CValidError_imp::BadMultipleSequenceLocation(feat_loc, scope))
        {
            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fi->GetSeq_feat_Handle())));
            modified = true;       
        }
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}


END_NCBI_SCOPE
