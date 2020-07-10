#ifndef GUI_WIDGETS_EDIT___TABLE_DATA_VALIDATE_JOB__HPP
#define GUI_WIDGETS_EDIT___TABLE_DATA_VALIDATE_JOB__HPP

/*  $Id: table_data_validate_job.hpp 32221 2015-01-21 19:14:21Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/objects.hpp>

#include <gui/utils/app_job_impl.hpp>

#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CTableDataValidateJob : public CJobCancelable
{
public:
    // objects[i].object is supposed to contain one of: Seq-entry, Seq-submit, Seq-feat, CSeq_annot_Handle, CBioSource, or CPubdesc 
    // which will be validated, anything else will be ignored
    CTableDataValidateJob(const TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual ~CTableDataValidateJob() {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    // CObject is CObjectFor<CValidError::TErrs>
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

private:
    void x_FetchAll();

private:
    TConstScopedObjects m_InputObjects;
    // stores CObjectFor<CValidError::TErrs>
    CRef<CObject> m_ResultErrs;

    // parameters
    CRef<objects::CUser_object> m_Params;

    CRef<CAppJobError>          m_Error;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___TABLE_DATA_VALIDATE_JOB__HPP
