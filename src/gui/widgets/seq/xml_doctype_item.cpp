/*  $Id: xml_doctype_item.cpp 23975 2011-06-28 15:22:08Z katargir $
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

#include <gui/widgets/seq/xml_doctype_item.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/seq/xml_styles.hpp>

BEGIN_NCBI_SCOPE

static string GetPublicModuleName(TTypeInfo type)
{
    const string& s = type->GetModuleName();
    string name;
    for ( string::const_iterator i = s.begin(); i != s.end(); ++i ) {
        char c = *i;
        if ( !isalnum((unsigned char) c) )
            name += ' ';
        else
            name += c;
    }
    return name;
}

static string GetDTDModuleName(TTypeInfo type)
{
    string name;
    const string& s = type->GetModuleName();
    for ( string::const_iterator i = s.begin(); i != s.end(); ++i ) {
        char c = *i;
        if ( c == '-' )
            name += '_';
        else
            name += c;
    }
    return name;
}

void CXmlDoctypeItem::x_RenderXml(CStyledTextOStream& ostream, CTextPanelContext*) const
{
    TTypeInfo type = m_ObjectInfo.GetTypeInfo();

    ostream
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << "<!"
        << CXmlStyles::GetStyle(CXmlStyles::kTag)        << "DOCTYPE "
        << CXmlStyles::GetStyle(CXmlStyles::kParam)      << type->GetName()
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << " PUBLIC"
        << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << " \""
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << "-//NCBI//" << GetPublicModuleName(type) << "/EN"
        << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
        << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << " \""
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << GetDTDModuleName(type) << ".dtd"
        << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << ">" << NewLine();
}

END_NCBI_SCOPE
