#ifndef GUI_CORE___TABLE_VIEW__HPP
#define GUI_CORE___TABLE_VIEW__HPP

/*  $Id: table_view.hpp 33484 2015-08-03 17:21:34Z falkrb $
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
#include <gui/core/project_view_factory.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/table_data.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_CORE
 *
 * @{
 */


///////////////////////////////////////////////////////////////////////////////
/// CTableView
class NCBI_GUICORE_EXPORT CTableView : public CProjectView, public IRegSettings
{
    friend class CTableViewFactory;
public:
    CTableView();

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*      GetWindow();
    virtual wxEvtHandler* GetCommandHandler();
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

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

protected:
    virtual void x_OnSetSelection(CSelectionEvent& evt);

    virtual CIRef<ITableData> x_CreateTableData(TConstScopedObjects& objects);

    void x_OnUrlHover(CEvent* evt);
    void x_QueryStatusChange(CEvent* evt);
    void x_OnRowDClick(CEvent* evt);

protected:
    TConstScopedObjects m_OrigObjects;

    wxWindow* m_Window;

    DECLARE_EVENT_MAP();
};


///////////////////////////////////////////////////////////////////////////////
/// CTableViewFactory
class CTableViewFactory :
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

///////////////////////////////////////////////////////////////////////////////
/// CProjectTableViewFactory
class CProjectTableViewFactory :
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

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual bool IsCompatibleWith(const CObject& object, objects::CScope& scope);
    virtual int TestInputObjects(TConstScopedObjects& objects);

    virtual bool ShowInOpenViewDlg() const { return false; }
    /// @}
};

/* @} */

END_NCBI_SCOPE

#endif  // GUI_CORE___TABLE_VIEW__HPP
