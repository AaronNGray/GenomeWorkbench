/*  $Id: lblast_load_params.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/loaders/lblast_load_params.hpp>

BEGIN_NCBI_SCOPE


/*!
 * CLBLASTLoadParams type definition
 */

/*!
 * Default constructor for CLBLASTLoadParams
 */

CLBLASTLoadParams::CLBLASTLoadParams()
{
    Init();
}

/*!
 * Copy constructor for CLBLASTLoadParams
 */

CLBLASTLoadParams::CLBLASTLoadParams(const CLBLASTLoadParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CLBLASTLoadParams
 */

CLBLASTLoadParams::~CLBLASTLoadParams()
{
}

/*!
 * Assignment operator for CLBLASTLoadParams
 */

void CLBLASTLoadParams::operator=(const CLBLASTLoadParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CLBLASTLoadParams
 */

bool CLBLASTLoadParams::operator==(const CLBLASTLoadParams& data) const
{
////@begin CLBLASTLoadParams equality operator
     if (!(m_NucDB == data.m_NucDB)) return false;
     if (!(m_CreateSeqs == data.m_CreateSeqs)) return false;
     if (!(m_ProtDB == data.m_ProtDB)) return false;
     if (!(m_ProtSeqs == data.m_ProtSeqs)) return false;
////@end CLBLASTLoadParams equality operator
    return true;
}

/*!
 * Copy function for CLBLASTLoadParams
 */

void CLBLASTLoadParams::Copy(const CLBLASTLoadParams& data)
{
////@begin CLBLASTLoadParams copy function
    m_NucDB = data.m_NucDB;
    m_CreateSeqs = data.m_CreateSeqs;
    m_ProtDB = data.m_ProtDB;
    m_ProtSeqs = data.m_ProtSeqs;
////@end CLBLASTLoadParams copy function
}

/*!
 * Member initialisation for CLBLASTLoadParams
 */

void CLBLASTLoadParams::Init()
{
////@begin CLBLASTLoadParams member initialisation
    m_CreateSeqs = false;
    m_ProtSeqs = false;
////@end CLBLASTLoadParams member initialisation
}

static const char* kNucDB = "NucDB";
static const char* kProtDB = "ProtDB";
static const char* kProtSeqs = "ProtSeqs";
static const char* kCreateSeqs = "CreateSeqs";

void CLBLASTLoadParams::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CLBLASTLoadParams::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        string nucDB(m_NucDB.ToUTF8());
        m_NucDB = wxString::FromUTF8(view.GetString(kNucDB, nucDB).c_str());

        string protDB(m_ProtDB.ToUTF8());
        m_ProtDB = wxString::FromUTF8(view.GetString(kProtDB, protDB).c_str());

        m_ProtSeqs = view.GetBool(kProtSeqs, m_ProtSeqs);
        m_CreateSeqs = view.GetBool(kCreateSeqs, m_CreateSeqs);
    }
}

void CLBLASTLoadParams::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kNucDB, m_NucDB.ToUTF8());
        view.Set(kProtDB, m_ProtDB.ToUTF8());

        view.Set(kProtSeqs, m_ProtSeqs);
        view.Set(kCreateSeqs, m_CreateSeqs);
    }
}


END_NCBI_SCOPE
