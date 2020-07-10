/*  $Id: tree_builder_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin, Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/packages/pkg_alignment/tree_builder_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

CTreeBuilderParams::CTreeBuilderParams()
{
    Init();
}
CTreeBuilderParams::CTreeBuilderParams(const CTreeBuilderParams& data)
{
    Init();
    Copy(data);
}
CTreeBuilderParams::~CTreeBuilderParams()
{
}
void CTreeBuilderParams::operator=(const CTreeBuilderParams& data)
{
    Copy(data);
}
bool CTreeBuilderParams::operator==(const CTreeBuilderParams& data) const
{
////@begin CTreeBuilderParams equality operator
     if (!(m_DistanceMethod == data.m_DistanceMethod)) return false;
     if (!(m_ConstructMethod == data.m_ConstructMethod)) return false;
     if (!(m_LeafLabels == data.m_LeafLabels)) return false;     
////@end CTreeBuilderParams equality operator
    return true;
}
void CTreeBuilderParams::Copy(const CTreeBuilderParams& data)
{
////@begin CTreeBuilderParams copy function
    m_DistanceMethod = data.m_DistanceMethod;
    m_ConstructMethod = data.m_ConstructMethod;
    m_LeafLabels = data.m_LeafLabels;
////@end CTreeBuilderParams copy function
    m_Object = data.m_Object;
}
void CTreeBuilderParams::Init()
{
////@begin CTreeBuilderParams member initialisation
    
////@end CTreeBuilderParams member initialisation
}

static const char* kDistTag = "kDistTag";
static const char* kConsTag = "kConsTag";
static const char* kLeafTag = "kLeafTag";

/// IRegSettings
void CTreeBuilderParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kDistTag, ToStdString( m_DistanceMethod ));
        view.Set(kConsTag, ToStdString( m_ConstructMethod ));
        view.Set(kLeafTag, ToStdString( m_LeafLabels ));
    }
}

void CTreeBuilderParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_DistanceMethod    = ToWxString(view.GetString(kDistTag, ""));
        m_ConstructMethod   = ToWxString(view.GetString(kConsTag, ""));
        m_LeafLabels        = ToWxString(view.GetString(kLeafTag, ""));
    }
}

END_NCBI_SCOPE
