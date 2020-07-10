#ifndef GUI_WIDGET_SEQ_GRAPHICS___DBVARFEATSORTER__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___DBVARFEATSORTER__HPP

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
/// CDbvarQualitySorter is a feature sorter used to sort variant features
///        based on variant quality. There are two categories:
///          - high (high or not set)
///          - low 

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDbvarQualitySorter
    : public CObject
    , public IFeatSorter
{
public:
    enum EGroup {
        eInvalid = -1, ///< maybe it is valid, but not considered
        eHigh = 0,
        eLow
    };

    CDbvarQualitySorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const;

    virtual SSorterDescriptor GetThisDescr() const;

    static const string& GetID();
    static SSorterDescriptor GetSorterDescr();

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't belong to any of the listed groups,
    /// an invalid group index will be assigned.
    set<EGroup>  m_SortGroups;
};



///////////////////////////////////////////////////////////////////////////////
/// CDbvarPilotSorter is a feature sorter used to sort variant features
///        based on variant pilot number. There are many categories:
///          - pilot1
///          - pilot2
///          - pilot3
///          - pilot4
///          - pilot5
///          - pilot not set
///          - other

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDbvarPilotSorter
    : public CObject
    , public IFeatSorter
{
public:
    enum EGroup {
        eInvalid = -1, ///< maybe it is valid, but not considered
        ePilot1 = 0,
        ePilot2,
        ePilot3,
        ePilot4,
        ePilot5,
        eNotSet,
        eOther
    };

    CDbvarPilotSorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const;

    virtual SSorterDescriptor GetThisDescr() const;

    static const string& GetID();
    static SSorterDescriptor GetSorterDescr();

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't belong to any of the listed groups,
    /// an invalid group index will be assigned.
    set<EGroup>  m_SortGroups;
};



///////////////////////////////////////////////////////////////////////////////
/// CDbvarClinicalAsstSorter is a feature sorter used to sort variant features
///        based on variant quality. There are two categories:
///          - high (high or not set)
///          - low 

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDbvarClinicalAsstSorter
    : public CObject
    , public IFeatSorter
{
public:
    enum EGroup {
        eInvalid = -1, ///< maybe it is valid, but not considered
        eBenign = 0,
        ePathogenic,
        eLikelyPathogenic,
        eLikelyBenign,
        eLikelyUnknown,
        eNotTested,
        eOther
    };

    CDbvarClinicalAsstSorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const;

    virtual SSorterDescriptor GetThisDescr() const;

    static const string& GetID();
    static SSorterDescriptor GetSorterDescr();

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't belong to any of the listed groups,
    /// an invalid group index will be assigned.
    set<EGroup>  m_SortGroups;
};




///////////////////////////////////////////////////////////////////////////////
/// CDbvarSamplesetTypeSorter is a feature sorter used to sort variant features
///        based on variant quality. There are two categories:
///          - high (high or not set)
///          - low 

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDbvarSamplesetTypeSorter
    : public CObject
    , public IFeatSorter
{
public:
    enum EGroup {
        eInvalid = -1, ///< maybe it is valid, but not considered
        eCase = 0,
        eControl,
        eNotSet,
        eOther
    };

    CDbvarSamplesetTypeSorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const;

    virtual SSorterDescriptor GetThisDescr() const;

    static const string& GetID();
    static SSorterDescriptor GetSorterDescr();

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't belong to any of the listed groups,
    /// an invalid group index will be assigned.
    set<EGroup>  m_SortGroups;
};

///////////////////////////////////////////////////////////////////////////////
/// CDbvarValidationSorter is a feature sorter used to sort variant features
///        based on variant quality. There are two categories:
///          - high (high or not set)
///          - low 

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CDbvarValidationSorter
    : public CObject
    , public IFeatSorter
{
public:
    enum EGroup {
        eInvalid = -1, ///< maybe it is valid, but not considered
        eSameSample_DiffPlatform = 0,
        eDiffSample_DiffPlatform,
        eDiffSample_SamePlatform,
        eSeenInPlubic,
        eNovel,
        eNotAssessed,
        eOther
    };

    CDbvarValidationSorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const;

    virtual SSorterDescriptor GetThisDescr() const;

    static const string& GetID();
    static SSorterDescriptor GetSorterDescr();

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't belong to any of the listed groups,
    /// an invalid group index will be assigned.
    set<EGroup>  m_SortGroups;
};


///////////////////////////////////////////////////////////////////////////////
/// CGRCStatusSorter is a feature sorter used to sort GRC issue features
///        based on resolving status stored in seq-feat::qual.
///        There are two categories:
///          - resolved
///          - unresolved or others

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGRCStatusSorter
    : public CObject
    , public IFeatSorter
{
public:
    enum EGroup {
        eInvalid = -1,
        eResolved = 0,
        eOther
    };

    CGRCStatusSorter(const string& sort_str);

    virtual string GroupIdxToName(int idx) const;

    virtual int GetGroupIdx(const objects::CMappedFeat& feat) const;

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


#endif  // GUI_WIDGET_SEQ_GRAPHICS___DBVARFEATSORTER__HPP
