#ifndef GUI_WIDGETS_EDIT___TABLE_DATA_SUC_JOB__HPP
#define GUI_WIDGETS_EDIT___TABLE_DATA_SUC_JOB__HPP

/*  $Id: table_data_suc_job.hpp 32231 2015-01-22 15:57:41Z katargir $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/objects.hpp>

#include <gui/utils/app_job_impl.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CTableDataSUCJob : public CJobCancelable
{
public:
    // objects[i].object is supposed to contain one of: Seq-entry, Seq-submit  
	// which will be Sort-Unique-Counted, anything else will be ignored
    CTableDataSUCJob(const TConstScopedObjects& objects);
    virtual ~CTableDataSUCJob() {}

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    // CObject is CSUCResults>
	virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

private:
	TConstScopedObjects m_InputObjects;
    CRef<CObject>       m_Result;
    CRef<CAppJobError>  m_Error;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___TABLE_DATA_SUC_JOB__HPP
