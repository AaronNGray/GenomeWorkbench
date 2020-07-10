/*  $Id: unit_test_format_guess.cpp 458251 2015-02-03 15:18:58Z kachalos $
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
* Author:  Sema
*
* File Description: Unit test for CFormatGuess class
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include <util/format_guess.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>

USING_NCBI_SCOPE;

// Make sure that no format is missing. Jira: MSS-289
BOOST_AUTO_TEST_CASE(Test_All_Formats_Enum)
{
	CFormatGuess Guess;
    for (int fmt = 1; fmt < (int)CFormatGuess::eFormat_max; fmt++) {
        if ( fmt == CFormatGuess::eGtf_POISENED) continue;
        Guess.TestFormat((CFormatGuess::EFormat)fmt, CFormatGuess::eDefault);
    }
}
