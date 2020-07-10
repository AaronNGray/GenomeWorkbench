/*  $Id: xml_version_item.cpp 23975 2011-06-28 15:22:08Z katargir $
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

#include <gui/widgets/seq/xml_version_item.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/seq/xml_styles.hpp>

BEGIN_NCBI_SCOPE

void CXmlVersionItem::x_RenderXml(CStyledTextOStream& ostream, CTextPanelContext*) const
{
    ostream
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << "<?"
        << CXmlStyles::GetStyle(CXmlStyles::kTag)        << "xml "
        << CXmlStyles::GetStyle(CXmlStyles::kParam)      << "version"
        << CXmlStyles::GetStyle(CXmlStyles::kParamValue) << "="
        << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
        << CXmlStyles::GetStyle(CXmlStyles::kParamValue) << "1.0"
        << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
        << CXmlStyles::GetStyle(CXmlStyles::kBracket)    << "?>" << NewLine();
}

END_NCBI_SCOPE
