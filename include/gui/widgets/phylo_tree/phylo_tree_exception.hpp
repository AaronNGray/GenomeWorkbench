#ifndef __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_EXCEPTION__HPP
#define __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_EXCEPTION__HPP

/*  $Id: phylo_tree_exception.hpp 43628 2019-08-09 18:55:17Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbiexpt.hpp>


BEGIN_NCBI_SCOPE

class CPhyloTreeException : public CException
{
public:
    enum EErrCode {
        eNoValue
    };

    virtual const char* GetErrCodeString(void) const override
    {
        switch (GetErrCode()) {
        case eNoValue: return "eNoValue";
        default:     return CException::GetErrCodeString();
        }
    }

    // Standard exception boilerplate code.
    NCBI_EXCEPTION_DEFAULT(CPhyloTreeException, CException);
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_EXCEPTION__HPP
