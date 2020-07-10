/*  $Id: cmd_change_seq_feat.cpp 33112 2015-05-28 11:54:16Z bollin $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/util/feature.hpp>
#include <objtools/edit/cds_fix.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


void CCmdChangeSeq_feat::Execute()
{
    x_Switch();
    if (m_PromoteRequested) {
        m_ShouldDemote = feature::PromoteCDSToNucProtSet(m_Orig_feat);
    }

}

void CCmdChangeSeq_feat::Unexecute()
{
    x_Switch();
    if (m_ShouldDemote) {
        edit::DemoteCDSToNucSeq(m_Orig_feat);
    }
}

string CCmdChangeSeq_feat::GetLabel()
{
    return "Modify feature";
}

void CCmdChangeSeq_feat::x_Switch()
{
    // This is necessary, to make sure that we are in "editing mode"
    const CSeq_annot_Handle& annot_handle = m_Orig_feat.GetAnnot();
    CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();

    // Now actually edit the feature
    CConstRef<CSeq_feat> saveFeat = m_Orig_feat.GetOriginalSeq_feat();
    CSeq_feat_EditHandle feh(m_Orig_feat);
    CSeq_entry_Handle parent_entry = feh.GetAnnot().GetParentEntry();

    feh.Replace(*m_New_feat);

    m_New_feat = saveFeat;
}

END_NCBI_SCOPE
