/*  $Id: string_formatters.cpp 24305 2011-09-09 15:54:30Z kuznets $
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
* Authors:  Andrey Yazhuk
*
* File Description:
*
*/
#include <ncbi_pch.hpp>

#include <corelib/ncbistr.hpp>

#include <gui/widgets/wx/string_formatters.hpp>
#include <gui/widgets/wx/wx_utils.hpp>


BEGIN_NCBI_SCOPE

wxString IwxStringFormat::Format( wxVariant& aVar ) const
{
    wxString rv;
    Format( aVar, rv );

    return rv;
}

void CwxIntPrintfFormat::Format( wxVariant& aVar, wxString& aStr, bool afAppend ) const
{
    if( afAppend ){
        aStr += wxString::Format( m_Format, aVar.GetInteger() );
    } else {
        aStr.Printf( m_Format, aVar.GetInteger() );
    }
}

void CwxDoublePrintfFormat::Format( wxVariant& aVar, wxString& aStr, bool afAppend ) const
{
    if( afAppend ){
        aStr += wxString::Format( m_Format, aVar.GetDouble() );
    } else {
        aStr.Printf( m_Format, aVar.GetDouble() );
    }
}

void CwxIntWithFlagsFormat::Format( wxVariant& aVar, wxString& aStr, bool afAppend ) const
{
    int i = aVar.GetInteger();
    if( afAppend ){
        aStr += ToWxString( NStr::IntToString( i, mf_Flags ) );
    } else {
        aStr = ToWxString( NStr::IntToString( i, mf_Flags ) );
    }
}

END_NCBI_SCOPE
