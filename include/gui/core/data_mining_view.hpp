#ifndef GUI_SERVICES___DATA_MINING_VIEW_HPP
#define GUI_SERVICES___DATA_MINING_VIEW_HPP

/*  $Id: data_mining_view.hpp 40880 2018-04-25 20:26:58Z katargir $
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
 * Authors:  Andrey Yazhuk, Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>

#include <gui/core/data_mining_service.hpp>
#include <gui/core/dm_search_tool.hpp>
#include <gui/core/search_form_base.hpp>
#include <gui/core/visible_range_service.hpp>

#include <gui/widgets/wx/wm_client.hpp>

#include <gui/utils/extension.hpp>

#include <wx/panel.h>
#include <wx/combobox.h>
#include <wx/menu.h>
#include <wx/animate.h>


BEGIN_NCBI_SCOPE

class CDataMiningService;
class CDataMiningView;
class CDataMiningPanel;
class CSelectionService;

/////////////////////////////////////////////////////////////////////////////////
/// CDataMiningView
class NCBI_GUICORE_EXPORT CDataMiningView
:   public CObjectEx,
    public CEventHandler,
    public IView,
    public IWMClient,
    public IRegSettings,
    public ISelectionClient,
    public IVisibleRangeClient
{
    friend class CDataMiningViewFactory;
    DECLARE_EVENT_MAP();
public:
    CDataMiningView();
    virtual ~CDataMiningView();

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
    virtual string GetClientLabel(IWMClient::ELabel ltype) const;
    virtual string GetIconAlias() const;
    virtual const CRgbaColor*   GetColor() const;
    virtual void SetColor( const CRgbaColor& color );
    virtual const wxMenu* GetMenu();
    virtual void UpdateMenu( wxMenu& root_menu );
    virtual CFingerprint GetFingerprint() const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name ISelectionClient interface implementation
    /// @{
    virtual void GetSelection( TConstScopedObjects& objects ) const;
    virtual void SetSelectionService( ISelectionService* service );

    virtual string GetSelClientLabel();

    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void SetSelection(CSelectionEvent& evt);

    virtual void GetMainObject( TConstScopedObjects& objects ) const;

    virtual void BroadcastSelection();
    /// @}

    /// @name IVisibleRangeClient interface implementation
    /// @{
    virtual void OnVisibleRangeChanged(const CVisibleRange&, IVisibleRangeClient*) {}
    ///  @}


    virtual void BroadcastVisibleRange();

    virtual void    SelectToolByName(const string& name);

    /// TODO these mehods must be protected
    void    OnWidgetSelChanged(CEvent* evt);
    void    x_UpdateSelectionScope();
    void    x_UpdateToolCombo(bool update_list);
    void    x_UpdateLocation(TConstScopedObjects & cso);

    void    x_OnStateChanged(CEvent*);
    void    x_OnSelectionEvent(CEvent*);
    
protected:
    static CViewTypeDescriptor      m_TypeDescr;

    IWorkbench* m_Workbench;
    CDataMiningService* m_DataMiningService;  
    CSelectionService*  m_SelectionService;

    CRef<objects::CScope>       m_CurrSelScope;
    objects::CSeq_loc::TRange   m_Range;

    bool    m_CurrSelScopeDirty;

    string m_RegPath; // path to the setting in CGuiRegistry

    CDataMiningPanel*   m_Panel;
};


///////////////////////////////////////////////////////////////////////////////
/// CDataMiningViewFactory
class NCBI_GUICORE_EXPORT   CDataMiningViewFactory :
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
    virtual const   CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    /// @}
};


END_NCBI_SCOPE

#endif  // GUI_SERVICES___DATA_MINING_VIEW_HPP
