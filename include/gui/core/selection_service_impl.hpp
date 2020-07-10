#ifndef GUI_CORE___SELECTION_SERVICE_IMPL__HPP
#define GUI_CORE___SELECTION_SERVICE_IMPL__HPP

/*  $Id: selection_service_impl.hpp 38846 2017-06-26 15:23:36Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/core/selection_service.hpp>
#include <gui/core/document.hpp>

#include <gui/framework/service.hpp>

#include <gui/objutils/reg_settings.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

BEGIN_NCBI_SCOPE

class CSelectionServiceEvent : public CEvent
{
public:
    enum EType {
        eClientAttached,
        eClientDetached,
        eActiveChanged,
        eSelectionChanged
    };

    CSelectionServiceEvent(EType type, ISelectionClient* client) : CEvent(type), m_Client(client) {}

    CIRef<ISelectionClient> GetClient() { return CIRef<ISelectionClient>(m_Client.Lock()); }

private:
    CWeakIRef<ISelectionClient> m_Client;
};

class CSelectionBroadcastSlot;

///////////////////////////////////////////////////////////////////////////////
/// CSelectionService
class NCBI_GUICORE_EXPORT CSelectionService
:   public CObjectEx,
    public IService,
    public IServiceLocatorConsumer,
    public ISelectionService,
    public CEventHandler,
    public IRegSettings
{

public:
    typedef vector<ISelectionClient*> TClients;

    CSelectionService();
    virtual ~CSelectionService();

    /// @name IService interface implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    void InitStatusBar(wxWindow* parent);

    void GetClients(TClients& clients);

    /// @name ISelectionService implemenation
    /// @{
    virtual void AttachClient(ISelectionClient* client);
    virtual void DetachClient(ISelectionClient* client);
    virtual bool IsAttached(ISelectionClient* client) const;

    virtual void GetCurrentSelection(TConstScopedObjects& sel);
    virtual void GetActiveObjects(TConstScopedObjects& sel);
    virtual void GetActiveObjects(vector<TConstScopedObjects>& sel); // Prioritized active objects

    virtual void Broadcast( CSelectionEvent& evt, ISelectionClient* source );

    virtual void OnActiveClientChanged(ISelectionClient* active);
    virtual void OnSelectionChanged(ISelectionClient* source);
    /// @}

    /// @name IRegSettings implementation
    /// @{
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name IServiceLocatorConsumer interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}

    bool IsAutoBroadcast() { return mf_AutoBroadcast; }
    void SetAutoBroadcast( bool pAuto );

protected:
    void x_Broadcast(
        CSelectionEvent& evt, ISelectionClient* source, CGBDocument::TViews& views
    );

protected:
    TClients m_Clients;
    ISelectionClient* m_ActiveClient;

    IServiceLocator* m_SrvLocator;
    string  m_RegPath;

private:
    /// @name various selection service options
    /// @{
    bool mf_AutoBroadcast;
    /// @}

    CSelectionBroadcastSlot* m_StatusBarSlot;
};


END_NCBI_SCOPE

#endif  // GUI_CORE___SELECTION_SERVICE_HPP
