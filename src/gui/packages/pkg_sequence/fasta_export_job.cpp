/*  $Id: fasta_export_job.cpp 38529 2017-05-23 14:48:11Z evgeniev $
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
* Authors: Roman Katargin
*
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seq_descr.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_vector.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_convert.hpp>

#include <gui/packages/pkg_sequence/fasta_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CFastaExportJob::CFastaExportJob(const CFastaExportParams& params)
    : CAppJob("FASTA Export"), m_Params(params)
{
}

IAppJob::EJobState CFastaExportJob::Run()
{
    bool master = m_Params.GetSaveMasterRecord();
    bool whole_seq = CFastaExportParams::eWholeSequences == (CFastaExportParams::EExportType)m_Params.GetExportType();
    wxString fname = m_Params.GetFileName();
    string err_msg;

    try {
        CNcbiOfstream ostr(fname.fn_str());
        CFastaOstream fasta_ostr(ostr);
        fasta_ostr.SetFlag(CFastaOstream::fAssembleParts);
        fasta_ostr.SetFlag(CFastaOstream::fInstantiateGaps);

        // Do not dump duplicate seq_ids
        TLocVector exported_locs;

        const TConstScopedObjects& objects = m_Params.GetObjects();
        ITERATE(TConstScopedObjects, it, objects) {
            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(it->object.GetPointer());
            if (nullptr == loc)
                continue;

            CScope& scope = const_cast<CScope&>(*(it->scope));

            if (master) {
                CObjectConverter::TObjList objs;
                CObjectConverter::Convert(scope, *loc, CSeq_entry::GetTypeInfo(), objs);
                ITERATE(CObjectConverter::TObjList, iter, objs) {
                    const CSeq_entry& entry =
                        dynamic_cast<const CSeq_entry&>(iter->GetObject());
                    CBioseq_CI bioseq_iter(scope, entry);
                    for (; bioseq_iter; ++bioseq_iter) {
                        fasta_ostr.Write(*bioseq_iter);
                    }
                }
            }
            else if (whole_seq) {
                CConstRef<CSeq_loc> whole_loc = x_GetWholeLoc(*loc, exported_locs, scope);
                if (whole_loc.Empty())
                    continue;

                CBioseq_Handle handle = scope.GetBioseqHandle(*whole_loc);
                fasta_ostr.Write(handle, whole_loc);
            }
            else {
                if (x_CheckForDuplicates(*loc, exported_locs, scope))
                    continue;

                CBioseq_Handle handle = scope.GetBioseqHandle(*loc);
                fasta_ostr.Write(handle, loc);
            }
        }
    }
    catch (CException& e) {
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CFastaExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    }
    else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

bool CFastaExportJob::x_CheckForDuplicates(const CSeq_loc &loc, TLocVector &loc_vector, CScope& scope) const
{
    CRef<CSeq_loc> best_id_loc(new CSeq_loc);
    best_id_loc->Assign(loc);

    const CSeq_id* id = loc.GetId();
    if (nullptr == id)
        return false;

    CSeq_id_Handle best_idh = sequence::GetId(*id, scope, sequence::eGetId_Best);
    best_id_loc->SetId(*best_idh.GetSeqId());
 
    for (auto &ex_loc : loc_vector) {
        if (ex_loc->Equals(*best_id_loc))
            return true;
    }

    loc_vector.push_back(best_id_loc);
    return false;
}

CConstRef<CSeq_loc> CFastaExportJob::x_GetWholeLoc(const CSeq_loc &loc, TLocVector &loc_vector, CScope& scope) const
{
    CRef<CSeq_loc> best_id_loc;

    const CSeq_id* id = loc.GetId();
    if (nullptr == id)
        return CConstRef<CSeq_loc>();

    CSeq_id_Handle best_idh = sequence::GetId(*id, scope, sequence::eGetId_Best);
    CBioseq_Handle bsh = scope.GetBioseqHandle(best_idh);
    if (!bsh)
        return CConstRef<CSeq_loc>();

    best_id_loc.Reset(new CSeq_loc);
    best_id_loc->SetId(*best_idh.GetSeqId());
    best_id_loc->SetWhole().Assign(*best_idh.GetSeqId());

    for (auto &ex_loc : loc_vector) {
        if (ex_loc->Equals(*best_id_loc))
            return CConstRef<CSeq_loc>();
    }

    loc_vector.push_back(best_id_loc);

    return best_id_loc;
}


END_NCBI_SCOPE

