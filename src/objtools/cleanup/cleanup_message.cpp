/*  $Id: cleanup_message.cpp 608332 2020-05-14 16:04:14Z ivanov $
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
 * Author:  Justin Foley
 *
 * File Description:
 *   .......
 *
 */
#include <ncbi_pch.hpp>
#include <objtools/cleanup/cleanup_message.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

CCleanupMessage::CCleanupMessage(string text, EDiagSev sev, ECode code, ESubcode subcode)
    : CObjtoolsMessage(text, sev), m_Code(code), m_Subcode(subcode) {}

CCleanupMessage* CCleanupMessage::Clone(void) const 
{
    return new CCleanupMessage(GetText(), GetSeverity(), m_Code, m_Subcode);
}

END_SCOPE(objects)
END_NCBI_SCOPE    

