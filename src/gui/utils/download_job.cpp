/*  $Id: download_job.cpp 39418 2017-09-21 20:24:21Z katargir $
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
 */

#include <ncbi_pch.hpp>
#include <math.h>

#include <corelib/ncbifile.hpp>
#include <corelib/ncbi_system.hpp>
#include <connect/ncbi_conn_stream.hpp>
#include <util/compress/stream_util.hpp>
#include <util/compress/tar.hpp>

#include <gui/utils/download_job.hpp>


BEGIN_NCBI_SCOPE

extern "C" {
static EIO_Status s_FtpCallback(CONN conn, TCONN_Callback type, void* data)
{
    if (type != eCONN_OnClose/*  &&  !s_Signaled*/) {
        // Reinstate the callback right away
        SCONN_Callback cb = { s_FtpCallback, data };
        CONN_SetCallback(conn, (ECONN_Callback)type, &cb, 0);
    }

    const CDownloadJob::CDownloadCallbackData* dlcbdata
        = reinterpret_cast<const CDownloadJob::CDownloadCallbackData*>(data);
    size_t pos  = CONN_GetPosition(conn, eIO_Read);
    //size_t size = dlcbdata->GetSize();

    dlcbdata->SetPos(pos);

#if 0
    //double time = dlcbdata->GetElapsed();
    if (!size) {        
        /*
        cerr << "Downloaded " << pos << "/unknown"
            " in " << fixed << setprecision(2) << time
             << left << setw(16) << 's' << '\r' << flush;
        */
    } else {

        /*
        double percent = (pos * 100.0) / size;
        if (percent > 100.00)
            percent = 100.00;
        */
        /*
        cerr << "Downloaded " << pos << '/' << size
             << " (" << fixed << setprecision(2) << percent << "%)"
            " in " << fixed << setprecision(2) << time;
        if (time) {
            double kbps = pos / time / 1024.0;
            cerr << "s (" << fixed << setprecision(2) << kbps
                 << left << setw(16) << "KB/s)" << '\r' << flush;
        
        } else {
            cerr << left << setw(16) << 's'     << '\r' << flush;
        }
        */
    }
#endif 

    if (type == eCONN_OnClose) {
        if (dlcbdata->CB().func)
            dlcbdata->CB().func(conn, type, dlcbdata->CB().data);
        cerr << endl;
    
	} else if (dlcbdata->GetCancel()) {
        return eIO_Interrupt;
    }

    return eIO_Success;
}
}

class CTarCheckpointed : public CTar 
{
public:
    CTarCheckpointed(istream& is, CDownloadJob::CDownloadCallbackData* dlcbdata)
        : CTar(is), m_DlCbData(dlcbdata) { }

protected:
    virtual bool Checkpoint(const CTarEntryInfo& current, bool /**/) 
    {
        m_DlCbData->AddTotalSize(current.GetSize());
        //cerr.flush();
        //cout << current << endl;
        return true;
    }

private:
    CDownloadJob::CDownloadCallbackData* m_DlCbData;
};


static size_t s_GetFtpFilesize(iostream& ios, const char* filename)
{
    size_t retval;
    ios << "SIZE " << filename << endl;
    if (!(ios >> retval)) {
        retval = 0;
    }
    ios.clear();
    return retval;
}

static void s_InitiateFtpFileDownload(iostream& ios, const char* filename)
{
    ios << "RETR " << filename << endl;
}



///////////////////////////////////////////////////////////////////////////////
/// CNetBlastLoadingJob

CDownloadJob::CDownloadJob(const string& desc,                           
                           const string& base_dir,
                           const string& download_url,
                           bool extract_archive)
: m_TaskName(desc),
  m_BaseDir(base_dir),
  m_URL(download_url),
  m_ExtractArchives(extract_archive),
  m_DlcbData(0)
{
}

CDownloadJob::CDownloadJob(const string&          desc,
                           const string&          base_dir,
                           const string&          download_url,
                           const vector<string>&  files_to_load,
                           bool extract_archive)
: m_TaskName(desc),
  m_BaseDir(base_dir),
  m_URL(download_url),
  m_FilesToLoad(files_to_load),
  m_ExtractArchives(extract_archive),
  m_DlcbData(0)
{
}


CDownloadJob::~CDownloadJob()
{
    delete m_DlcbData;
}

string CDownloadJob::GetDescr() const
{
    return m_TaskName;
}

void CDownloadJob::RequestCancel()
{
    CFastMutexGuard lock(m_Mutex);
    if (m_DlcbData) {
        m_DlcbData->Cancel();
    }
}

bool CDownloadJob::IsCanceled() const
{
    CFastMutexGuard lock(m_Mutex);
    if (m_DlcbData) {
        return m_DlcbData->GetCancel() != 0;
    }
    return false;
}

string CDownloadJob::x_FormatSize(size_t bytes)
{
    if (0 == bytes)
        return "0 Bytes";
    static const string sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB" };
    
    unsigned i = (unsigned)floor(log10((double)bytes) / 3);
    
    string result = NStr::DoubleToString(bytes / pow(1000,i),2);
    result += ' ';
    result += sizes[i];
    return result;
}

CConstIRef<IAppJobProgress> CDownloadJob::GetProgress()
{
    CFastMutexGuard lock(m_Mutex);

    float progress = 0.0;
    if (!m_DlcbData) {
        return CConstIRef<IAppJobProgress>(new CAppJobProgress(progress, "unknown"));
    }

    size_t size = m_DlcbData->GetSize();
    size_t pos = m_DlcbData->GetPos();
    if (size) {
        progress = float(pos * 100.0) / (float)size;
       if (progress > 100.00)
           progress = 100.00;
    }

    string status;
    status = m_TaskName + " (" + x_FormatSize(pos) + "/" + x_FormatSize(size) + ")";

    return CConstIRef<IAppJobProgress>(new CAppJobProgress(progress, status));
}

CRef<CObject> CDownloadJob::GetResult()
{
    return m_Result;
}

CConstIRef<IAppJobError>  CDownloadJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}

IAppJob::EJobState CDownloadJob::Run()
{
    // check the base directory 
    //

    try {
        CDirEntry de(m_BaseDir);
        if (!de.Exists()) {
            CDirEntry::CreateAbsolutePath(m_BaseDir);
        }
    } catch (CException& ex) {
        ERR_POST(ex.ReportAll());
        m_Error = new CAppJobError("Target path create error: '" + m_BaseDir + "'");
        return IAppJob::eFailed;
    }

    if (m_FilesToLoad.empty()) {
        if (m_ExtractArchives)
            return x_DownloadAndExtract(m_URL);
        else {
            size_t pos = m_URL.rfind('/');
            if ((string::npos == pos) || (pos == (m_URL.length() - 1))) {
                m_Error = new CAppJobError("Invalid URL specified: '" + m_URL + "'");
                return IAppJob::eFailed;
            }
            string file_name = m_URL.substr(pos + 1);
            return x_Download(m_URL, file_name);
        }
    }
    else {
        ITERATE(vector<string>, it, m_FilesToLoad) {
            string fname = m_URL;
            if ('/' != m_URL[m_URL.length()-1])
                fname.append("/");
            fname.append(*it);
            IAppJob::EJobState js;
            if (m_ExtractArchives)
                js = x_DownloadAndExtract(fname);
            else
                js = x_Download(fname,*it);
            if (js != IAppJob::eCompleted)
                return js;
        }
    }
    return IAppJob::eCompleted;
}

IAppJob::EJobState CDownloadJob::x_Download(const string& file_url, const string& file_name)
{
    SConnNetInfo* net_info = ConnNetInfo_Create(0);

    const char* url = file_url.c_str();

    if (!ConnNetInfo_ParseURL(net_info, url)) {
        string err = "Cannot parse URL \"";
        err.append(m_URL);
        err.append("\"");
        ERR_POST(Fatal << err);
        m_Error = new CAppJobError(err);
        return IAppJob::eFailed;
    }

    if (net_info->scheme != eURL_Ftp) {
        string err = "URL scheme must be FTP";
        err.append(m_URL);
        err.append("\"");
        ERR_POST(Fatal << err);
        m_Error = new CAppJobError(err);
        return IAppJob::eFailed;
    }

    if (!*net_info->user) {
        strcpy(net_info->user, "ftp");
    }

    CConn_FtpStream ftp(net_info->host,
        net_info->user,
        net_info->pass);


    size_t size = s_GetFtpFilesize(ftp, net_info->path);
    if (size) {
        ERR_POST(Info << "Downloading " << net_info->path
            << " from " << net_info->host << ", "
            << size << " byte" << &"s"[size == 1]);
    }
    else {
        ERR_POST(Info << "Downloading " << net_info->path
            << " from " << net_info->host);
    }
    s_InitiateFtpFileDownload(ftp, net_info->path);

    ConnNetInfo_Destroy(net_info);
    // Reset I/O positions
    CONN_GetPosition(ftp.GetCONN(), eIO_Open);

    // Set both read and close callbacks
    CDownloadCallbackData* dblcdata;
    if (m_DlcbData)
        dblcdata = m_DlcbData;
    else
        dblcdata = new CDownloadCallbackData(size);

    {{
            CFastMutexGuard lock(m_Mutex);

            SCONN_Callback cb = { s_FtpCallback, dblcdata };
            CONN_SetCallback(ftp.GetCONN(), eCONN_OnRead, &cb, 0);
            CONN_SetCallback(ftp.GetCONN(), eCONN_OnClose, &cb, dblcdata->CB());
            // Build on-the-fly ungzip stream on top of ftp

            m_DlcbData = dblcdata;
        }}

       
    bool download_ok = false;

    // make temp directory for download
    // 
    static unsigned s_cnt = 0;
    ++s_cnt;
    string filePath(m_BaseDir);
    if (CDirEntry::GetPathSeparator() != m_BaseDir[m_BaseDir.length() - 1])
        filePath += CDirEntry::GetPathSeparator();
    filePath += file_name;

    const size_t buff_size = 1;// 65536;
    char* buffer = new char[buff_size];

    try {
        CNcbiFstream os(filePath.c_str(), IOS_BASE::out | IOS_BASE::binary | IOS_BASE::trunc);
        
        while (!ftp.eof() && !m_DlcbData->GetCancel()) {
            ftp.read(buffer, buff_size);
            if (ftp.gcount())
                os.write(buffer, ftp.gcount());
        }

        os.close();
        ftp.Close();

        download_ok = true;

    }
    catch (CException& ex) {
        if (!m_DlcbData->GetCancel()) {
            m_Error = new CAppJobError(ex.ReportAll());
        }
    }
    catch (std::exception& e) {
        if (!m_DlcbData->GetCancel()) {
            m_Error = new CAppJobError(e.what());
        }
    }
    delete[] buffer;

    if (!download_ok || m_DlcbData->GetCancel()) {
        CDirEntry de(filePath);
        de.Remove();
        if (m_DlcbData->GetCancel()) {
            return IAppJob::eCanceled;
        }
        return IAppJob::eFailed;
    }

    ERR_POST(Info << "Download OK");

    return IAppJob::eCompleted;
}

IAppJob::EJobState CDownloadJob::x_DownloadAndExtract(const string& file_url)
{
    SConnNetInfo* net_info = ConnNetInfo_Create(0);

    const char* url = file_url.c_str();

    if (!ConnNetInfo_ParseURL(net_info, url)) {
        string err = "Cannot parse URL \"";
        err.append(m_URL);
        err.append("\"");
        ERR_POST(Fatal << err);
        m_Error = new CAppJobError(err);
        return IAppJob::eFailed;
    }

    if (net_info->scheme != eURL_Ftp) {
        string err = "URL scheme must be FTP";
        err.append(m_URL);
        err.append("\"");
        ERR_POST(Fatal << err);
        m_Error = new CAppJobError(err);
        return IAppJob::eFailed;
    }

    if (!*net_info->user) {
        strcpy(net_info->user, "ftp");
    }

    CConn_FtpStream ftp(net_info->host,
                        net_info->user,
                        net_info->pass);


    size_t size = s_GetFtpFilesize(ftp, net_info->path);
    if (size) {
        ERR_POST(Info << "Downloading " << net_info->path
                      << " from " << net_info->host << ", "
                      << size << " byte" << &"s"[size == 1]);
    } else {
        ERR_POST(Info << "Downloading " << net_info->path
                      << " from " << net_info->host);
    }
    s_InitiateFtpFileDownload(ftp, net_info->path);

    ConnNetInfo_Destroy(net_info);
    // Reset I/O positions
    CONN_GetPosition(ftp.GetCONN(), eIO_Open);

    // Set both read and close callbacks
    CDownloadCallbackData* dblcdata;
    if (m_DlcbData)
        dblcdata = m_DlcbData;
    else
        dblcdata = new CDownloadCallbackData(size);

    {{
    CFastMutexGuard lock(m_Mutex);
    
    SCONN_Callback cb = { s_FtpCallback, dblcdata };
    CONN_SetCallback(ftp.GetCONN(), eCONN_OnRead,  &cb, 0);
    CONN_SetCallback(ftp.GetCONN(), eCONN_OnClose, &cb, dblcdata->CB());
    // Build on-the-fly ungzip stream on top of ftp

    m_DlcbData = dblcdata;
    }}

    CDecompressIStream is(ftp, CCompressStream::eGZipFile);

    bool download_ok = false;

    // make temp directory for download
    // 
    static unsigned s_cnt = 0;
    ++s_cnt;
    string base_temp_dir = m_BaseDir;
    string temp_dir = string("tmp") + NStr::IntToString(s_cnt);
    base_temp_dir = CDirEntry::ConcatPath(base_temp_dir, temp_dir);
    
    try {
        CDirEntry::CreateAbsolutePath(base_temp_dir);

        // Build streaming [un]tar on top of uncompressed data
        CTarCheckpointed tar(is, m_DlcbData);

        tar.SetBaseDir(base_temp_dir);
        tar.SetFlags(CTar::fOverwrite);

        tar.Extract();

        tar.Close();

        ftp.Close();

        download_ok = true;

    } catch (CException& ex) {
        if (!m_DlcbData->GetCancel()) {
            m_Error = new CAppJobError(ex.ReportAll());
        }
    } catch (std::exception& e) {
        if (!m_DlcbData->GetCancel()) {
            m_Error = new CAppJobError(e.what());
        }
    }

    if (!download_ok || m_DlcbData->GetCancel()) {
        CDirEntry de(base_temp_dir);
        de.Remove();
        if (m_DlcbData->GetCancel()) {
           // return IAppJob::eCompleted;
            return IAppJob::eCanceled;
        }
        return IAppJob::eFailed;
    }

    // download ok, need to move temp directory content one level up
    try {
        {{
        CDir dir(base_temp_dir);
        CDir::TEntries contents = dir.GetEntries("*", CDir::fIgnoreRecursive);
        ITERATE(CDir::TEntries, i, contents) {
            string entry = (*i)->GetPath();
            string new_name = CDirEntry::MakePath(m_BaseDir, (*i)->GetName());
            CDirEntry(entry).Rename(new_name, CDirEntry::fRF_Overwrite); 
        }
        }}
        CDirEntry de(base_temp_dir);
        de.Remove();
    } catch (CException& ex) {
        if (!m_DlcbData->GetCancel()) {
            m_Error = new CAppJobError(ex.ReportAll());
            return IAppJob::eFailed;
        }
        return IAppJob::eCanceled;
    } catch (std::exception& e) {
        if (!m_DlcbData->GetCancel()) {
            m_Error = new CAppJobError(e.what());
            return IAppJob::eFailed;
        }
        return IAppJob::eCanceled;
    }
    ERR_POST(Info << "Download OK");

    return IAppJob::eCompleted;
}


END_NCBI_SCOPE
