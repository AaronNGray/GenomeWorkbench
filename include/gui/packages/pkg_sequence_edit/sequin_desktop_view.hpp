#ifndef PKG_SEQUENCE_EDIT___SEQUIN_DESKTOP_VIEW__HPP
#define PKG_SEQUENCE_EDIT___SEQUIN_DESKTOP_VIEW__HPP

/*  $Id: sequin_desktop_view.hpp 37865 2017-02-23 21:52:38Z asztalos $
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

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/text_widget/text_widget_host.hpp>

#include <gui/widgets/wx/gui_widget.hpp>

BEGIN_NCBI_SCOPE

class CSeqDesktopPanel;

/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CSequinDesktopView
class CSequinDesktopView : public CProjectView
    , public IGuiWidgetHost
{
    friend class CSequinDesktopViewFactory;
    DECLARE_EVENT_MAP();

public:
    CSequinDesktopView();
    ~CSequinDesktopView();

    /// @name IGuiWidgetHost implementation
    /// @{
    virtual void CloseWidget();
    virtual void WidgetSelectionChanged();
    virtual void WidgetActivated();
    virtual void GetAppSelection(TConstObjects& objects);
    virtual void OnWidgetCommand(const string& command);
    virtual void ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat);
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*      GetWindow();
    virtual wxEvtHandler* GetCommandHandler();
    /// @}

    /// @name IMenuContributor interface implementation
    /// @{
    virtual const wxMenu* GetMenu();
    /// @}


    /// @name IView implementation
    /// @(
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void OnProjectChanged(CProjectViewEvent& evt);
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void GetSelection(TConstScopedObjects& objs) const;
    virtual void GetMainObject(TConstScopedObjects& objs) const;
    /// @}

    /// @addtogroup CProjectViewBase overridables
    /// @{
    virtual void GetCompatibleToolBars(vector<string>& names);
    virtual const CObject* x_GetOrigObject() const;
    /// @}

protected:
    virtual void x_OnSetSelection(CSelectionEvent& evt);

    void x_OnRefreshData(CEvent* event);
    /// Scrolls to the corresponding item in the Flat File View and selects it
    void x_OnSingleMouseClick(CEvent* event);
    
private:

    CSeqDesktopPanel* m_Window;
};


///////////////////////////////////////////////////////////////////////////////
/// CSequinDesktopViewFactory
class CSequinDesktopViewFactory :
    public CObject,
    public IExtension,
    public IProjectViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IProjectViewFactory interface implementation
    /// @{
    virtual IView* CreateInstance() const;
    virtual IView* CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);
    virtual void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual int TestInputObjects( TConstScopedObjects& objects );
    /// @}

protected:
    /// @name COpenViewManager implementation
    /// @{
    virtual string GetTargetType() const
    { return "BioSeq"; }
    virtual bool AllowMerge() const { return false; }
    /// @}
};


/* @} */

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE_EDIT___SEQUIN_DESKTOP_VIEW__HPP
