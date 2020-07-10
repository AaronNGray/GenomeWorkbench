/*  $Id: taxtree_tool_params.cpp 37336 2016-12-23 21:13:16Z katargir $
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
#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_alignment/taxtree_tool_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CTaxTreeToolParams::CTaxTreeToolParams()
{
    Init();
}
CTaxTreeToolParams::CTaxTreeToolParams(const CTaxTreeToolParams& data)
{
    Init();
    Copy(data);
}
CTaxTreeToolParams::~CTaxTreeToolParams()
{
}
void CTaxTreeToolParams::operator=(const CTaxTreeToolParams& data)
{
    Copy(data);
}
bool CTaxTreeToolParams::operator==(const CTaxTreeToolParams& data) const
{
////@begin CTaxTreeToolParams equality operator
////@end CTaxTreeToolParams equality operator
    return true;
}
void CTaxTreeToolParams::Copy(const CTaxTreeToolParams& data)
{
////@begin CTaxTreeToolParams copy function
////@end CTaxTreeToolParams copy function
    m_Objects = data.m_Objects;
}
void CTaxTreeToolParams::Init()
{
////@begin CTaxTreeToolParams member initialisation
////@end CTaxTreeToolParams member initialisation
}

void CTaxTreeToolParams::SaveSettings() const
{
    //if (!m_RegPath.empty()) {
    //    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    //}
}

void CTaxTreeToolParams::LoadSettings()
{
    //if (!m_RegPath.empty()) {
    //    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    //}
}

END_NCBI_SCOPE
