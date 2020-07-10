#ifndef GUI_OBJUTILS___TAXID_SEL_SET__HPP
#define GUI_OBJUTILS___TAXID_SEL_SET__HPP

/*  $Id: taxid_sel_set.hpp 26143 2012-07-25 19:53:48Z falkrb $
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
 *  Government have not placed CAnyType restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for CAnyType particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Robert Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <util/bitset/bm.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Class designed to hold a set of tax-ids represented as (on) bits in 
/// a bit vector
class  NCBI_GUIOBJUTILS_EXPORT CTaxIdSelSet : public CObject
{
public:
    typedef bm::bvector<> TBitVec;
    typedef bm::id_t TTaxId;

    CTaxIdSelSet() {}
    ~CTaxIdSelSet() {}

    const TBitVec& GetSelectedTaxIds() const { return m_SelectedTaxIds; }
    TBitVec& GetSelectedTaxIds() { return m_SelectedTaxIds; }

    void SelectTaxId(TTaxId tid) { m_SelectedTaxIds.set_bit(tid, true); }
    void DeSelectTaxId(TTaxId tid) { m_SelectedTaxIds.set_bit(tid, false); }

    bool IsSelected(TTaxId tid) const { return m_SelectedTaxIds[tid]; }

protected:

    /// Bit vector of  taxids
    TBitVec  m_SelectedTaxIds;
};


END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___TAXID_SEL_SET__HPP
