#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEARCH_UTILS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEARCH_UTILS__HPP

/*  $Id: search_utils.hpp 44617 2020-02-06 19:35:49Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE
class CFeaturePanel;

void s_GatherFeatures(const string &text, bool match_case,  objects::CBioseq_Handle bsh, objects::CScope &scope, CFeaturePanel *panel, vector<objects::CMappedFeat> &search_results);
TSeqRange s_splitPosOrRange(const string &text);
void s_GetSubtypesForType(set<objects::CSeqFeatData::ESubtype> &subtypes, objects::CSeqFeatData::E_Choice feat);

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEARCH_UTILS__HPP
