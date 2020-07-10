/*  $Id: 5col_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence/5col_export_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

/*!
 * C5ColExportParams type definition
 */

/*!
 * Default constructor for C5ColExportParams
 */

C5ColExportParams::C5ColExportParams()
{
    Init();
}

/*!
 * Copy constructor for C5ColExportParams
 */

C5ColExportParams::C5ColExportParams(const C5ColExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for C5ColExportParams
 */

C5ColExportParams::~C5ColExportParams()
{
}

/*!
 * Assignment operator for C5ColExportParams
 */

void C5ColExportParams::operator=(const C5ColExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for C5ColExportParams
 */

bool C5ColExportParams::operator==(const C5ColExportParams& data) const
{
////@begin C5ColExportParams equality operator
     if (!(m_FeatureDepth == data.m_FeatureDepth)) return false;
     if (!(m_ExactFlevel == data.m_ExactFlevel)) return false;
////@end C5ColExportParams equality operator
    return true;
}

/*!
 * Copy function for C5ColExportParams
 */

void C5ColExportParams::Copy(const C5ColExportParams& data)
{
////@begin C5ColExportParams copy function
    m_FeatureDepth = data.m_FeatureDepth;
    m_ExactFlevel = data.m_ExactFlevel;
////@end C5ColExportParams copy function
}

/*!
 * Member initialisation for C5ColExportParams
 */

void C5ColExportParams::Init()
{
////@begin C5ColExportParams member initialisation
    m_FeatureDepth = wxT("1");
    m_ExactFlevel = false;
////@end C5ColExportParams member initialisation
}

static const char* kFeatureDepth = "FeatureDepth";
static const char* kExactFlevel = "ExactFlevel";

/// IRegSettings
void C5ColExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kFeatureDepth, ToStdString( m_FeatureDepth ));
        view.Set(kExactFlevel, m_ExactFlevel);
    }
}

void C5ColExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_FeatureDepth =
            ToWxString(view.GetString(kFeatureDepth,
                                      ToStdString(m_FeatureDepth)));
        m_ExactFlevel = view.GetBool(kExactFlevel, m_ExactFlevel);
    }
}

END_NCBI_SCOPE
