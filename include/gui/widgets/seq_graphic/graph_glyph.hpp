#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_GLYPH__HPP

/*  $Id: graph_glyph.hpp 20151 2009-09-29 15:41:34Z wuliangs $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <objmgr/graph_ci.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///    CGraphGlyph -- a speical histograms glyph converted from a seq-graph.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGraphGlyph : public CHistogramGlyph
{
public:
    /// @name ctors
    /// @{
    CGraphGlyph(const objects::CMappedGraph& graph, const TMap& map);
    /// @}

    const objects::CMappedGraph&  GetGraph(void) const;

private:
    // actual graph information
    // CConstRef<CMappedGraph>  m_SeqGraph;
    objects::CMappedGraph m_SeqGraph;
};


inline
const objects::CMappedGraph& CGraphGlyph::GetGraph(void) const
{ return m_SeqGraph; }


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_GLYPH__HPP
