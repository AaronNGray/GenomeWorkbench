/*  $Id: gff_load_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/widgets/loaders/gff_load_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CGffLoadParams type definition
 */

/*!
 * Default constructor for CGffLoadParams
 */

CGffLoadParams::CGffLoadParams()
{
    Init();
}

/*!
 * Copy constructor for CGffLoadParams
 */

CGffLoadParams::CGffLoadParams(const CGffLoadParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CGffLoadParams
 */

CGffLoadParams::~CGffLoadParams()
{
}

/*!
 * Assignment operator for CGffLoadParams
 */

void CGffLoadParams::operator=(const CGffLoadParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CGffLoadParams
 */

bool CGffLoadParams::operator==(const CGffLoadParams& data) const
{
////@begin CGffLoadParams equality operator
     if (!(m_FileFormat == data.m_FileFormat)) return false;
     if (!(m_MapAssembly == data.m_MapAssembly)) return false;
     if (!(m_NameFeatSet == data.m_NameFeatSet)) return false;
     if (!(m_ParseSeqIds == data.m_ParseSeqIds)) return false;
////@end CGffLoadParams equality operator
    return true;
}

/*!
 * Copy function for CGffLoadParams
 */

void CGffLoadParams::Copy(const CGffLoadParams& data)
{
////@begin CGffLoadParams copy function
    m_FileFormat = data.m_FileFormat;
    m_MapAssembly = data.m_MapAssembly;
    m_NameFeatSet = data.m_NameFeatSet;
    m_ParseSeqIds = data.m_ParseSeqIds;
////@end CGffLoadParams copy function
}

/*!
 * Member initialisation for CGffLoadParams
 */

void CGffLoadParams::Init()
{
////@begin CGffLoadParams member initialisation
    m_FileFormat = 0;
    m_ParseSeqIds = 0;
////@end CGffLoadParams member initialisation
}

static const char* kParseSeqIds = "ParseSeqIds";
static const char* kNameFeatSet = "NameFeatSet";
static const char* kFileFormat  = "FileFormat";
static const char* kMapAssembly = ".MapAssembly";

/// IRegSettings
void CGffLoadParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kParseSeqIds, m_ParseSeqIds);
        view.Set(kNameFeatSet, ToStdString( m_NameFeatSet ));
        view.Set(kFileFormat, m_FileFormat);

        view = gui_reg.GetWriteView(m_RegPath + kMapAssembly);
        m_MapAssembly.SaveSettings(view);
    }
}

void CGffLoadParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_ParseSeqIds = view.GetInt(kParseSeqIds, m_ParseSeqIds);
        m_NameFeatSet = ToWxString(view.GetString(kNameFeatSet,
                                                  ToStdString(m_NameFeatSet)));
        m_FileFormat  = view.GetInt(kFileFormat, m_FileFormat);

        view = gui_reg.GetReadView(m_RegPath + kMapAssembly);
        m_MapAssembly.LoadSettings(view);
    }
}

END_NCBI_SCOPE
