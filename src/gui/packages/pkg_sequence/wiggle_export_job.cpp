/*  $Id: wiggle_export_job.cpp 37888 2017-02-28 15:44:33Z shkeda $
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
 * Authors: Vladislav Evgeniev
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include <objtools/writers/wiggle_writer.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence/wiggle_export_job.hpp>
#include <gui/packages/pkg_sequence/bam_utils.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CWiggleExportJob::CWiggleExportJob(const CWiggleExportParams& params)
: CAppJob("Wiggle Export"), m_Params(params)
{
}


CRef<CSeq_annot> CWiggleExportJob::x_RecalculateGraph(const CSeq_annot& seq_annot, CScope& scope)
{

    CRef<CSeq_annot> annot(0);
    string bam_data;
    string bam_index;
    string assembly;
    if (!CBamUtils::GetCoverageGraphInfo(seq_annot, bam_data, bam_index, assembly))
        return annot;
    string path, file, ext;
    CFile::SplitPath(bam_data, &path, &file, &ext);
    if (bam_index.empty())
        bam_index = path + file + ".bai";
    CBamUtils::TSeqIdsVector ids;
    for (auto&& graph : seq_annot.GetData().GetGraph()) {
        if (graph->CanGetComp() && graph->GetComp() == m_Params.GetBinSize())
            continue;
        auto id = graph->GetLoc().GetId();
        _ASSERT(id);
        if (!id)
            continue;
        ids.push_back(ConstRef(id));
    }

    if (ids.empty())
        return annot;
    x_SetStatusText("Creating coverage graphs");
    annot = CBamUtils::CreateCoverageGraphs(bam_data, bam_index, file, assembly, m_Params.GetBinSize(), &ids);
    return annot;
}

IAppJob::EJobState CWiggleExportJob::Run()
{
    string err_msg;

    wxString fileName = m_Params.GetFileName();

    try {
        ios::openmode mode = ios::out;
        TSeqRange range;

        CNcbiOfstream os(fileName.fn_str(), mode);
        CWiggleWriter writer(os);
        writer.SetCanceler(x_GetICanceled());
        /*
        Export by range is not supported yet
        if (!m_Params.GetFrom().empty() && !m_Params.GetTo().empty()) {
            auto from = NStr::StringToInt(ToStdString(m_Params.GetFrom()), NStr::fConvErr_NoThrow);
            auto to = NStr::StringToInt(ToStdString(m_Params.GetTo()), NStr::fConvErr_NoThrow);
            if (from > 0 && to > 0 && from < to)
                writer.SetRange().Set(from, to);
        }
        */

        ITERATE(TConstScopedObjects, it, m_Params.GetObjects()) {
            const CSeq_annot *seq_annot = dynamic_cast<const CSeq_annot*>(it->object.GetPointer());
            if (!seq_annot)
                continue;
            CRef<CSeq_annot> new_annot = x_RecalculateGraph(*seq_annot, const_cast<CScope&>(*it->scope.GetPointer()));
            x_SetStatusText("Exporting data");
            writer.WriteAnnot(new_annot ? *new_annot : *seq_annot);
        }
        writer.WriteFooter();
    }
    catch (CException& e) {
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CWiggleExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
