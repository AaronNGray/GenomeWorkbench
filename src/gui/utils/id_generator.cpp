/*  $Id: id_generator.cpp 28864 2013-09-05 19:16:28Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Peter Meric, Andrey Yazhuk
 *
 * File Description:
 *   CIdGenerator - generates consecutive integer identifiers
 */


#include <ncbi_pch.hpp>
#include <gui/utils/id_generator.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CIdGenerator - generates consecutive integer identifiers
CIdGenerator::CIdGenerator(unsigned int first_id)
    : m_Id(first_id)
{
}


CIdGenerator::~CIdGenerator()
{
}


unsigned int CIdGenerator::NextId(void)
{
    return m_Id++;
}


////////////////////////////////////////////////////////////////////////////////
/// CUniqueLabelGenerator
CUniqueLabelGenerator::CUniqueLabelGenerator()
{
}


void CUniqueLabelGenerator::AddExistingLabel(const string& label)
{
    m_ExistingLabels.insert(label);
}


string CUniqueLabelGenerator::MakeUniqueLabel(const string& label) const
{
    string s = label;
    int i = 0;

    set<string>::const_iterator it = m_ExistingLabels.find(s);

    while(it != m_ExistingLabels.end()) {
        // make a new unique label
        s = label + " (";
        s += NStr::IntToString(++i);
        s += ")";

        it = m_ExistingLabels.find(s);
    }
    return s;
}


END_NCBI_SCOPE

