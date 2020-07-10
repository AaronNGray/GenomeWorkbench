/*  $Id: xml_styles.cpp 19243 2009-05-05 19:37:05Z katargir $
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
* Authors:  Roman Katargin
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/widgets/seq/xml_styles.hpp>

BEGIN_NCBI_SCOPE

vector<CTextStyle> CXmlStyles::m_Styles;

const CTextStyle& CXmlStyles::GetStyle(Element element)
{
    static CXmlStyles instance;
    return m_Styles[element];
}

CXmlStyles::CXmlStyles()
{
    m_Styles.push_back(CTextStyle(0,0,         255,255,255,255,wxTRANSPARENT,false,false)); // kBracket
    m_Styles.push_back(CTextStyle(255,0,0,     255,255,255,wxTRANSPARENT,false,false));     // kParam
    m_Styles.push_back(CTextStyle(0,0,         255,255,255,255,wxTRANSPARENT,false,false)); // kParamValue
    m_Styles.push_back(CTextStyle(0,0,0,       255,255,255,wxTRANSPARENT,false,false));     // kParamQuote
    m_Styles.push_back(CTextStyle(163,21,21,   255,255,255,wxTRANSPARENT,false,false));     // kTag
    m_Styles.push_back(CTextStyle(0,0,0,       255,255,255,wxTRANSPARENT,false,false));     // kTagValue
    m_Styles.push_back(CTextStyle(255,0,0,     255,255,255,wxTRANSPARENT,false,false));     // kEscChar
    m_Styles.push_back(CTextStyle(128,128,128, 255,255,255,wxTRANSPARENT,false,false));      // kCollapsedBlock
    m_Styles.push_back(CTextStyle(255,0,0,     255,255,255,wxSOLID,true,false));            // kError

}

END_NCBI_SCOPE
