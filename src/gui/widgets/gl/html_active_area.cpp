/*  $Id: html_active_area.cpp 35291 2016-04-20 18:03:16Z evgeniev $
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
#include <corelib/ncbistd.hpp>

#include <gui/widgets/gl/html_active_area.hpp>

BEGIN_NCBI_SCOPE


CHTMLActiveArea::CHTMLActiveArea()
    : m_Type(eInvalid)
    , m_Flags(0)
    , m_PositiveStrand(true)
    , m_MergedFeatsCount(0)
{
}

CHTMLActiveArea::CHTMLActiveArea(int type, const TVPRect& bounds,
                                 const string& id,
                                 const string& descr,
                                 const string& action)
    : m_Bounds(bounds)
    , m_Type(type)
    , m_Flags(0)
    , m_PositiveStrand(true)
    , m_ID(id)
    , m_Descr(descr)
    , m_Action(action)
    , m_MergedFeatsCount(0)
{
}

END_NCBI_SCOPE
