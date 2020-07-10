/*  $Id: debug_macro_tool_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence_edit/debug_macro_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CDebugMacroToolParams type definition
 */

/*!
 * Default constructor for CDebugMacroToolParams
 */

CDebugMacroToolParams::CDebugMacroToolParams()
{
    Init();
}

/*!
 * Copy constructor for CDebugMacroToolParams
 */

CDebugMacroToolParams::CDebugMacroToolParams(const CDebugMacroToolParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CDebugMacroToolParams
 */

CDebugMacroToolParams::~CDebugMacroToolParams()
{
}

/*!
 * Assignment operator for CDebugMacroToolParams
 */

void CDebugMacroToolParams::operator=(const CDebugMacroToolParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CDebugMacroToolParams
 */

bool CDebugMacroToolParams::operator==(const CDebugMacroToolParams& data) const
{
////@begin CDebugMacroToolParams equality operator
     if (!(m_Text == data.m_Text)) return false;
////@end CDebugMacroToolParams equality operator
    return true;
}

/*!
 * Copy function for CDebugMacroToolParams
 */

void CDebugMacroToolParams::Copy(const CDebugMacroToolParams& data)
{
////@begin CDebugMacroToolParams copy function
    m_Text = data.m_Text;
////@end CDebugMacroToolParams copy function
    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CDebugMacroToolParams
 */

void CDebugMacroToolParams::Init()
{
////@begin CDebugMacroToolParams member initialisation
////@end CDebugMacroToolParams member initialisation
}

static const char
*kText = "kText";

void CDebugMacroToolParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kText, ToStdString(m_Text));
    }
}

void CDebugMacroToolParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Text = ToWxString(view.GetString(kText, ToStdString(m_Text)));
    }
}


END_NCBI_SCOPE
