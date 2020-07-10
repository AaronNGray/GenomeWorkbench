/*  $Id: table_data_validate_job.cpp 44447 2019-12-19 16:31:18Z filippov $
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
 * Authors: Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/table_data_validate_job.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>

#include <objects/valerr/ValidError.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seq/Pubdesc.hpp>

#include <objtools/validator/validator.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableDataValidateJob::CTableDataValidateJob(const TConstScopedObjects& objects, const CUser_object* params)
    : m_InputObjects(objects)
{
    m_ResultErrs.Reset(new CObjectFor<CValidError::TErrs>());
    if (params) {
        m_Params.Reset(new CUser_object());
        m_Params->Assign(*params);
    } else {
        m_Params.Reset(NULL);
    }
}

CConstIRef<IAppJobProgress> CTableDataValidateJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>();
}

CRef<CObject> CTableDataValidateJob::GetResult()
{
    return m_ResultErrs;
}


CConstIRef<IAppJobError> CTableDataValidateJob::GetError()
{
   return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CTableDataValidateJob::GetDescr() const
{
    return "Validator job";
}


IAppJob::EJobState CTableDataValidateJob::Run()
{
    string jobName = "Validating";
    string err_msg, logMsg = jobName + " - exception in Run() ";

    try {
        CLockerGuard guard = x_GetGuard();
        if (IsCanceled()) return eCanceled;
        x_FetchAll();
    }
    catch (CException& e) {
        err_msg = e.GetMsg();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
        //e.ReportAll();
    } catch (std::exception& e) {
        err_msg = GetDescr() + ". " + e.what();
        LOG_POST(Error << logMsg << GetDescr() << ". " << err_msg);
    }

    if (IsCanceled())
        return eCanceled;

    if (!err_msg.empty()) {
        string s = err_msg;
        m_Error.Reset(new CAppJobError(s));
        return eFailed;
    }

    return eCompleted;
}


void CTableDataValidateJob::x_FetchAll()
{
    CValidError::TErrs& ValidErrs(dynamic_cast<CObjectFor<CValidError::TErrs>*>(m_ResultErrs.GetPointer())->GetData());
    CRef<CObjectManager> om(CObjectManager::GetInstance());
    validator::CValidator Validator(*om);
    Uint4 validator_opts =
        validator::CValidator::eVal_do_tax_lookup |
        validator::CValidator::eVal_remote_fetch |
        validator::CValidator::eVal_far_fetch_cds_products |
        validator::CValidator::eVal_far_fetch_mrna_products |
        validator::CValidator::eVal_val_exons |
        validator::CValidator::eVal_need_isojta |
        validator::CValidator::eVal_use_entrez;

    if (RunningInsideNCBI()) {
        validator_opts |= validator::CValidator::eVal_indexer_version;
    }

    if (!m_Params || CTableDataValidateParams::GetDoAlignment(*m_Params)) {
        // validate alignments by default (no parameter) or if requested in parameter
        validator_opts |= validator::CValidator::eVal_val_align;
    }
    if (m_Params && CTableDataValidateParams::GetDoInference(*m_Params)) {
        // only validate inferences by request
        validator_opts |= validator::CValidator::eVal_inference_accns;
    }

    TConstScopedObjects::size_type nInputObjects(m_InputObjects.size());
    for(TConstScopedObjects::size_type iInputObjects = 0; 
        iInputObjects < nInputObjects; 
        ++iInputObjects) {
        const type_info& ObjType(typeid(*(m_InputObjects[iInputObjects].object.GetPointer())));
        CConstRef<CValidError> ValidError;
        if(ObjType == typeid(CSeq_entry)) {
            const CSeq_entry* pSeq_entry(dynamic_cast<const CSeq_entry*>(m_InputObjects[iInputObjects].object.GetPointer()));
            ValidError = Validator.Validate(*pSeq_entry,
                                             m_InputObjects[iInputObjects].scope.GetPointer(),
                                             validator_opts);
        }
        if (ObjType == typeid(CBioseq_set)) {
            const CBioseq_set* pBioseq_set(dynamic_cast<const CBioseq_set*>(m_InputObjects[iInputObjects].object.GetPointer()));
            CBioseq_set_Handle bssh = m_InputObjects[iInputObjects].scope->GetBioseq_setHandle(*pBioseq_set);
            CSeq_entry_Handle seh = bssh.GetParentEntry();
            ValidError = Validator.Validate(seh, validator_opts);
        }
        if(ObjType == typeid(CSeq_submit)) {
            ValidError = Validator.Validate(*dynamic_cast<const CSeq_submit*>(m_InputObjects[iInputObjects].object.GetPointer()),
                                             m_InputObjects[iInputObjects].scope.GetPointer(),
                                             validator_opts);
        }
        if(ObjType == typeid(CSeq_annot)) {
            const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(m_InputObjects[iInputObjects].object.GetPointer());
            CSeq_annot_Handle ah = m_InputObjects[iInputObjects].scope->GetSeq_annotHandle(*annot);
            ValidError = Validator.Validate(ah, validator_opts);
        }
        if(ObjType == typeid(CSeq_feat)) {
            ValidError = Validator.Validate(*dynamic_cast<const CSeq_feat*>(m_InputObjects[iInputObjects].object.GetPointer()), 
                                             m_InputObjects[iInputObjects].scope.GetPointer(),
                                             validator_opts);
        }
        if(ObjType == typeid(CBioSource)) {
            ValidError = Validator.Validate(*dynamic_cast<const CBioSource*>(m_InputObjects[iInputObjects].object.GetPointer()),
                                             m_InputObjects[iInputObjects].scope.GetPointer(),
                                             validator_opts);
        }
        if(ObjType == typeid(CPubdesc)) {
            ValidError = Validator.Validate(*dynamic_cast<const CPubdesc*>(m_InputObjects[iInputObjects].object.GetPointer()),
                                             m_InputObjects[iInputObjects].scope.GetPointer(),
                                             validator_opts);
        }
        if (ObjType == typeid(CSeq_id)) {
            const CSeq_id* seq_id(dynamic_cast<const CSeq_id*>(m_InputObjects[iInputObjects].object.GetPointer()));
            CBioseq_Handle bsh = m_InputObjects[iInputObjects].scope->GetBioseqHandle(*seq_id);
            if (bsh) {
                CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
                ValidError = Validator.Validate(*seh.GetCompleteSeq_entry(),
                                                m_InputObjects[iInputObjects].scope.GetPointer(),
                                                validator_opts);
            }
        }
        if(ValidError && ValidError->TotalSize()) {
            const CValidError::TErrs& errs(ValidError->GetErrs());
            ValidErrs.insert(ValidErrs.end(), errs.begin(), errs.end());
            //!! remove me!
#ifdef _DEBUG
            CNcbiOstrstream ostr;
            {
                auto_ptr<CObjectOStream>
                    out(CObjectOStream::Open(eSerial_AsnText, ostr));
                ITERATE(CValidError::TErrs, iValidErrs, ValidErrs) {
                    try {
                        *out << *(*iValidErrs);
                        *out << (*iValidErrs)->GetObject();
                    } catch (const CException&) {
                         ostr << "Unable to write error" << endl;
                    }
                }
            }
            _TRACE(string(CNcbiOstrstreamToString(ostr)));
            //LOG_POST(Info << string(CNcbiOstrstreamToString(ostr)));
#endif
        }
    }
}

END_NCBI_SCOPE

