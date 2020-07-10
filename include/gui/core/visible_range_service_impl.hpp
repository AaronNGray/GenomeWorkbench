#ifndef GUI_CORE___VISIBLE_RANGE_SERVICE_IMPL__HPP
#define GUI_CORE___VISIBLE_RANGE_SERVICE_IMPL__HPP

/*  $Id: visible_range_service_impl.hpp 27463 2013-02-20 21:15:37Z katargir $
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
#include <gui/gui_export.h>

#include <gui/core/visible_range_service.hpp>

#include <gui/framework/service.hpp>

#include <gui/objutils/reg_settings.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CVisibleRangeService

class NCBI_GUICORE_EXPORT CVisibleRangeService
:   public CObject,
    public IService,
    public IVisibleRangeService,
    public IRegSettings
{
public:
    typedef vector<IVisibleRangeClient*> TClients;

    CVisibleRangeService();
    virtual ~CVisibleRangeService();

    void DetachAllClients();

    /// @name IService interface implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IVisibleRangeService implemenation
    /// @{
    virtual void AttachClient(IVisibleRangeClient* client);
    virtual void DetachClient(IVisibleRangeClient* client);

    virtual void BroadcastVisibleRange(const CVisibleRange& vrange,
                                       IVisibleRangeClient* source);
    /// @}

    /// @name IRegSettings implementation
    /// @{
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

private:
    TClients m_Clients;
    string   m_RegPath;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___VISIBLE_RANGE_SERVICE_IMPL__HPP

