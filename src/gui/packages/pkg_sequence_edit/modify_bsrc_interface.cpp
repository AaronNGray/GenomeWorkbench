/*  $Id: modify_bsrc_interface.cpp 42254 2019-01-22 15:22:29Z asztalos $
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
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/modify_bsrc_interface.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


void IModifyBiosourceQuals::x_ApplyToSeqAndFeat(CCmdComposite* composite)
{
    x_ApplyToDescriptors(*(m_Seh.GetCompleteSeq_entry()), composite);

    for (CFeat_CI feat_it(m_Seh, SAnnotSelector(CSeqFeatData::e_Biosrc)); feat_it; ++feat_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_it->GetOriginalFeature());
        if (x_ApplyToBioSource(new_feat->SetData().SetBiosrc())) {
            CRef<CCmdChangeSeq_feat> cmd(new CCmdChangeSeq_feat(*feat_it, *new_feat));
            composite->AddCommand(*cmd);
        }
    }
}

void IModifyBiosourceQuals::x_ApplyToDescriptors(const CSeq_entry& se, CCmdComposite* composite)
{
    FOR_EACH_SEQDESC_ON_SEQENTRY(it, se) {
        if ((*it)->IsSource()) {
            const CSeqdesc& orig_desc = **it;
            CRef<CSeqdesc> new_desc(new CSeqdesc);
            new_desc->Assign(orig_desc);
            if (x_ApplyToBioSource(new_desc->SetSource())) {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_Seh.GetScope().GetSeq_entryHandle(se), orig_desc, *new_desc));
                composite->AddCommand(*cmd);
            }
        }
    }

    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet()) {
            x_ApplyToDescriptors(**it, composite);
        }
    }
}

CRef<CCmdComposite> IModifyBiosourceQuals::x_GetCommand(CSeq_entry_Handle tse, const string& title)
{
    if (!tse)
        return CRef<CCmdComposite>();

    m_Seh = tse;
    CRef<CCmdComposite> composite(new CCmdComposite(title));
    x_ApplyToSeqAndFeat(composite);
    return composite;
}

END_NCBI_SCOPE

