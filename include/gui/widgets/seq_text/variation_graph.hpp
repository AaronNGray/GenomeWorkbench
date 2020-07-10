#ifndef __GUI_WIDGETS_SEQTEXT___VARIATION_GRAPH__HPP
#define __GUI_WIDGETS_SEQTEXT___VARIATION_GRAPH__HPP

/*  $Id: variation_graph.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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

#include <corelib/ncbistl.hpp>
#include <gui/objutils/object_index.hpp>

#include <objmgr/feat_ci.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CSeqTextVariationGraph

class NCBI_GUIWIDGETS_SEQTEXT_EXPORT CSeqTextVariationGraph
{
public:
    CSeqTextVariationGraph(TSeqPos left, TSeqPos right, string repl_text);
    ~CSeqTextVariationGraph();

    string GetText();
    TSeqPos GetFeatLeft();
    TSeqPos GetFeatRight();
    TSeqPos GetFeatCenter();
    TSeqPos GetDrawWidth();
    unsigned int GetDisplayLine();
    void SetDisplayLine(unsigned int display_line);
private:
    TSeqPos m_FeatLeft;
    TSeqPos m_FeatRight;
    unsigned int m_DisplayLine;
    string m_Text;
};



END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_SEQTEXT___VARIATION_GRAPH__HPP
