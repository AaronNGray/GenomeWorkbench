/*  $Id: cleanup_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_sequence/cleanup_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CCleanupParams type definition
 */

/*!
 * Default constructor for CCleanupParams
 */

CCleanupParams::CCleanupParams()
{
    Init();
}

/*!
 * Copy constructor for CCleanupParams
 */

CCleanupParams::CCleanupParams(const CCleanupParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CCleanupParams
 */

CCleanupParams::~CCleanupParams()
{
}

/*!
 * Assignment operator for CCleanupParams
 */

void CCleanupParams::operator=(const CCleanupParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CCleanupParams
 */

bool CCleanupParams::operator==(const CCleanupParams& data) const
{
////@begin CCleanupParams equality operator
     if (!(m_CleanupMode == data.m_CleanupMode)) return false;
////@end CCleanupParams equality operator
    return true;
}

/*!
 * Copy function for CCleanupParams
 */

void CCleanupParams::Copy(const CCleanupParams& data)
{
////@begin CCleanupParams copy function
    m_CleanupMode = data.m_CleanupMode;
////@end CCleanupParams copy function
    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CCleanupParams
 */

void CCleanupParams::Init()
{
////@begin CCleanupParams member initialisation
    m_CleanupMode = 0;
////@end CCleanupParams member initialisation
}

static const char* kCleanupModeTag = "CleanupMode";

/// IRegSettings
void CCleanupParams::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CCleanupParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kCleanupModeTag, m_CleanupMode);

    }
}

void CCleanupParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_CleanupMode = view.GetInt(kCleanupModeTag, m_CleanupMode);
    }
}

END_NCBI_SCOPE
