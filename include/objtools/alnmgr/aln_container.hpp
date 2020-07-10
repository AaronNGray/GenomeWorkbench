#ifndef OBJTOOLS_ALNMGR___ALN_CONTAINER__HPP
#define OBJTOOLS_ALNMGR___ALN_CONTAINER__HPP
/*  $Id: aln_container.hpp 601341 2020-02-05 20:27:33Z vasilche $
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
* Author:  Kamen Todorov, NCBI
*
* File Description:
*   Seq-align container
*
* ===========================================================================
*/


#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/seqalign_exception.hpp>

#include <unordered_map>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/// CSeq_align container. Stores a list of alignments, prevents insertion of
/// duplicate objects, splits align-sets if necessary (on by default).
class CAlnContainer
{
public:
    NCBI_XALNMGR_EXPORT CAlnContainer();

    virtual NCBI_XALNMGR_EXPORT ~CAlnContainer(void);

private:
    typedef CSeq_align::TSegs TSegs;
    typedef list< CConstRef<CSeq_align> > TAlnSet;
    TAlnSet m_AlnSet;
    unordered_map<const CSeq_align*, TAlnSet::const_iterator> m_AlnMap;

public:
    typedef TAlnSet::size_type size_type;
    typedef TAlnSet::const_iterator const_iterator;
    typedef TAlnSet::const_reverse_iterator const_reverse_iterator;

    /// Enable/disable splitting disc alignments.
    void SetSplitDisc(void)
    {
        m_SplitDisc = true;
    }
    void UnsetSplitDisc(void)
    {
        m_SplitDisc = false;
    }
    bool IsSetSplitDisc(void)
    {
        return m_SplitDisc;
    }

    /// Insert new CSeq_align into the list. Don't insert multiple pointers to
    /// the same object.
    /// @return
    ///   Return iterator poiting to the inserted (or the existing) object.
    const_iterator NCBI_XALNMGR_EXPORT insert(const CSeq_align& seq_align);

    const_iterator begin(void) const
    {
        return m_AlnSet.begin();
    }

    const_iterator end(void) const
    {
        return m_AlnSet.end();
    }

    const_reverse_iterator rbegin(void) const
    {
        return m_AlnSet.rbegin();
    }

    const_reverse_iterator rend(void) const
    {
        return m_AlnSet.rend();
    }

    size_type size(void) const
    {
        return m_AlnSet.size();
    }

    bool empty(void) const
    {
        return m_AlnSet.empty();
    }

private:
    bool m_SplitDisc;
};


END_NCBI_SCOPE

#endif  // OBJTOOLS_ALNMGR___ALN_CONTAINER__HPP
