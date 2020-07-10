/*  $Id: clear_nomenclature.cpp 37224 2016-12-16 14:54:23Z filippov $
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
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/clear_nomenclature.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CRef<CCmdComposite> CClearNomenclature::apply(CSeq_entry_Handle tse)
{
    if (!tse) 
        return CRef<CCmdComposite>(NULL);
    bool modified = false;
    CRef<CCmdComposite> composite( new CCmdComposite("Clear Nomenclature") );   
    for (CFeat_CI feat_ci(tse, CSeqFeatData::eSubtype_gene); feat_ci; ++feat_ci)
    {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_ci->GetOriginalFeature());
      
        if (new_feat->IsSetData() && new_feat->GetData().IsGene() && new_feat->GetData().GetGene().IsSetFormal_name())
        {
            new_feat->SetData().SetGene().ResetFormal_name();
            composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }
    
    if (!modified)
        composite.Reset();
    return composite;
}


END_NCBI_SCOPE
