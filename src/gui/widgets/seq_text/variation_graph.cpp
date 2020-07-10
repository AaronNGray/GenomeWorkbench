/*  $Id: variation_graph.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>
#include <gui/widgets/seq_text/variation_graph.hpp>

#include <objects/seqfeat/Gb_qual.hpp>
#include <objmgr/seq_vector.hpp>

BEGIN_NCBI_SCOPE


CSeqTextVariationGraph::CSeqTextVariationGraph(TSeqPos left, TSeqPos right, string repl_text)
{
    m_FeatLeft = min (left, right);
    m_FeatRight = max (left, right);

    m_Text = repl_text;
    int diff = m_FeatRight - m_FeatLeft + 1 - repl_text.length();
    for (int k = 0; k <diff; k++) {
        m_Text.append("-");
    }
    m_DisplayLine = 1;
}


CSeqTextVariationGraph::~CSeqTextVariationGraph()
{
}


string CSeqTextVariationGraph::GetText()
{
    return m_Text;
}


TSeqPos CSeqTextVariationGraph::GetFeatLeft()
{
    return m_FeatLeft;
}


TSeqPos CSeqTextVariationGraph::GetFeatRight()
{
    return m_FeatRight;
}


TSeqPos CSeqTextVariationGraph::GetDrawWidth()
{
    TSeqPos feat_width = m_FeatRight - m_FeatLeft + 1;

    return max(feat_width, (TSeqPos)m_Text.length());
}

TSeqPos CSeqTextVariationGraph::GetFeatCenter()
{
    TSeqPos feat_center = m_FeatLeft + (m_FeatRight - m_FeatLeft + 1) / 2;
    return feat_center;
}


unsigned int CSeqTextVariationGraph::GetDisplayLine()
{
    return m_DisplayLine;
}


void CSeqTextVariationGraph::SetDisplayLine(unsigned int display_line)
{
    m_DisplayLine = display_line;
}


END_NCBI_SCOPE
