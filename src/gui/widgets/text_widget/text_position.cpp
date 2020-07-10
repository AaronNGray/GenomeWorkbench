/*  $Id: text_position.cpp 19147 2009-04-15 21:44:08Z katargir $
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

#include <gui/widgets/text_widget/text_position.hpp>

BEGIN_NCBI_SCOPE

CNcbiOstream& operator<<(CNcbiOstream& strm, const CTextPosition& pos)
{
    return strm << " Ln " << pos.GetRow() + 1 << " Col " << pos.GetCol() + 1;
}

CNcbiOstream& operator<<(CNcbiOstream& strm, const CTextBlock& block)
{
    return strm << "Start " << block.GetStart() << " End " << block.GetEnd();
}

END_NCBI_SCOPE
