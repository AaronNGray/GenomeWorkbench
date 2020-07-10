#ifndef GUI_WIDGETS_SEQ_GRAPHIC___AGGREGATE_FEATURE_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___AGGREGATE_FEATURE_TRACK__HPP

/*  $Id: aggregate_feature_track.hpp 44424 2019-12-17 16:14:32Z filippov $
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
* Authors:  Vladislav Evgeniev
*
* File Description: Defines a track, combining a set of features into a single track. The list of features is defined in the registry and identified by the subkey.
*
*/

#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAggregateFeatureTrack - combines a set of features into a single track. The list of features is defined in the registry and identified by the subkey.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAggregateFeatureTrack :
    public CFeatureTrack
{
    friend class CAggregateFeatureTrackFactory;

public:
    typedef std::vector<objects::CSeqFeatData::E_Choice>    TTypeVec;
    typedef std::vector<objects::CSeqFeatData::ESubtype>    TSubtypeVec;
    typedef std::map<std::string, objects::CSeqFeatData::E_Choice > TTypeMap;
    
public:
    CAggregateFeatureTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx);
    
    static TTypeVec GetTypeList(const std::string &subkey);
    static TSubtypeVec GetSubtypeList(const std::string &subkey);
    static vector<string> GetSubtypeListAsString(const std::string &subkey);
    
    void SetFeatTypes(const TTypeVec &types);
    void SetFeatSubtypes(const TSubtypeVec &subtypes);

    /// @name CLayoutTrack public methods overloading/implementation.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    virtual void GetFeatSubtypes(set<objects::CSeqFeatData::ESubtype> &subtypes) const;

protected:
    /// Reads the string vector for the specified subkey from the registry
    static void x_GetSubkeyStringVec(const string &subkey, const string &field, vector<string> &values);
    /// Initializes the feature name to type map
    static TTypeMap x_GetFeatTypeMap();

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CAggregateFeatureTrack(const CAggregateFeatureTrack&);
    CAggregateFeatureTrack& operator=(const CAggregateFeatureTrack&);
    /// @}

private:
    static CTrackTypeInfo   m_TypeInfo;
    set<objects::CSeqFeatData::ESubtype> m_subtypes;
};

///////////////////////////////////////////////////////////////////////////////
///   CAggregateFeatureTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAggregateFeatureTrackFactory :
    public CFeatureTrackFactory
{
public:
    CAggregateFeatureTrackFactory();

    /// Create a biological region track based on the input object and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual void GetMatchedAnnots(
        const TAnnotMetaDataList& src_annots,
        const ILayoutTrackFactory::SExtraParams& params,
        TAnnotNameTitleMap& out_annots) const;

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    {
        return GetTypeInfo();
    }

    static const CTrackTypeInfo& GetTypeInfo()
    {
        return CAggregateFeatureTrack::m_TypeInfo;
    }

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___AGGREGATE_FEATURE_TRACK__HPP
