/*  $Id: visible_range_service.cpp 40888 2018-04-26 14:55:12Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/core/visible_range_service_impl.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/visible_range.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CVisibleRangeService::CVisibleRangeService()
{
}

CVisibleRangeService::~CVisibleRangeService()
{
}

void CVisibleRangeService::DetachAllClients()
{
    m_Clients.clear();
}

void CVisibleRangeService::InitService()
{
    LOG_POST(Info << "Initializing Visible Range Service...");
    LoadSettings();
    LOG_POST(Info << "Finished initializing Visible Range Service");
}

void CVisibleRangeService::ShutDownService()
{
    LOG_POST(Info << "Shutting down Visible Range Service...");
    DetachAllClients();
    SaveSettings();
    LOG_POST(Info << "Finished shutting down Visible Range Service");
}

void CVisibleRangeService::AttachClient(IVisibleRangeClient* client)
{
    _ASSERT(client);

    TClients::const_iterator it = std::find(m_Clients.begin(), m_Clients.end(), client);
    if(it == m_Clients.end())   {
        m_Clients.push_back(client);
    } else {
        ERR_POST("CVisibleRangeService::AttachClient() - client already registered");
    }
}

void CVisibleRangeService::DetachClient(IVisibleRangeClient* client)
{
    TClients::iterator it = std::find(m_Clients.begin(), m_Clients.end(), client);
    if(it != m_Clients.end()) {
        m_Clients.erase(it);
    }
    else {
        ERR_POST("CVisibleRangeService::DetachClient() - client is not attached");
    }
}

void CVisibleRangeService::BroadcastVisibleRange(const CVisibleRange& vrange,
                                                 IVisibleRangeClient* source)
{
    static bool fReentry = false;
    if (fReentry)
        return;
    CBoolGuard _guard(fReentry);

    _ASSERT(std::find(m_Clients.begin(), m_Clients.end(), source) != m_Clients.end());

    ITERATE(TClients, it, m_Clients) {
        if (*it == source)
            continue;
        (**it).OnVisibleRangeChanged (vrange, source);
    }
}

void CVisibleRangeService::SetRegistryPath(const string& path)
{
    m_RegPath = path + ".VisibleRange";
}

static const char* kAutoBroadcast = "AutoBroadcast";
static const char* kPolicy = "Policy";

void CVisibleRangeService::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        CVisibleRange::SetAutoBroadcast(view.GetBool(kAutoBroadcast, CVisibleRange::IsAutoBroadcast()));
        int policy = view.GetInt(kPolicy, CVisibleRange::GetDefaultVisibleRangePolicy());
        if (policy >= CVisibleRange::eBasic_Track && policy <= CVisibleRange::eBasic_Ignore)
            CVisibleRange::SetDefaultVisibleRangePolicy((CVisibleRange::EBasicPolicy)policy);
    }
}

void CVisibleRangeService::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kAutoBroadcast, CVisibleRange::IsAutoBroadcast());
        view.Set(kPolicy, CVisibleRange::GetDefaultVisibleRangePolicy());
    }
}

END_NCBI_SCOPE
