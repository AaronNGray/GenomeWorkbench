/*  $Id: cmd_assign_feat_local_ids.cpp 42162 2019-01-07 20:09:12Z filippov $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>

#include <objmgr/util/sequence.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include "cmd_assign_feat_local_ids.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CIRef<IEditCommand> CCmdAssignFeatLocalIds::x_CreateActionCmd()
{
    CRef<CCmdComposite> result;
    CObject_id::TId top_id = CFixFeatureId::s_FindHighestFeatureId(m_seh);

    for (CFeat_CI feat_it(m_seh); feat_it; ++feat_it) {
        CSeq_feat_Handle fh = feat_it->GetSeq_feat_Handle();
        if (fh.IsSetId() && fh.GetId().IsLocal()) {
            continue;
        }

        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(*feat_it->GetOriginalSeq_feat());
        top_id++;
        new_feat->SetId().SetLocal().SetId(top_id);

        if (!result)
            result.Reset(new CCmdComposite("Assign feature local ids"));

        CIRef<IEditCommand> chg_feat(new CCmdChangeSeq_feat(fh, *new_feat));
        result->AddCommand(*chg_feat);
    }

    return CIRef<IEditCommand>(result);
}

string CCmdAssignFeatLocalIds::GetLabel()
{
    return "Assign feature local ids";
}

END_NCBI_SCOPE
