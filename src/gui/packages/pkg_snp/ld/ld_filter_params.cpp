/*  $Id: ld_filter_params.cpp 19845 2009-08-13 12:59:38Z quintosm $
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
 * Authors:  Melvin Quintos
 */


#include <ncbi_pch.hpp>

////@begin includes
#include <gui/packages/pkg_snp/ld/ld_filter_params.hpp>

////@end includes

BEGIN_NCBI_SCOPE

/*!
 * CLDFilterParams type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLDFilterParams, wxObject )

/*!
 * Default constructor for CLDFilterParams
 */

CLDFilterParams::CLDFilterParams()
{
    Init();
}

/*!
 * Copy constructor for CLDFilterParams
 */

CLDFilterParams::CLDFilterParams(const CLDFilterParams& data)
{
    Init();
    Copy(data);
}

/*!
 * Destructor for CLDFilterParams
 */

CLDFilterParams::~CLDFilterParams()
{
}

/*!
 * Assignment operator for CLDFilterParams
 */

void CLDFilterParams::operator=(const CLDFilterParams& data)
{
    Copy(data);
}

/*!
 * Equality operator for CLDFilterParams
 */

bool CLDFilterParams::operator==(const CLDFilterParams& data) const
{
////@begin CLDFilterParams equality operator
     if (!(m_nScore == data.m_nScore)) return false;
     if (!(m_nLengthExp == data.m_nLengthExp)) return false;
////@end CLDFilterParams equality operator
    return true;
}

/*!
 * Copy function for CLDFilterParams
 */

void CLDFilterParams::Copy(const CLDFilterParams& data)
{
////@begin CLDFilterParams copy function
    m_nScore = data.m_nScore;
    m_nLengthExp = data.m_nLengthExp;
////@end CLDFilterParams copy function
}

/*!
 * Member initialisation for CLDFilterParams
 */

void CLDFilterParams::Init()
{
////@begin CLDFilterParams member initialisation
    m_nScore = 0;
    m_nLengthExp = 0;
////@end CLDFilterParams member initialisation
}

END_NCBI_SCOPE

