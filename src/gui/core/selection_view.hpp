#ifndef GUI_CORE___SELECTION_VIEW__HPP
#define GUI_CORE___SELECTION_VIEW__HPP

/*  $Id: selection_view.hpp 38835 2017-06-23 18:15:53Z katargir $
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
 * Authors:  Andrey Yazhuk, Yury Voronov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/framework/view.hpp>
#include <gui/core/selection_service.hpp>
#include <gui/utils/event_handler.hpp>

#include <gui/objutils/reg_settings.hpp>
#include <gui/utils/extension.hpp>

BEGIN_NCBI_SCOPE

class CSelectionService;
class CSelectionPanel;

///////////////////////////////////////////////////////////////////////////////
/// CSelectionView
class CSelectionView
:   public CObjectEx,
    public IView,
    public IWMClient,
    public CEventHandler,
    public IRegSettings,
    public ISelectionClient
{
    friend class CSelectionViewFactory;
    DECLARE_EVENT_MAP();
public:
    CSelectionView();
    virtual ~CSelectionView();

    /// @name IView
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void SetWorkbench( IWorkbench* workbench );
    virtual void CreateViewWindow( wxWindow* parent );
    virtual void DestroyViewWindow();
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow* GetWindow();
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const;
    virtual string GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor( const CRgbaColor& aColor );
    virtual const wxMenu* GetMenu();
    virtual void UpdateMenu( wxMenu& aRootMenu );
    virtual CFingerprint  GetFingerprint() const;
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

    virtual void GetSelection( CSelectionEvent& evt ) const;
    virtual void SetSelection( CSelectionEvent& evt );

    virtual void GetMainObject( TConstScopedObjects& objects ) const;
    /// @}

protected:
    void    x_OnSelectionEvent(CEvent*);

    static CViewTypeDescriptor m_TypeDescr;
    CSelectionService* m_Service;

    bool m_Broadcasting; // view is broadcasting selection

    CSelectionPanel* m_Panel;

private:
    CSelectionView(const CSelectionView&);
    CSelectionView& operator=(const CSelectionView&);
};


///////////////////////////////////////////////////////////////////////////////
/// CSelectionViewFactory
class NCBI_GUICORE_EXPORT   CSelectionViewFactory :
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


END_NCBI_SCOPE;

#endif // GUI_CORE___SELECTION_VIEW__HPP
