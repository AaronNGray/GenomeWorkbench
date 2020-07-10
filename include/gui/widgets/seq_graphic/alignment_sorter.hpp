#ifndef GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENTSORTER__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENTSORTER__HPP

/*  $Id:
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
 * Authors:  Liangshou Wu
 *
 */


#include <gui/widgets/seq_graphic/feature_sorter.hpp>
#include <set>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAlignStrandSorter is a sorter used to sort alignments based on
///        alignment strand. There are two categories:
///          - forward
///          - backward
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignStrandSorter
    : public CObject
    , public IAlignSorter
{
public:
    enum EGroup {
        eInvalid = -1, ///< maybe it is valid, but not considered
        eForward = 0,
        eReverse
    };

    CAlignStrandSorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CSeq_align& align) const;

    virtual SSorterDescriptor GetThisDescr() const;

    static const string& GetID();
    static SSorterDescriptor GetSorterDescr();

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't belong to any of the listed groups,
    /// an invalid group index will be assigned.
    set<EGroup>  m_SortGroups;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENTSORTER__HPP
