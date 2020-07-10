/*  $Id: agp_export_job.cpp 22810 2010-12-23 21:11:40Z katargir $
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

#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/util/sequence.hpp>
#include <serial/iterator.hpp>

#include <gui/objutils/label.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objtools/writers/agp_write.hpp>

#include <gui/packages/pkg_sequence/agp_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CAgpExportJob::CAgpExportJob(const CAgpExportParams& params)
: CAppJob("AGP Export"), m_Params(params)
{
}

IAppJob::EJobState CAgpExportJob::Run()
{
    SConstScopedObject seqLoc = m_Params.GetSeqLoc();
    const CSeq_loc* loc =
        dynamic_cast<const CSeq_loc*> (seqLoc.object.GetPointer());
    CScope& scope = seqLoc.scope.GetObject();
    CBioseq_Handle handle = scope.GetBioseqHandle(*loc);

    wxString fname = m_Params.GetFileName();

    string err_msg;

    /// make sure we get a good object-id to show here
    string object_id =  ToStdString(m_Params.GetAltObjId());

    if (object_id.empty()) {
        const CSeq_id* id = NULL;
        try {
            id = &sequence::GetId(*loc, &scope);
        }
        catch (CException&) {
            CTypeConstIterator<CSeq_id> id_iter(*loc);
            if (id_iter) {
                id = &*id_iter;
            }
        }
        if (id) {
            CLabel::GetLabel(*id, &object_id, CLabel::eDefault, &scope);
        } else {
            object_id = "(unknown)";
        }
    }

    try {
        CNcbiOfstream ostr(fname.fn_str());
        AgpWrite(ostr, handle,
            loc->GetTotalRange().GetFrom(),
            loc->GetTotalRange().GetTo(),
            object_id,
            m_Params.GetGapType(),
            m_Params.GetAssertEvidence());
    }
    catch (CException& e) {
        err_msg = "AGP export. Failed to save file: " + string(fname.ToUTF8()) + "\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CAgpExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
