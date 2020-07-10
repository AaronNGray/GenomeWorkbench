/*  $Id: flatfile_export_job.cpp 39403 2017-09-20 21:16:33Z katargir $
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

#include <objects/seqloc/Seq_loc.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>
#include <objtools/format/flat_expt.hpp>

#include <gui/packages/pkg_sequence/flatfile_export_job.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CFlatFileExportJob::CFlatFileExportJob(const CFlatFileExportParams& params)
: CAppJob("Flat File Export"), m_Params(params)
{
}

IAppJob::EJobState CFlatFileExportJob::Run()
{
    SConstScopedObject seqLoc = m_Params.GetSeqLoc();
    const CSeq_loc* loc =
        dynamic_cast<const CSeq_loc*> (seqLoc.object.GetPointer());
    CScope& scope = seqLoc.scope.GetObject();

    wxString fname = m_Params.GetFileName();

    string err_msg;

    CBioseq_Handle bsh = scope.GetBioseqHandle(*loc);
    if (!bsh) {
        err_msg = "Can't load location: ";
        CLabel::GetLabel(*loc, &err_msg, CLabel::eDefault, &scope);
        LOG_POST(Error << err_msg);
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }

    try {
        CFlatFileConfig ff_config;
        ff_config.SetCanceledCallback(x_GetICanceled());

        switch(m_Params.GetMode()) {
        case 1:
            ff_config.SetMode(CFlatFileConfig::eMode_Entrez);
            break;
        case 2:
            ff_config.SetMode(CFlatFileConfig::eMode_Release);
            break;
        case 3:
            ff_config.SetMode(CFlatFileConfig::eMode_Dump);
            break;
        default:
            ff_config.SetMode(CFlatFileConfig::eMode_GBench);
            break;
        }

        ff_config
            .SetShowContigFeatures()
            .SetShowContigSources();

        CFlatFileGenerator ff(ff_config);
        ff.SetAnnotSelector() = CSeqUtils::GetAnnotSelector();

        CNcbiOfstream ostr(fname.fn_str());
        ff.Generate(*loc, scope, ostr);
    }
    catch (const CFlatException& e){
        if (e.GetErrCode() == CFlatException::eHaltRequested)
            return eCanceled;
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }
    catch (const CException& e) {
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CFlatFileExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
