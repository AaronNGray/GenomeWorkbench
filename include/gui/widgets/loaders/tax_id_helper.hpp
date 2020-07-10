#ifndef GUI_WIDGETS___LOADERS___TAX_ID_HELPER__HPP
#define GUI_WIDGETS___LOADERS___TAX_ID_HELPER__HPP

/*  $Id: tax_id_helper.hpp 37352 2016-12-27 19:46:15Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <util/icanceled.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

class wxControlWithItems;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_LOADERS_EXPORT CTaxIdHelper : public CObjectEx, public CEventHandler
{
    DECLARE_EVENT_MAP();

public:
    class ICallback
    {
    public:
        virtual ~ICallback() {}
        virtual void TaxonsLoaded(bool local) = 0;
    };

    class CAutoDelete {
    public:
        CAutoDelete(ICallback* callback) : m_Callback(callback) {}
        ~CAutoDelete()
        {
            CTaxIdHelper::GetInstance().x_DeleteCallback(m_Callback);
        }
    private:
        ICallback* m_Callback;
    };

    struct STaxonData
    {
        STaxonData() : m_Id(0) {}
        STaxonData(int id, const wxString& label) : m_Id(id), m_Label(label) {}

        int m_Id;
        wxString m_Label;
    };

    typedef vector<STaxonData> TTaxonVector;
    static CTaxIdHelper& GetInstance();
    bool FillControl(ICallback* callback, wxControlWithItems& ctrl, bool local, int selectTaxId);

    void Reset(bool local = true);

private:
    CTaxIdHelper();
    ~CTaxIdHelper();

    void x_DeleteCallback(ICallback* callback);
    void x_AddCallback(ICallback* callback);
    void x_DoCallbacks(bool local);

    void x_CancelJobs();

    static void x_FillControl(wxControlWithItems& ctrl, const TTaxonVector& data, int selectTaxId);

    void x_OnJobNotification(CEvent* evt);
    void x_JobCompleted(CAppJobDispatcher::TJobID jobId, const TTaxonVector& result);
    void x_JobFailed(CAppJobDispatcher::TJobID jobId);
    void x_JobCanceled(CAppJobDispatcher::TJobID jobId);

    bool m_LocalTaxonsLoaded;
    TTaxonVector m_LocalTaxons;
    CAppJobDispatcher::TJobID m_LocalJobId;

    bool m_NetTaxonsLoaded;
    TTaxonVector m_NetTaxons;
    CAppJobDispatcher::TJobID m_NetJobId;

    list<ICallback*> m_Callbacks;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS___LOADERS___TAX_ID_HELPER__HPP
