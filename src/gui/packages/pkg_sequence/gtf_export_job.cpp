/*  $Id: gtf_export_job.cpp 37293 2016-12-21 19:29:31Z katargir $
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

#include <objtools/writers/gtf_writer.hpp>

#include <objmgr/feat_ci.hpp>

#include <gui/packages/pkg_sequence/gtf_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CGtfExportJob::CGtfExportJob(const CGtfExportParams& params)
: CAppJob("GTF Export"), m_Params(params)
{
}

IAppJob::EJobState CGtfExportJob::Run()
{
    string err_msg;

    try {
        const TConstScopedObjects& objects = m_Params.GetObjects();

        if (objects.empty()) {
            m_Error.Reset(new CAppJobError("No objects specified for export."));
            return eFailed;
        }

        CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
        scope->AddDefaults();

        CNcbiOfstream ostr(m_Params.GetFileName().fn_str());

        unsigned int flags = 0;
        if (m_Params.GetSuppressGeneFeatures())
            flags |= CGtfWriter::fNoGeneFeatures;
        if (m_Params.GetSuppressExonNumbers())
            flags |= CGtfWriter::fNoExonNumbers;

        unsigned long depth;
        m_Params.GetFeatureDepth().ToULong(&depth);

        CGtfWriter writer(*scope, ostr, flags);

        writer.WriteHeader();
        ITERATE(TConstScopedObjects, it, objects) {
            const CSeq_annot* seqAnnot = dynamic_cast<const CSeq_annot*> (it->object.GetPointer());
            if (seqAnnot) {
                CRef<CSeq_annot> annot(new CSeq_annot());
                annot->Assign(*seqAnnot);
                writer.WriteAnnot(*annot);
                continue;
            }

            const CSeq_align* seqAlign = dynamic_cast<const CSeq_align*> (it->object.GetPointer());
            if (seqAlign) {
                writer.WriteAlign(*seqAlign);
                continue;
            }

            const CSeq_entry* seqEntry = dynamic_cast<const CSeq_entry*> (it->object.GetPointer());
            if (seqEntry) {
                CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*seqEntry);
                writer.WriteSeqEntryHandle(seh);
                continue;
            }

            const CBioseq* bioseq = dynamic_cast<const CBioseq*> (it->object.GetPointer());
            if (bioseq) {
                CBioseq_Handle bsh = scope->AddBioseq(*bioseq);
                writer.WriteBioseqHandle(bsh);
                continue;
            }

            const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*> (it->object.GetPointer());
            if (seqLoc) {
                CRef<CSeq_annot> annot(new CSeq_annot());

                SAnnotSelector sel(CSeq_annot::C_Data::e_Ftable);
                if (depth > 0) {
                    sel.SetResolveDepth(depth);
                    if (m_Params.GetExactFlevel())
                        sel.SetExactDepth(true);
                }

                CFeat_CI iter(*scope, *seqLoc, sel);
                for (; iter; ++iter) {
                    CRef<CSeq_feat> feat(new CSeq_feat());
                    feat->Assign(iter->GetOriginalFeature());
                    annot->SetData().SetFtable().push_back(feat);
                }

                writer.WriteAnnot(*annot);
                continue;
            }

            const CSeq_id* seqId = dynamic_cast<const CSeq_id*>(it->object.GetPointer());
            if (seqId) {
                CBioseq_Handle bsh = scope->GetBioseqHandle(*seqId);
                writer.WriteBioseqHandle(bsh);
                continue;
            }
        }
        writer.WriteFooter();
	}
    catch (CException& e) {
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CGtfExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
