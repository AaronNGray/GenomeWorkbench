#ifndef GUI_WIDGETS_SEQ___TEXT_VIEW__HPP
#define GUI_WIDGETS_SEQ___TEXT_VIEW__HPP

/*  $Id: text_view.hpp 38170 2017-04-05 18:39:48Z katargir $
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

class CTextPanel;

/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CTextView
class CTextView : public CProjectView
    , public IFlatFileCtrl
    , public IGuiWidgetHost
    , public ITextWidgetHost
{
    friend class CTextViewFactory;
public:
    CTextView();
    ~CTextView();

    /// @name IGuiWidgetHost implementation
    /// @{
    virtual void CloseWidget();
    virtual void WidgetSelectionChanged();
    virtual void WidgetActivated();
    virtual void GetAppSelection(TConstObjects& objects);
    virtual void OnWidgetCommand(const string& command);
    virtual void ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat);
    /// @}

    /// @name ITextWidgetHost implementation
    /// @{
    virtual void OnTextGotFocus();
    virtual void OnTextLostFocus();
    virtual void OnTextPositionChanged(int row, int col);
    /// @}


    /// @name IFlatFileCtrl interface implementation
    /// @{
    virtual bool SetPosition(const string& seq, const CObject* object);
    virtual bool SetPosition(const objects::CBioseq_Handle& h, const CObject* object);
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
    virtual void GetActiveObjects( TConstScopedObjects& objects ) const;
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects);
    /// @}

    /// @addtogroup CProjectViewBase overridables
    /// @{
    virtual void GetCompatibleToolBars(vector<string>& names);
    virtual const CObject* x_GetOrigObject() const;
    /// @}

protected:
    virtual void x_OnSetSelection(CSelectionEvent& evt);

private:
    CTextPanel* m_Window;

    bool m_UsingStatusBar;
};


///////////////////////////////////////////////////////////////////////////////
/// CTextViewFactory
class CTextViewFactory :
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
    { return "SerialObject"; }
    virtual bool AllowMerge() const { return false; }
    /// @}
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___TEXT_VIEW__HPP
