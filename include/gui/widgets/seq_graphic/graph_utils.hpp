#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_UTILS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_UTILS__HPP

/* $Id: graph_utils.hpp 35915 2016-07-11 15:01:10Z katargir $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbistr.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
class CSeq_loc;
class CScope;
END_SCOPE(objects)

class CGraphUtils
{
public:
    static void CalcGraphLevels(const string& annotName, objects::CScope& scope, const objects::CSeq_loc& loc, set<int>& levels);
    /// @name help methods about coverage graph levels.
    /// @{
    /// Return the nearest level available for a given zoom level.
    /// @param zoom is measured as number of bases per screen pixel
    /// Initialize levels on which coverage graphs are avaiable.
    /// @param annot the original annotation name
    static int  GetNearestLevel(const set<int>& levels, double zoom);
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_UTILS__HPP
