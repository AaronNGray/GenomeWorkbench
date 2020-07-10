/*  $Id: suc_refresh_cntrl.hpp 40191 2018-01-04 21:03:28Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */
#ifndef _SUC_REFRESH_CNTRL_H_
#define _SUC_REFRESH_CNTRL_H_

#include <corelib/ncbistd.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/widgets/edit/generic_panel.hpp>  
#include <gui/widgets/edit/suc_data.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/utils/app_job_dispatcher.hpp>



BEGIN_NCBI_SCOPE

class CSUCRefreshCntrl : public IRefreshCntrl
{
public:
    CSUCRefreshCntrl(IWorkbench* wb);
    CSUCRefreshCntrl();
    virtual ~CSUCRefreshCntrl();

    virtual void RefreshText(CGenericPanel* text);
    virtual string GetRefreshedText();

    void OnError(const string error);
    void OnSUCData(CSUCResults& sucData);

    class CSUCRefreshCntrlHelper : public CObjectEx, public CEventHandler
    {
    public:
        CSUCRefreshCntrlHelper(CSUCRefreshCntrl* ctrl)
            : m_Ctrl(ctrl), m_JobId(CAppJobDispatcher::eInvalidJobID)
        {
            NEditingStats::ReportUsage(_("Sort Unique Count"));
        }
        virtual ~CSUCRefreshCntrlHelper() {
            if (m_JobId != CAppJobDispatcher::eInvalidJobID)
                CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
        }

        void LoadData(IWorkbench* wb);

    private:
        void x_OnJobNotification(CEvent* evt);
        DECLARE_EVENT_MAP();
        CSUCRefreshCntrl* m_Ctrl;
        int m_JobId;
    };

    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;
protected:
    IWorkbench*     m_Workbench;
    CGenericPanel* m_Text;
    CRef<CSUCRefreshCntrlHelper> m_Helper;
    string m_RegPath;
};

END_NCBI_SCOPE

#endif
    // _SUC_REFRESH_CNTRL_H_


