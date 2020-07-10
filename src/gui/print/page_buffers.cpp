/*  $Id: page_buffers.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPageBuffers - holds buffers and information for a logical page
 *
 */

#include <ncbi_pch.hpp>
#include <gui/print/print_buffer.hpp>
#include "page_buffers.hpp"


BEGIN_NCBI_SCOPE



CPageBuffers::CPageBuffers()
{
}


CPageBuffers::~CPageBuffers()
{
}


void CPageBuffers::Add(const CPBuffer* buf)
{
    m_Buffers.push_back(buf);
    m_BoundingBox.Add(buf->GetBoundingBox());
}


void CPageBuffers::Clear(void)
{
    m_Buffers.clear();
}


bool CPageBuffers::Empty(void) const
{
    return (m_Buffers.size() == 0);
}


CBBox < 3> CPageBuffers::GetBoundingBox(void) const
{
    return m_BoundingBox;
}


CPageBuffers::TOffset CPageBuffers::GetPageSize(void) const
{
    const pair<float, float> xs = m_BoundingBox.GetNthRange(0);
    const pair<float, float> ys = m_BoundingBox.GetNthRange(1);
    return TOffset((unsigned int) (xs.second - xs.first),
                   (unsigned int) (ys.second - ys.first)
                  );
}


CPageBuffers::iterator CPageBuffers::begin(void)
{
    return m_Buffers.begin();
}


CPageBuffers::const_iterator CPageBuffers::begin(void) const
{
    return m_Buffers.begin();
}


CPageBuffers::iterator CPageBuffers::end(void)
{
    return m_Buffers.end();
}


CPageBuffers::const_iterator CPageBuffers::end(void) const
{
    return m_Buffers.end();
}


CPageBuffers::reverse_iterator CPageBuffers::rbegin(void)
{
    return m_Buffers.rbegin();
}


CPageBuffers::const_reverse_iterator CPageBuffers::rbegin(void) const
{
    return m_Buffers.rbegin();
}


CPageBuffers::reverse_iterator CPageBuffers::rend(void)
{
    return m_Buffers.rend();
}


CPageBuffers::const_reverse_iterator CPageBuffers::rend(void) const
{
    return m_Buffers.rend();
}



END_NCBI_SCOPE
