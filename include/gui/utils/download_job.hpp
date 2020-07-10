#ifndef GUI_UTILS__DOWNLOAD_JOB__HPP
#define GUI_UTILS__DOWNLOAD_JOB__HPP

/*  $Id: download_job.hpp 37388 2017-01-04 17:23:45Z katargir $
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
 * Authors: Anatoliy Kuznetsov
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbimtx.hpp>
#include <corelib/ncbicntr.hpp>
#include <connect/ncbi_conn_stream.hpp>

#include <gui/gui_export.h>
#include <gui/utils/app_job_impl.hpp>


BEGIN_NCBI_SCOPE

/// Job to download list of files
/// By default it extracts the downloaded  tarball archives
///
class NCBI_GUIUTILS_EXPORT CDownloadJob : public CObject, public IAppJob
{
public:
    class CDownloadCallbackData 
    {
    public:
        CDownloadCallbackData(size_t size) :
            m_Size(size),
            m_TotalSize(0)
        { 
            memset(&m_Cb, 0, sizeof(m_Cb)); 
        }

        SCONN_Callback*       CB(void)       { return &m_Cb;          }
        const SCONN_Callback& CB(void) const { return m_Cb;           }
        size_t           GetSize(void) const { return m_Size;         }
//        double        GetElapsed(void) const { return m_Sw.Elapsed(); }
        void        AddTotalSize(size_t add) { m_TotalSize += add;    }
        size_t      GetTotalSize(void) const { return m_TotalSize;    }
        void Cancel() { m_Cancelled.Add(1); }
        CAtomicCounter::TValue GetCancel() const { return m_Cancelled.Get(); }
        void SetPos(size_t pos) const { m_Pos.Set((int)pos); }
        size_t GetPos() const { return m_Pos.Get(); }

    private:
        SCONN_Callback              m_Cb;
        size_t                      m_Size;
        size_t                      m_TotalSize;
        CAtomicCounter_WithAutoInit m_Cancelled;
        mutable CAtomicCounter      m_Pos;
    };


public:
    CDownloadJob(const string& desc,
                 const string& base_dir,
                 const string& download_url,
                 bool extract_archive = true);

    CDownloadJob(const string&          desc,
                 const string&          base_dir,
                 const string&          download_url,
                 const vector<string>&  files_to_load,
                 bool extract_archive = true);

    ~CDownloadJob();

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    virtual void                        RequestCancel();
    virtual bool                        IsCanceled() const;
    /// @}

private:
    EJobState       x_Download(const string& url, const string& file_name);
    EJobState       x_DownloadAndExtract(const string& url);
    string          x_FormatSize(size_t bytes);

private:
    mutable CFastMutex    m_Mutex;              ///< Mutex for guarding state access
    string                m_TaskName;           ///< Name of the task
    string                m_BaseDir;            ///< Base directory to download and optionally unpack
    string                m_URL;                ///< URL to download
    vector<string>        m_FilesToLoad;        ///< List of files to load
    bool                  m_ExtractArchives;    ///< Flag, indicating whether to extract archives on download

    CRef<CObject>         m_Result;
    CRef<CAppJobError>    m_Error;
    CDownloadCallbackData* m_DlcbData;
};


END_NCBI_SCOPE

#endif // GUI_UTILS__DOWNLOAD_JOB__HPP
