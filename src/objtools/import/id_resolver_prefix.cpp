/*  $Id: id_resolver_prefix.cpp 571255 2018-09-24 14:00:41Z ludwigf $
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
 * Author: Frank Ludwig
 *
 * File Description:  Iterate through file names matching a given glob pattern
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <objects/seqloc/Seq_loc.hpp>

#include "id_resolver_prefix.hpp"

USING_NCBI_SCOPE;
USING_SCOPE(objects);

//  ============================================================================
CIdResolverPrefix::CIdResolverPrefix(
    const string& prefix):
//  ============================================================================
    mPrefix(prefix)
{
}

//  ============================================================================
CIdResolverPrefix::~CIdResolverPrefix()
//  ============================================================================
{
}

//  ============================================================================
CRef<CSeq_id>
CIdResolverPrefix::operator() (
    const string& rawId) const
//  ============================================================================
{
    CRef<CSeq_id> pResolvedId(new CSeq_id(CSeq_id::e_Local, mPrefix + rawId));
    return pResolvedId;
}

