/*  $Id: bed_load_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/widgets/loaders/bed_load_params.hpp>

BEGIN_NCBI_SCOPE


/*!
 * CBedLoadParams type definition
 */

/*!
 * Default constructor for CBedLoadParams
 */

CBedLoadParams::CBedLoadParams()
{
    Init();
}

/*!
 * Copy constructor for CBedLoadParams
 */

CBedLoadParams::CBedLoadParams(const CBedLoadParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CBedLoadParams
 */

CBedLoadParams::~CBedLoadParams()
{
}

/*!
 * Assignment operator for CBedLoadParams
 */

void CBedLoadParams::operator=(const CBedLoadParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CBedLoadParams
 */

bool CBedLoadParams::operator==(const CBedLoadParams& data) const
{
////@begin CBedLoadParams equality operator
     if (!(m_NumErrors == data.m_NumErrors)) return false;
     if (!(m_MapAssembly == data.m_MapAssembly)) return false;
////@end CBedLoadParams equality operator
    return true;
}

/*!
 * Copy function for CBedLoadParams
 */

void CBedLoadParams::Copy(const CBedLoadParams& data)
{
////@begin CBedLoadParams copy function
    m_NumErrors = data.m_NumErrors;
    m_MapAssembly = data.m_MapAssembly;
////@end CBedLoadParams copy function
}

/*!
 * Member initialisation for CBedLoadParams
 */

void CBedLoadParams::Init()
{
////@begin CBedLoadParams member initialisation
    m_NumErrors = 10;
////@end CBedLoadParams member initialisation
}

static const char* kNumErrors = "NumErrors";
static const char* kMapAssembly = ".MapAssembly";

/// IRegSettings
void CBedLoadParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kNumErrors, m_NumErrors);

        view = gui_reg.GetWriteView(m_RegPath + kMapAssembly);
        m_MapAssembly.SaveSettings(view);
    }
}

void CBedLoadParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_NumErrors = view.GetInt(kNumErrors, m_NumErrors);

        view = gui_reg.GetReadView(m_RegPath + kMapAssembly);
        m_MapAssembly.LoadSettings(view);
    }
}

END_NCBI_SCOPE
