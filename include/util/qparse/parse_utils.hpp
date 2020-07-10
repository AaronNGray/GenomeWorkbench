#ifndef UTIL__PARSE_UTILS_HPP__
#define UTIL__PARSE_UTILS_HPP__

/*  $Id: parse_utils.hpp 506813 2016-07-12 14:24:36Z asztalos $
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
 * Authors: Mike DiCuccio, Andrea Asztalos
 *
 * File Description: Utility functions for parsed trees
 *
 */

/// @file parse_util.hpp

#include <corelib/ncbi_tree.hpp>
#include <corelib/ncbiobj.hpp>
#include <util/qparse/query_parse.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup QParser
 *
 * @{
 */

/// Flatten the AND and the OR subtrees transforming them from binary subtrees to 
/// n-ary subtrees. 
/// @param node - the root node of the parsed tree
///
void NCBI_XUTIL_EXPORT Flatten_ParseTree(CQueryParseTree::TNode& node);

 
 /* @} */

END_NCBI_SCOPE


#endif  // UTIL__PARSE_UTILS_HPP__


