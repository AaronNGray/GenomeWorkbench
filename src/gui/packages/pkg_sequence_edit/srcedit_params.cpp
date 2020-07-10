/*  $Id: srcedit_params.cpp 39591 2017-10-13 14:45:28Z katargir $
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
 * Authors:  Colleen Bollin, based on a file by Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/packages/pkg_sequence_edit/srcedit_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * CSrcEditParams type definition
 */

/*!
 * Default constructor for CSrcEditParams
 */

CSrcEditParams::CSrcEditParams()
{
    Init();
}

/*!
 * Copy constructor for CSrcEditParams
 */

CSrcEditParams::CSrcEditParams(const CSrcEditParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CCleanupParams
 */

CSrcEditParams::~CSrcEditParams()
{
}

/*!
 * Assignment operator for CSrcEditParams
 */

void CSrcEditParams::operator=(const CSrcEditParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CSrcEditParams
 */

bool CSrcEditParams::operator==(const CSrcEditParams& data) const
{
////@begin CSrcEditParams equality operator
////@end CSrcEditParams equality operator
    return true;
}

/*!
 * Copy function for CSrcEditParams
 */

void CSrcEditParams::Copy(const CSrcEditParams& data)
{
////@begin CSrcEditParams copy function
////@end CSrcEditParams copy function
    m_Objects = data.m_Objects;
}

/*!
 * Member initialisation for CSrcEditParams
 */

void CSrcEditParams::Init()
{
////@begin CSrcEditParams member initialisation
////@end CSrcEditParams member initialisation
}

/// IRegSettings
void CSrcEditParams::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CSrcEditParams::SaveSettings() const
{
}

void CSrcEditParams::LoadSettings()
{
}

END_NCBI_SCOPE
