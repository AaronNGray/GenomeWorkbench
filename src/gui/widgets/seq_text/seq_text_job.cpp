/*  $Id: seq_text_job.cpp 30100 2014-04-08 10:56:52Z bollin $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin, based on a file by Vlad Lebedev
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/seq_text/seq_text_job.hpp>

#include <gui/objutils/utils.hpp>

#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/seq_map_switch.hpp>
#include <objmgr/graph_ci.hpp>

#include <gui/objutils/seqfetch_queue.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CSeqTextJob::CSeqTextJob(const CSeq_loc& loc,
                         CScope& scope,
                         TJobTask task)
    : m_Loc(loc),
      m_Scope(scope),
      m_TaskID(task)
{
    m_Sel = CSeqUtils::GetAnnotSelector();
}


IAppJob::EJobState CSeqTextJob::Run()
{
    m_Progress.Reset(NULL);
    m_Error.Reset(NULL);
    m_Result.Reset(NULL);

    switch (m_TaskID) {
        case eLoadFeatureTypesTask:
            x_LoadFeatureTypes();
            break;
        default:
            break;
    }
    // TODO properly handle canceling and exceptions, return eCanceled and
    // eFailed if needed
    return eCompleted;
}


void CSeqTextJob::x_LoadFeatureTypes()
{
    m_Result.Reset(new CSeqTextJobResult());


    unsigned int i;

    m_Result->m_AvailableSubtypes.clear();

    for (i = CSeqFeatData::eSubtype_bad; i < CSeqFeatData::eSubtype_any; i++) {
        m_Result->m_AvailableSubtypes.push_back (false);
    }
    CFeat_CI feat_ci(m_Scope, m_Loc, m_Sel);
    while (feat_ci)
    {
        int subtype = feat_ci->GetFeatSubtype();
        m_Result->m_AvailableSubtypes [subtype] = true;

        ++feat_ci;
    }
}


CConstIRef<IAppJobProgress> CSeqTextJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>(m_Progress.GetPointer());
}


CRef<CObject> CSeqTextJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CSeqTextJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CSeqTextJob::GetDescr() const
{
    return "CSeqTextJob";
}



END_NCBI_SCOPE
