/*  $Id: convert_bad_inference.cpp 39643 2017-10-23 18:53:38Z filippov $
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
#include <objtools/validator/validatorp.hpp>
#include <objtools/validator/validerror_feat.hpp>
#include <gui/packages/pkg_sequence_edit/convert_bad_inference.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRef<CCmdComposite> CConvertBadInference::apply(CSeq_entry_Handle tse)
{
    CRef<CCmdComposite> cmd;
    if (!tse)
        return cmd;
    cmd.Reset(new CCmdComposite("Convert Bad Inference"));
    CScope& scope = tse.GetScope();
    bool modified = false;

    for (CFeat_CI fi(tse); fi; ++fi)
    {         
        const CSeq_feat& feat = fi->GetOriginalFeature();
        vector<string> comments;
        if (feat.IsSetComment())
            comments.push_back(feat.GetComment());

        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat);
        bool found = false;
        EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *new_feat)
        {
            if ((*gbqual_it) && (*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == "inference" &&
                (*gbqual_it)->IsSetVal() && validator::CValidError_feat::ValidateInference((*gbqual_it)->GetVal(), false) != validator::CValidError_feat::eInferenceValidCode_valid)
            {              
                found = true;
                comments.push_back((*gbqual_it)->GetVal());
                ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *new_feat);
            }
        }

        if (new_feat->IsSetQual() && new_feat->GetQual().empty())
            new_feat->ResetQual();

        if (found)
        {            
            new_feat->SetComment(NStr::Join(comments, "; "));        
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fi->GetSeq_feat_Handle(), *new_feat)));
            modified = true;       
        }
    }
    if (!modified)
        cmd.Reset();
    return cmd;
}


END_NCBI_SCOPE
