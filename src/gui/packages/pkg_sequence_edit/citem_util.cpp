/* $Id: citem_util.cpp 28857 2013-09-05 17:53:50Z chenj $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Jie Chen 
 */
#include <ncbi_pch.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/citem_util.hpp>

using namespace ncbi;
using namespace objects;

bool CItem :: apply(const CSeq_entry_Handle& tse, ICommandProccessor* cmdProcessor, const string& title)
{
    if (tse) {
        CRef<CCmdComposite> composite(new CCmdComposite(title));
        apply_to(tse, composite);
        cmdProcessor->Execute(composite.GetPointer());
        return true;
    }
    else return false;
};
