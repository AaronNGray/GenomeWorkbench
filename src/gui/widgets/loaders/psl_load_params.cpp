/*  $Id: psl_load_params.cpp 44439 2019-12-18 19:51:52Z katargir $
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

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/loaders/psl_load_params.hpp>

BEGIN_NCBI_SCOPE


/*!
 * CPslLoadParams type definition
 */

/*!
 * Default constructor for CPslLoadParams
 */

CPslLoadParams::CPslLoadParams()
{
    Init();
}

/*!
 * Copy constructor for CPslLoadParams
 */

CPslLoadParams::CPslLoadParams(const CPslLoadParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CPslLoadParams
 */

CPslLoadParams::~CPslLoadParams()
{
}

/*!
 * Assignment operator for CPslLoadParams
 */

void CPslLoadParams::operator=(const CPslLoadParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CPslLoadParams
 */

bool CPslLoadParams::operator==(const CPslLoadParams& data) const
{
////@begin CPslLoadParams equality operator
     if (!(m_MapAssembly == data.m_MapAssembly)) return false;
////@end CPslLoadParams equality operator
    return true;
}

/*!
 * Copy function for CPslLoadParams
 */

void CPslLoadParams::Copy(const CPslLoadParams& data)
{
////@begin CPslLoadParams copy function
    m_MapAssembly = data.m_MapAssembly;
////@end CPslLoadParams copy function
}

/*!
 * Member initialisation for CPslLoadParams
 */

void CPslLoadParams::Init()
{
////@begin CPslLoadParams member initialisation
////@end CPslLoadParams member initialisation
}

static const char* kMapAssembly = ".MapAssembly";

/// IRegSettings
void CPslLoadParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view = gui_reg.GetWriteView(m_RegPath + kMapAssembly);
        m_MapAssembly.SaveSettings(view);
    }
}

void CPslLoadParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        view = gui_reg.GetReadView(m_RegPath + kMapAssembly);
        m_MapAssembly.LoadSettings(view);
    }
}


END_NCBI_SCOPE
