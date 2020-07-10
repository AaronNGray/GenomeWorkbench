#ifndef GUI_WIDGETS_SEQ___TEXT_RETRIEVE_JOB__HPP
#define GUI_WIDGETS_SEQ___TEXT_RETRIEVE_JOB__HPP

/*  $Id: text_retrieve_job.hpp 35449 2016-05-09 20:51:12Z katargir $
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
* Authors:  Roman Katargin
*
* File Description:
*
*/

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/app_job_impl.hpp>

#include <gui/widgets/seq/text_ds.hpp>

#include <gui/utils/command_processor.hpp>

BEGIN_NCBI_SCOPE

class CCompositeTextItem;

class NCBI_GUIWIDGETS_SEQ_EXPORT CTextRetrieveJob : public CJobCancelable
{
public:
    CTextRetrieveJob(CTextPanelContext& context) : m_Context(&context) {}
    ~CTextRetrieveJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    /// @}

protected:
    virtual string x_GetJobName() const = 0;
    virtual EJobState x_Run() = 0;

    void x_CheckCancelled() const;

    void x_AddItem(ITextItem* item);
    void x_AddItems(vector<ITextItem*>& items);
    void x_CreateResult();

    // for job results/status
    CRef<CTextViewRetrieveResult>  m_Result;
    CRef<CAppJobError>             m_Error;

    CFastMutex m_Mutex;
    auto_ptr<CRootTextItem> m_RootItem;
    auto_ptr<CTextPanelContext> m_Context;

friend class CFlatFileFactoryBuilder;

private:
    class CCancelException
    {
    };
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___TEXT_RETRIEVE_JOB__HPP
