#ifndef GUI_WIDGET_SEQ_GRAPHICS___CFEATSORTER__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___CFEATSORTER__HPP

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


#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

namespace objects {
    class CMappedFeat;
    class CSeq_align;
}

///////////////////////////////////////////////////////////////////////////////
/// IObjectSorter is an abstract interface for sorting objects based on
///             any criteria.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IObjectSorter
{
public:
    struct SSorterDescriptor {
        SSorterDescriptor(const string& name,
            const string& disp_name,
            const string& descr)
            : m_Name(name)
            , m_DisplayName(disp_name)
            , m_Descr(descr) {}

        string m_Name;
        string m_DisplayName;
        string m_Descr;
    };

    virtual ~IObjectSorter() {}

    /// Convert group index into group name.
    virtual string GroupIdxToName(int idx) const = 0;

    /// Return a name for the sorter.
    virtual SSorterDescriptor GetThisDescr() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// IFeatSorter is an abstract interface for sorting features based on
///             any criteria.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IFeatSorter : public IObjectSorter
{
public:
    /// Get the sorted group index for a given feature.
    /// @ret any index >=0 will be considered as a valid index.
    ///      Otherwise, it is an invalid index.
    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const = 0;

    /// Priority on sorting parent or child features.
    /// In case there are parent-child relation features, and the sorting
    /// may be in conflict for parent and child features, we need a priority
    /// of sorting parent feature or child features to decide which sorted
    /// group they should go together.
    virtual bool ChildFeatureFirst() const { return true; }
};



///////////////////////////////////////////////////////////////////////////////
/// IAlignSorter is an abstract interface for sorting alignments based on
///             any criteria.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT IAlignSorter : public IObjectSorter
{
public:
    /// Get the sorted group index for a given feature.
    /// @ret any index >=0 will be considered as a valid index.
    ///      Otherwise, it is an invalid index.
    virtual int GetGroupIdx(const objects::CSeq_align& align) const = 0;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___CFEATSORTER__HPP
