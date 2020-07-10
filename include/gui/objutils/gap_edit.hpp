#ifndef GUI_UTILS___GAP_EDIT__HPP
#define GUI_UTILS___GAP_EDIT__HPP

/*  $Id: gap_edit.hpp 36705 2016-10-25 20:07:22Z filippov $
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

#include <corelib/ncbiobj.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objects/seq/Seq_literal.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

// This object is intended to be created on the fly
// Not related to any other bio object
// But it can be used by Seqdesc editor for initialization and
// after modification applied to CBioseq_Handle.

class CGapEdit : public CSeq_literal
{
public:
    CGapEdit(CBioseq_Handle& bh, const TSeqPos from, const TSeqPos length) : m_BH(bh), m_From(from)
    {
        SetLength(length);
    }

    CBioseq_Handle GetBioseq_Handle() const { return m_BH; }
    TSeqPos GetFrom(void) const {return m_From;}
        
protected:
    CBioseq_Handle m_BH;
    TSeqPos m_From;
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif  // GUI_UTILS___GAP_EDIT__HPP
