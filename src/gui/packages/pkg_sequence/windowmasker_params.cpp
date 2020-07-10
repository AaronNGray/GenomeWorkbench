/*  $Id: windowmasker_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Vladimir Tereshkov
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_sequence/windowmasker_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CWindowMaskerParams type definition
 */

/*!
 * Default constructor for CWindowMaskerParams
 */

CWindowMaskerParams::CWindowMaskerParams()
{
    Init();
}

/*!
 * Copy constructor for CWindowMaskerParams
 */

CWindowMaskerParams::CWindowMaskerParams(const CWindowMaskerParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CWindowMaskerParams
 */

CWindowMaskerParams::~CWindowMaskerParams()
{
}

/*!
 * Assignment operator for CWindowMaskerParams
 */

void CWindowMaskerParams::operator=(const CWindowMaskerParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CWindowMaskerParams
 */

bool CWindowMaskerParams::operator==(const CWindowMaskerParams& data) const
{
////@begin CWindowMaskerParams equality operator
     if (!(m_Standalone == data.m_Standalone)) return false;
     if (!(m_OutputType == data.m_OutputType)) return false;
     if (!(m_TaxId == data.m_TaxId)) return false;
////@end CWindowMaskerParams equality operator
    return true;
}

/*!
 * Copy function for CWindowMaskerParams
 */

void CWindowMaskerParams::Copy(const CWindowMaskerParams& data)
{
////@begin CWindowMaskerParams copy function
    m_Standalone = data.m_Standalone;
    m_OutputType = data.m_OutputType;
    m_TaxId = data.m_TaxId;
////@end CWindowMaskerParams copy function

    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CWindowMaskerParams
 */

void CWindowMaskerParams::Init()
{
////@begin CWindowMaskerParams member initialisation
    m_Standalone = true;
    m_OutputType = 0;
    m_TaxId = 0;
////@end CWindowMaskerParams member initialisation
}

/// IRegSettings
void CWindowMaskerParams::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

static const char* kStandalone = "Standalone";
static const char* kOutputType = "OutputType";
static const char* kTaxId = "TaxId";

void CWindowMaskerParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kStandalone, m_Standalone);
        view.Set(kOutputType, m_OutputType);
        view.Set(kTaxId, m_TaxId);
    }
}

void CWindowMaskerParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Standalone = view.GetBool(kStandalone, m_Standalone);
        m_OutputType = view.GetInt(kOutputType, m_OutputType);
        m_TaxId = view.GetInt(kTaxId, m_TaxId);
    }
}

END_NCBI_SCOPE
