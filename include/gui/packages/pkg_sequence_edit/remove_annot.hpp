/*  $Id: remove_annot.hpp 27132 2012-12-26 20:23:14Z filippov $
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
 * Authors:  Igor Filippov
 */
#ifndef _REMOVE_ANNOT_H_
#define _REMOVE_ANNOT_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_annot_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>

BEGIN_NCBI_SCOPE
using namespace objects;

class CRemoveSeqAnnot
{
public:
    bool ApplyToSeqAnot(const CSeq_annot_Handle& eh,  CSeq_annot::C_Data::E_Choice which);
    void ApplyToCSeq_entry (CSeq_entry_Handle tse, const CSeq_entry& se, CCmdComposite* composite, CSeq_annot::C_Data::E_Choice which);
    void apply_to_seq_and_feat(CSeq_entry_Handle tse, CCmdComposite* composite, CSeq_annot::C_Data::E_Choice which);
    bool apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, CSeq_annot::C_Data::E_Choice which);
};

END_NCBI_SCOPE

#endif
    // _REMOVE_ANNOT_H_
