/*  $Id: remove_xrefs.hpp 42340 2019-02-05 14:40:28Z asztalos $
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


#ifndef _REMOVE_XREFS_H_
#define _REMOVE_XREFS_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/utils/command_processor.hpp>


BEGIN_NCBI_SCOPE

template<typename T>
class CRemoveXrefs
{
public:
    static void FindFeatures(objects::CSeq_entry_Handle seh, objects::CSeqFeatData::E_Choice type, CRef<CCmdComposite> cmd);
    static void ModifyFeature(CRef<objects::CSeq_feat> feat) {T::ModifyFeature(feat);}
    static void ModifyBioSource(objects::CBioSource& biosource) {T::ModifyBioSource(biosource);}
    static bool ChangeBioSource(objects::CBioSource& biosource);
    static void GetDesc(const objects::CSeq_entry& se, objects::CScope& scope, CRef<CCmdComposite> composite);
    static void FindBioSource(objects::CSeq_entry_Handle tse, CRef<CCmdComposite> composite);
};

template<typename T>
void CRemoveXrefs<T>::FindFeatures(objects::CSeq_entry_Handle seh, objects::CSeqFeatData::E_Choice type, CRef<CCmdComposite> cmd)
{
    for (objects::CFeat_CI feat_ci(seh, objects::SAnnotSelector(type)); feat_ci; ++feat_ci)
    {
        const objects::CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CRef<objects::CSeq_feat> new_feat(new objects::CSeq_feat());
        new_feat->Assign(orig);
             
        if (new_feat->IsSetDbxref())
        {
            ModifyFeature(new_feat);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
        }
    }
}

template<typename T>
bool CRemoveXrefs<T>::ChangeBioSource(objects::CBioSource& biosource)
{
    if (biosource.IsSetOrg() && biosource.GetOrg().IsSetDb())
    {
        ModifyBioSource(biosource);
        return true;
    }
    return false;
}

USING_SCOPE(objects);
template<typename T>
void CRemoveXrefs<T>::GetDesc(const objects::CSeq_entry& se, objects::CScope& scope, CRef<CCmdComposite> composite)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            const objects::CSeqdesc& orig_desc = **it;
            CRef<objects::CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            if (ChangeBioSource(new_desc->SetSource())) {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(scope.GetSeq_entryHandle(se), orig_desc, *new_desc));
                composite->AddCommand(*cmd);
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            GetDesc(**it, scope, composite);
        }
    }
}

template<typename T>
void CRemoveXrefs<T>::FindBioSource(objects::CSeq_entry_Handle tse, CRef<CCmdComposite> composite)
{
    GetDesc(*(tse.GetCompleteSeq_entry()), tse.GetScope(), composite);

    for (objects::CFeat_CI feat_it(tse, objects::SAnnotSelector(objects::CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        CRef<objects::CSeq_feat> new_feat(new objects::CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        if (ChangeBioSource(new_feat->SetData().SetBiosrc())) {
            CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
            composite->AddCommand(*cmd);
        }
    }
}

class CRemoveDbXrefsFeats : public CRemoveXrefs<CRemoveDbXrefsFeats>
{
public:
    static void rm_dbxref_from_feat(objects::CSeq_entry_Handle seh, objects::CSeqFeatData::E_Choice type, ICommandProccessor* cmd_processor);
    static void ModifyFeature(CRef<objects::CSeq_feat> feat);
};

class CRemoveDbXrefsBioSource : public CRemoveXrefs<CRemoveDbXrefsBioSource>
{
public:
    static void rm_dbxref_from_biosource(objects::CSeq_entry_Handle seh, ICommandProccessor* cmd_processor);
    static void ModifyBioSource(objects::CBioSource& biosource);
};

class CRemoveDbXrefsBioSourceAndFeats : public CRemoveXrefs<CRemoveDbXrefsBioSourceAndFeats>
{
public:
    static void rm_dbxref_from_biosource_and_feats(objects::CSeq_entry_Handle seh, ICommandProccessor* cmd_processor);
};


class CRemoveTaxonFeats : public CRemoveXrefs<CRemoveTaxonFeats>
{
public:
    static void rm_taxon_from_feats(objects::CSeq_entry_Handle seh, ICommandProccessor* cmd_processor);
    static void ModifyFeature(CRef<objects::CSeq_feat> feat);
};

class CRemoveTaxonBioSource : public CRemoveXrefs<CRemoveTaxonBioSource>
{
public:
    static void ModifyBioSource(objects::CBioSource& biosource);
};

class CRemoveTaxonFeatsAndBioSource : public CRemoveXrefs<CRemoveTaxonFeatsAndBioSource>
{
public:
    static void rm_taxon_from_feats_and_biosource(objects::CSeq_entry_Handle seh, ICommandProccessor* cmd_processor);
};

END_NCBI_SCOPE

#endif  // _REMOVE_XREFS_H_
