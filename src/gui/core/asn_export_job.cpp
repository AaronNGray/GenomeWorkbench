/*  $Id: asn_export_job.cpp 39341 2017-09-13 19:09:28Z katargir $
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

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/core/asn_export_job.hpp>

#include <objects/seqloc/Seq_loc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CAsnExportJob::CAsnExportJob(const CAsnExportParams& params)
: CAppJob("ASN Export"), m_Params(params)
{
}

IAppJob::EJobState CAsnExportJob::Run()
{
    string err_msg;

    wxString fileName = m_Params.GetFileName();

    try {
        ios::openmode mode = ios::out;
        if (m_Params.GetAsnType())
            mode |= ios::binary;

        CNcbiOfstream os(fileName.fn_str(), mode);

        if (m_Params.GetAsnType())
            os << MSerial_AsnBinary;
        else
            os << MSerial_AsnText;

        for (auto& o : m_Params.SetObjects()) {
            const CSerialObject* so = dynamic_cast<const CSerialObject*>(o.object.GetPointer());
            CScope* scope = o.scope;

            const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(so);
            const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(so);
            CConstRef<CSeq_entry> seq_entry;

            if (seq_id || seq_loc) {
                CBioseq_Handle handle;
                if (seq_id)
                    handle = scope->GetBioseqHandle(*seq_id);
                else if (seq_loc)
                    handle = scope->GetBioseqHandle(*seq_loc);

                if (handle) {
                    CSeq_entry_Handle seh = handle.GetTopLevelEntry();
                    if (seh) {
                        seq_entry = seh.GetSeq_entryCore();
                    }
                }
            }

            if (seq_entry)
                so = seq_entry.GetPointer();

            if (so) {
                os << *so;
            }
        }
    }
    catch (CException& e) {
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CAsnExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
