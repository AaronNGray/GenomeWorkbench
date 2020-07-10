#ifndef GUI_WIDGETS___LOADERS___WINMASK_FILES__HPP
#define GUI_WIDGETS___LOADERS___WINMASK_FILES__HPP

/*  $Id $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>

#include <corelib/ncbiobj.hpp>

#include <gui/utils/event_handler.hpp>
#include <gui/utils/app_job_dispatcher.hpp>

#include <wx/string.h>

class wxWindow;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_LOADERS_EXPORT CWinMaskerFileStorage : public CObjectEx, public CEventHandler
{    
public:
    friend class CWinMaskFilesDlg;

    /// @name Singleton maintenance
    /// @{

    /// access the application-wide singleton
    static CWinMaskerFileStorage& GetInstance();

    /// @}

    void SetRegistryPath( const string& path );
    void SaveSettings() const;
    void LoadSettings();

    void GetTaxIds(vector<int>& ids);

    wxString GetPath() const;

    bool GetUseEnvPath() const { return m_UseEnvPath; }
    void SetUseEnvPath(bool value) { m_UseEnvPath = value; }
    wxString GetEnvPath() const;
    wxString GeFtpUrl() const;

    bool ShowOptionsDlg();

    bool IsDownloading() const { return m_JobId != CAppJobDispatcher::eInvalidJobID; }

private:
    CWinMaskerFileStorage();
    virtual ~CWinMaskerFileStorage();

    wxString x_GetDownloadPath() const;
    bool x_ClearDownloads() const;
    void x_DownloadFileList(vector<string>& files);
    void x_DownloadFiles(const vector<string>& files);

    string m_RegPath;
    bool m_UseEnvPath;

    CAppJobDispatcher::TJobID m_JobId;

    void x_OnJobNotification(CEvent* evt);

    DECLARE_EVENT_MAP();
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___WINMASK_FILES__HPP
