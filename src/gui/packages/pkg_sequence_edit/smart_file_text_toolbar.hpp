#ifndef PKG_SMART___SMART_FILE_TEXT_TOOLBAR__HPP
#define PKG_SMART___SMART_FILE_TEXT_TOOLBAR__HPP

/*  $Id: smart_file_text_toolbar.hpp 40861 2018-04-23 14:20:01Z filippov $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/event.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <wx/aui/aui.h>

BEGIN_NCBI_SCOPE

class IProjectView;
class IServiceLocator;

///////////////////////////////////////////////////////////////////////////////
/// class CSmartFileTextToolbar

class CSmartFileTextToolbar : public wxAuiToolBar
{
    DECLARE_EVENT_TABLE()
public:
    CSmartFileTextToolbar(wxWindow* parent,
        IProjectView& view,
        IServiceLocator* service_locator,
        SConstScopedObject validateObj,
        const wxString& fullPathName);
    enum {
        ID_TB_ValidateButton = 20000,
        ID_TB_SaveButton,
        ID_TB_SaveAsButton,
        ID_TB_CloseButton,
        ID_TB_MegaButton
    };
    ~CSmartFileTextToolbar();

    void OnMegaBtn(wxCommandEvent&);
    void OnValidateBtn(wxCommandEvent&);
    void OnSaveBtn(wxCommandEvent&);
    void OnSaveAsBtn(wxCommandEvent&);
    void OnCloseBtn(wxCommandEvent&);

    void OnUpdateValidateBtn(wxUpdateUIEvent& event);
    void OnUpdateSaveBtn(wxUpdateUIEvent& event);
    void OnUpdateSaveAsBtn(wxUpdateUIEvent& event);
    void OnUpdateCloseBtn(wxUpdateUIEvent& event);
    void OnJobNotification(CEvent* evt);

    class CEventProxy : public CObjectEx, public CEventHandler
    {
        DECLARE_EVENT_MAP();
    public:
        CEventProxy(CSmartFileTextToolbar* toolbar) : m_Toolbar(toolbar) {}
        void ResetToolbar() { m_Toolbar = 0; }
    private:
        void OnJobNotification(CEvent* evt);
        CSmartFileTextToolbar* m_Toolbar;
    };


private:
    void ReallyDone();
    void RunValidateView();

    wxString x_AskForFileName();
    void x_Save(const CSerialObject& so, const wxString& fileName);
    string x_GetProject_Path(int projid);

    IProjectView&      m_View;
    IServiceLocator*   m_ServiceLocator;
    SConstScopedObject m_ValidateObj;
    int m_JobId;
    wxButton *m_MegaButton;
    wxButton *m_ValidateButton;
    wxButton *m_SaveButton;
    wxButton *m_SaveAsButton;
    wxButton *m_CloseButton;
    bool m_ValidationInProgress;
    CRef<CEventProxy> m_EventProxy;
    wxString m_FilePath;
    wxString m_FileName;
};

END_NCBI_SCOPE


#endif  // PKG_SMART___SMART_FILE_TEXT_TOOLBAR__HPP

