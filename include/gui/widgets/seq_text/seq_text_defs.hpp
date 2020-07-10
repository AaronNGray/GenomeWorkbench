#ifndef __GUI_WIDGETS_SEQTEXT___SEQ_TEXT_DEFS__HPP
#define __GUI_WIDGETS_SEQTEXT___SEQ_TEXT_DEFS__HPP

/*  $Id: seq_text_defs.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
#include <gui/widgets/seq_text/variation_graph.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CSeqTextDefs

class CSeqTextDefs
{
public:
    typedef vector<int> TSubtypeVector;
    typedef vector<TSeqPos> TSeqPosVector;
    typedef vector<bool> TSpliceSiteVector;
    typedef vector<CSeqTextVariationGraph> TVariationGraphVector;
};



END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_SEQTEXT___SEQ_TEXT_DEFS__HPP
