#ifndef GUI_GRAPH___IGRAPH_UTILS__HPP
#define GUI_GRAPH___IGRAPH_UTILS__HPP

/*  $Id: igraph_utils.hpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbistd.hpp>

#include <set>
#include <algorithm>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

template<class P> void  destroy(P pT)  { delete  pT; }

template<class P> void  destroy_null(P& pT)  {   delete  pT;  pT = NULL; }

template<class P> void  destroy_null_array(P& pT)  {   delete[]  pT;  pT = NULL; }

template<class P> struct Destroyer
{
    static void  destroy(P pT)  { delete  pT; };
    static void  destroy_null(P& pT)  {   delete  pT;  pT = NULL; };
};

template<class ContT>  void  destroy_and_erase_elems(ContT& C)
{
    typedef typename ContT::value_type value_type;
    for_each(C.begin(), C.end(), Destroyer<value_type>::destroy);
    C.clear();
}

template<class InIt>  void  destroy_elems(InIt first, InIt last)
{
    typedef typename InIt::value_type value_type;
    for_each(first, last, Destroyer<value_type>::destroy);
}

template<class T> bool  set_contains(const set<T>& Set, const T& Elem)
{
    return Set.find(Elem)!=Set.end();
}

END_NCBI_SCOPE

#endif
