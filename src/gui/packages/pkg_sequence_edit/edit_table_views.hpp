#ifndef PKG_SEQUENCE___TABLE_VIEWS__HPP
#define PKG_SEQUENCE___TABLE_VIEWS__HPP

/*  $Id: edit_table_views.hpp 32411 2015-02-24 14:07:57Z asztalos $
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

#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/grid_widget/grid_event.hpp>

#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE

class CTableViewWithTextViewNav : public CProjectView, public IRegSettings
{
public:
    CTableViewWithTextViewNav() : m_Window() {}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow* GetWindow() { return m_Window; }
    virtual wxEvtHandler* GetCommandHandler() { return m_Window->GetEventHandler(); }
    /// @}

    /// @name IView implementation
    /// @(
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

    void SetParams(const objects::CUser_object* params);

    static void CombineObjects(const TConstScopedObjects& objects, TObjectsMap& objects_map);

protected:
    CIRef<IFlatFileCtrl> x_GetFlatFileCtrl(TConstScopedObjects& objects);
    CIRef<IProjectView> x_CreateDefaultTextView(CIRef<CProjectService> prjSrv, TConstScopedObjects& objects);
    ICommandProccessor* x_GetCmdProcessor(TConstScopedObjects& objects);
    void x_EditObject(SConstScopedObject& obj);

    virtual void x_LoadData() = 0;
    
    void x_GetViewObjects(void);
    void x_OnRefreshData(CEvent* evt);
    void x_QueryStatusChange(CEvent* evt);

    DECLARE_EVENT_MAP();

    CRef<objects::CUser_object> m_Params;
    TConstScopedObjects m_OrigObjects;
    wxWindow* m_Window;
};

END_NCBI_SCOPE

#endif  // PKG_SEQUENCE___TABLE_VIEWS__HPP
