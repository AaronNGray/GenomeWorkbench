/*  $Id: asn_header_primitive.cpp 19246 2009-05-06 19:02:00Z katargir $
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

#include <serial/typeinfo.hpp>

#include <gui/widgets/seq/asn_header_primitive.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>

#include "asn_styles.hpp"

BEGIN_NCBI_SCOPE

void CAsnHeaderPrimitive::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
{
    ostream << CAsnStyles::GetStyle(CAsnStyles::kHeader1)
            << m_Type->GetName()
            << CAsnStyles::GetStyle(CAsnStyles::kHeader2)
            << " ::= ";
    CAsnElementPrimitive::x_RenderText(ostream, context);
}

END_NCBI_SCOPE
