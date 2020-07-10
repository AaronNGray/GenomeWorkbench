#ifndef GUI_SERVICES___SEQLOC_SEARCH_CONTEXT_TOOL_HPP
#define GUI_SERVICES___SEQLOC_SEARCH_CONTEXT_TOOL_HPP

/*  $Id: seqloc_search_context.hpp 19359 2009-06-08 20:46:01Z tereshko $
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

#include <gui/core/data_mining_context.hpp>

#include <gui/objutils/objects.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/scope.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// ISeqlocSearchContext
class NCBI_GUICORE_EXPORT ISeqLocSearchContext : public IDataMiningContext
{   
public:
    virtual ~ISeqLocSearchContext(){}
    virtual CRef<objects::CSeq_loc>  GetSearchLoc() = 0;
    virtual CRef<objects::CScope>    GetSearchScope() = 0;

    // handling multiple location extensions
    virtual void GetMultiple(TConstScopedObjects &) {}; 
};


END_NCBI_SCOPE


/*
 * ===========================================================================
 * ===========================================================================
 */

#endif  // GUI_SERVICES___SEQLOC_SEARCH_CONTEXT_TOOL_HPP
