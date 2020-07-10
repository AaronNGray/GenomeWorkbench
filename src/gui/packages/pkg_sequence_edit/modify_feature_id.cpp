/*  $Id: modify_feature_id.cpp 43893 2019-09-16 15:16:49Z filippov $
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
#include <objects/seqfeat/Feat_id.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/modify_feature_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);



bool CAssignFeatureId::AssignHighestFeatureId(const CSeq_entry_Handle& entry, CObject_id::TId& feat_id, CCmdComposite* composite)
{
    bool modified = false;
    for (CFeat_CI feat_it(entry); feat_it; ++feat_it)
    {
        CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        if (fh.IsSetId() && fh.GetId().IsLocal()) continue;

        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(feat_it->GetOriginalFeature());
        feat_id++;
        new_feat->SetId().SetLocal().SetId(feat_id);
        
        CIRef<IEditCommand> cmd(new CCmdChangeSeq_feat(fh, *new_feat));
        composite->AddCommand(*cmd);
        modified = true;
    }
    return modified;
}

bool CAssignFeatureId::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) 
    {
        CObject_id::TId top_id = CFixFeatureId::s_FindHighestFeatureId(tse);
        CRef<CCmdComposite> composite(new CCmdComposite(title)); 
        bool modified = AssignHighestFeatureId(tse, top_id, composite);
        if (modified)
        {
           cmdProcessor->Execute(composite.GetPointer());
           return true;
       }
    }
    return false;
}

void CClearFeatureId::ClearFeatureIds(const CSeq_entry_Handle& entry, CCmdComposite* composite)
{
    for ( CFeat_CI feat_it(entry); feat_it; ++feat_it ) 
    {
        bool modified = false;
        
        CRef<CSeq_feat> edited(new CSeq_feat);
        edited->Assign(feat_it->GetOriginalFeature());
        
        if (edited->IsSetId())    
        {    
            edited->ResetId();  
            modified = true;
        }
       if (edited->CanGetXref())
        {
            CSeq_feat::TXref::iterator xref_it = edited->SetXref().begin();
            while ( xref_it != edited->SetXref().end() )
            {
                if ((*xref_it)-> IsSetId())
                {
                    (*xref_it)->ResetId();
                    modified = true;
                }
                if (!(*xref_it)-> IsSetData()) 
                {
                    xref_it = edited->SetXref().erase(xref_it);
                    modified = true;
                }
                else
                    ++xref_it;
            }
            if (edited->SetXref().empty())
            {
                edited->ResetXref();
                modified = true;
            }
        }
       if (modified)
       {
           CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
           CIRef<IEditCommand> cmd(new CCmdChangeSeq_feat(fh, *edited));
           composite->AddCommand(*cmd);
       }
    }
}


bool CClearFeatureId::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) 
    {
        CRef<CCmdComposite> composite(new CCmdComposite(title)); 
        ClearFeatureIds(tse, composite);
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    return false;
}

bool CReassignFeatureId::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    if (tse) 
    {
        map<CSeq_feat_Handle, CRef<CSeq_feat> > changed_feats;
        CFixFeatureId::s_ReassignFeatureIds(tse, changed_feats);
        if (!changed_feats.empty())
        {
            CRef<CCmdComposite> composite(new CCmdComposite(title)); 
            for (auto &fh_feat : changed_feats)
            {
                CIRef<IEditCommand> cmd(new CCmdChangeSeq_feat(fh_feat.first, *fh_feat.second));
                composite->AddCommand(*cmd);
            }
            cmdProcessor->Execute(composite.GetPointer());
            return true;
        }
    }
    return false;
}

bool CUniqifyFeatureId::apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title)
{
    map<CSeq_feat_Handle, CRef<CSeq_feat> > changed_feats;
    CFixFeatureId::s_ApplyToSeqInSet(tse, changed_feats);

    if (!changed_feats.empty())
    {
        CRef<CCmdComposite> composite(new CCmdComposite(title)); 
        for (auto &fh_feat : changed_feats)
        {
            CIRef<IEditCommand> cmd(new CCmdChangeSeq_feat(fh_feat.first, *fh_feat.second));
            composite->AddCommand(*cmd);
        }
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    return false;
}


END_NCBI_SCOPE
