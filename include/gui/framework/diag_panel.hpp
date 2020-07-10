#ifndef GUI_FRAMEWORK___DIAG_PANEL_HPP
#define GUI_FRAMEWORK___DIAG_PANEL_HPP

/*  $Id: diag_panel.hpp 41643 2018-09-05 16:47:14Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/objutils/reg_settings.hpp>
#include <gui/framework/view.hpp>
#include <gui/widgets/wx/wm_client.hpp>

#include <gui/utils/extension.hpp>
#include <gui/utils/command.hpp>

#include <wx/imaglist.h>

////@begin includes
#include <wx/toolbar.h>
#include <wx/listctrl.h>
////@end includes

////@begin forward declarations
////@end forward declarations

////@begin control identifiers
#define ID_CWXDIAGPANEL 10030
#define ID_TOOLBAR2 10039
#define ID_TOOL_ERRORS 10040
#define ID_TOOL_WARNINGS 10041
#define ID_TOOL_INFO 10042
#define ID_TOOL_OTHER 10043
#define ID_CHOICE3 10044
#define ID_LISTCTRL1 10037
#define SYMBOL_CWXDIAGPANEL_STYLE 0
#define SYMBOL_CWXDIAGPANEL_TITLE wxT("Diagnostics Console")
#define SYMBOL_CWXDIAGPANEL_IDNAME ID_CWXDIAGPANEL
#define SYMBOL_CWXDIAGPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CWXDIAGPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CDiagConsoleList;

///////////////////////////////////////////////////////////////////////////////
/// CDiagConsolePanel
class NCBI_GUIFRAMEWORK_EXPORT CDiagConsolePanel :
    public wxPanel,
    public IRegSettings
{
    DECLARE_EVENT_TABLE();
public:
    CDiagConsolePanel();
    virtual ~CDiagConsolePanel();

    /// Creation
    bool Create(wxWindow* parent,
                wxWindowID id = SYMBOL_CWXDIAGPANEL_IDNAME,
                const wxPoint& pos = SYMBOL_CWXDIAGPANEL_POSITION,
                const wxSize& size = SYMBOL_CWXDIAGPANEL_SIZE,
                long style = SYMBOL_CWXDIAGPANEL_STYLE );

    void Init();
    void CreateControls();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    void OnToolErrorsClick( wxCommandEvent& event );
    void OnToolWarningsClick( wxCommandEvent& event );
    void OnToolInfoClick( wxCommandEvent& event );
    void OnToolOtherClick( wxCommandEvent& event );
    void OnChoice3Selected( wxCommandEvent& event );

    void OnCopy( wxCommandEvent& event );
    void OnCopyUpdateUI( wxUpdateUIEvent& event );

    int GetLogBufferIndex() const { return m_LogBufferIndex ; }
    void SetLogBufferIndex(int value) { m_LogBufferIndex = value ; }

    wxBitmap GetBitmapResource(const wxString& name);

private:
    /// command callback for listener
    void x_LoadMessages();
    void x_UpdateUI();

protected:
    int m_LogBufferIndex;
    CDiagConsoleList* m_MsgList;
    string m_RegPath;
    wxImageList m_ImageList;
};

///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleView
class NCBI_GUIFRAMEWORK_EXPORT CDiagConsoleView :
    public CObject,
    public IWMClient,
    public IView,
    public IRegSettings
{
    friend class CDiagConsoleViewFactory;

public:
    CDiagConsoleView();
    virtual ~CDiagConsoleView();

    /// @name IView
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow* GetWindow();
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const;
    virtual string GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor( const CRgbaColor& color );
    virtual const wxMenu*  GetMenu();
    virtual void UpdateMenu( wxMenu& root_menu );
    virtual CFingerprint GetFingerprint() const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    static CViewTypeDescriptor m_TypeDescr;

    string m_RegPath;

    CDiagConsolePanel*  m_Panel;
};


///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleViewFactory
class NCBI_GUIFRAMEWORK_EXPORT CDiagConsoleViewFactory :
    public CObject,
    public IExtension,
    public IViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IViewFactory interface implementation
    /// @{
    virtual void    RegisterIconAliases(wxFileArtProvider& provider);
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    /// @}
};


END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___DIAG_PANEL_HPP
