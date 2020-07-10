/*  $Id: gff_export_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence/gff_export_params.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CGffExportParams type definition
 */

/*!
 * Default constructor for CGffExportParams
 */

CGffExportParams::CGffExportParams()
{
    Init();
}

/*!
 * Copy constructor for CGffExportParams
 */

CGffExportParams::CGffExportParams(const CGffExportParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CGffExportParams
 */

CGffExportParams::~CGffExportParams()
{
}

/*!
 * Assignment operator for CGffExportParams
 */

void CGffExportParams::operator=(const CGffExportParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CGffExportParams
 */

bool CGffExportParams::operator==(const CGffExportParams& data) const
{
////@begin CGffExportParams equality operator
     if (!(m_FeatureDepth == data.m_FeatureDepth)) return false;
     if (!(m_ExactFlevel == data.m_ExactFlevel)) return false;
     if (!(m_FileName == data.m_FileName)) return false;
     if (!(m_ExtraQuals == data.m_ExtraQuals)) return false;
////@end CGffExportParams equality operator
    return true;
}

/*!
 * Copy function for CGffExportParams
 */

void CGffExportParams::Copy(const CGffExportParams& data)
{
////@begin CGffExportParams copy function
    m_FeatureDepth = data.m_FeatureDepth;
    m_ExactFlevel = data.m_ExactFlevel;
    m_FileName = data.m_FileName;
    m_ExtraQuals = data.m_ExtraQuals;
////@end CGffExportParams copy function

    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CGffExportParams
 */

void CGffExportParams::Init()
{
////@begin CGffExportParams member initialisation
    m_FeatureDepth = wxT("1");
    m_ExactFlevel = false;
    m_ExtraQuals = false;
////@end CGffExportParams member initialisation
}

static const char* kFeatureDepth = "FeatureDepth";
static const char* kExactFlevel = "ExactFlevel";
static const char* kFileName = "FileName";
static const char* kExtraQuals = "ExtraQuals";

/// IRegSettings
void CGffExportParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kFeatureDepth, ToStdString( m_FeatureDepth ));
        view.Set(kExactFlevel, m_ExactFlevel);
        view.Set(kExtraQuals, m_ExtraQuals);
        view.Set(kFileName, FnToStdString( m_FileName ));
    }
}

void CGffExportParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_FeatureDepth =
            ToWxString(view.GetString(kFeatureDepth,
                                      ToStdString(m_FeatureDepth)));
        m_ExactFlevel = view.GetBool(kExactFlevel, m_ExactFlevel);
        m_ExtraQuals = view.GetBool(kExtraQuals, m_ExtraQuals);
        m_FileName =
            FnToWxString(view.GetString(kFileName, FnToStdString(m_FileName)));
    }
}

END_NCBI_SCOPE
