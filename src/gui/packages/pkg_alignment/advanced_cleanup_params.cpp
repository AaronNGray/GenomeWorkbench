/*  $Id: advanced_cleanup_params.cpp 44059 2019-10-17 17:08:54Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

// Generated by DialogBlocks (unregistered), 24/09/2019 09:38:19

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

////@begin includes
#include "advanced_cleanup_params.hpp"

////@end includes

BEGIN_NCBI_SCOPE

CAdvancedCleanupParams::CAdvancedCleanupParams()
{
    Init();
}

CAdvancedCleanupParams::CAdvancedCleanupParams(const CAdvancedCleanupParams& data)
{
    Init();
    Copy(data);
}

CAdvancedCleanupParams::~CAdvancedCleanupParams()
{
}

void CAdvancedCleanupParams::operator=(const CAdvancedCleanupParams& data)
{
    Copy(data);
}

bool CAdvancedCleanupParams::operator==(const CAdvancedCleanupParams& data) const
{
////@begin CAdvancedCleanupParams equality operator
     if (!(m_InvokeBest == data.m_InvokeBest)) return false;
     if (!(m_Objects == data.m_Objects)) return false;
     if (!(m_QueryType == data.m_QueryType)) return false;
     if (!(m_SplignDirection == data.m_SplignDirection)) return false;
////@end CAdvancedCleanupParams equality operator
    return true;
}

void CAdvancedCleanupParams::Copy(const CAdvancedCleanupParams& data)
{
////@begin CAdvancedCleanupParams copy function
    m_InvokeBest = data.m_InvokeBest;
    m_Objects = data.m_Objects;
    m_QueryType = data.m_QueryType;
    m_SplignDirection = data.m_SplignDirection;
////@end CAdvancedCleanupParams copy function
}

void CAdvancedCleanupParams::Init()
{
////@begin CAdvancedCleanupParams member initialisation
    m_InvokeBest = true;
    m_QueryType = CAdvancedAlignCleanup::eInfer;
    m_SplignDirection = CAdvancedAlignCleanup::eDirSense;
////@end CAdvancedCleanupParams member initialisation
}

void CAdvancedCleanupParams::SetRegistryPath(const string& reg_path)
{
     m_RegPath = reg_path;
}

static const char
*kQueryType = "QueryType",
*kSplignDirection = "SplignDirection",
*kInvokeBest = "InvokeBest";

void CAdvancedCleanupParams::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CRegistryReadView view = CGuiRegistry::GetInstance().GetReadView(m_RegPath);

    switch(view.GetInt(kQueryType, m_QueryType))
    {
    case CAdvancedAlignCleanup::eGenomic :
        m_QueryType = CAdvancedAlignCleanup::eGenomic;
        break;
    case CAdvancedAlignCleanup::eRna :
        m_QueryType = CAdvancedAlignCleanup::eRna;
        break;
    case CAdvancedAlignCleanup::eProtein :
        m_QueryType = CAdvancedAlignCleanup::eProtein;
        break;
    default :
        m_QueryType = CAdvancedAlignCleanup::eInfer;
        break;
    }

    switch(view.GetInt(kSplignDirection, m_SplignDirection))
    {
    case CAdvancedAlignCleanup::eDirSense :
        m_SplignDirection = CAdvancedAlignCleanup::eDirSense;
        break;
    case CAdvancedAlignCleanup::eDirAntisense :
        m_SplignDirection = CAdvancedAlignCleanup::eDirAntisense;
        break;
    default :
        m_SplignDirection = CAdvancedAlignCleanup::eDirBoth;
        break;
    }

    m_InvokeBest = view.GetBool(kInvokeBest, m_InvokeBest);
}

void CAdvancedCleanupParams::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CRegistryWriteView view = CGuiRegistry::GetInstance().GetWriteView(m_RegPath);

    view.Set(kQueryType, m_QueryType);
    view.Set(kSplignDirection, m_SplignDirection);
    view.Set(kInvokeBest, m_InvokeBest);
}

END_NCBI_SCOPE
