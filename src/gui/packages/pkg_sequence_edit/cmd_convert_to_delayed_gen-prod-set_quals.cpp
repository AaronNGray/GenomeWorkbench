/*  $Id: cmd_convert_to_delayed_gen-prod-set_quals.cpp 41022 2018-05-10 15:14:39Z katargir $
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

#include <objmgr/feat_ci.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include "cmd_convert_to_delayed_gen-prod-set_quals.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CIRef<IEditCommand> CCmdConvertToDelayedGenProdSetQuals::x_CreateActionCmd()
{
    CRef<CCmdComposite> composite( new CCmdComposite("Convert to Delayed Gen-Prod-Set Qualifiers") );   
    for (CFeat_CI feat_ci(m_seh); feat_ci; ++feat_ci)
    {        
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        if (orig.IsSetData() 
            && (orig.GetData().GetSubtype() ==  CSeqFeatData::eSubtype_cdregion ||  orig.GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA))
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(orig);
            bool changed = false;  
            EDIT_EACH_GBQUAL_ON_SEQFEAT(qual, *new_feat)
            {
                if ((*qual)->IsSetQual())
                {
                    if ((*qual)->GetQual() == "protein_id")
                    {
                        (*qual)->SetQual() = "orig_protein_id";
                        changed = true;
                    }
                    if ((*qual)->GetQual() == "transcript_id")
                    {
                        (*qual)->SetQual() = "orig_transcript_id";
                        changed = true;
                    }
                }
            }   
            if (changed)
            {
                composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            }
        }
    }

    return CIRef<IEditCommand>(composite);
}

string CCmdConvertToDelayedGenProdSetQuals::GetLabel()
{
    return "Convert to Delayed Gen-Prod-Set Qualifiers";
}

END_NCBI_SCOPE
