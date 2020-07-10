/*  $Id: remove_unnecessary_exceptions.cpp 39517 2017-10-04 15:23:59Z filippov $
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
#include <objtools/validator/utilities.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/remove_unnecessary_exceptions.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;



void CRemoveUnnecessaryExceptions::apply(objects::CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor)
{
    if (!tse) 
        return;

    bool modified = false;
    CRef<CCmdComposite> composite(new CCmdComposite("Remove Unnecessary Exceptions"));    
    for (CFeat_CI feat_ci(tse); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        if (validator::DoesFeatureHaveUnnecessaryException(orig, tse.GetScope()))
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(orig);
            new_feat->ResetExcept();
            new_feat->ResetExcept_text();
            composite->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
            modified = true;
        }
    }
    if (modified)
        cmdProcessor->Execute(composite.GetPointer());
}


END_NCBI_SCOPE
