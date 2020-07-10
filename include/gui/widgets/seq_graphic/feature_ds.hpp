#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_DS__HPP

/*  $Id: feature_ds.hpp 44658 2020-02-18 16:04:29Z shkeda $
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
 * File Description:
 *
 */

#include <corelib/ncbimtx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/feature_enums.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGFeatureDS : public CSGGenBankDS
{
public:
    CSGFeatureDS(objects::CScope& scope, const objects::CSeq_id& id);

    void LoadFeatures(objects::SAnnotSelector& sel,
                      const TSeqRange& range, TModelUnit window,
                      TJobToken token = -1, int max_feat = -1,
                      ELinkedFeatDisplay LinkedFeatDisplay = ELinkedFeatDisplay::eLFD_Default);

    void LoadFeatures(const TSeqRange& range, TModelUnit window,
        TJobToken token, const TFeatBatchJobRequests& requests);

    void CalcFeatHistogram(const CSeqGlyph::TObjects& objs,
        const TSeqRange& range, TModelUnit window,
        TJobToken token = -1);

    /// load features for gene model.
    /// This method will either load landmark features or all main features
    /// based on both overview flag and number of genes presented given
    /// the range. The main features include genes, Rnas, CDSs, and exons.
    /// @param lm_feat_sel annot selector for loading landmark features (genes)
    /// @param m_feat_sel annot selector for loading main features.
    /// @param lm_token token for loading landmark features job.
    /// @param main_token token for loading main features job.
    /// @param style the merging style for RNA and cds features.
    /// @param landmark_feat show landmark features if suitable.
    /// @param min_overview_feat minimal feature number to be in overview model.
    /// @param range the data range.
    /// @param window the window scale bases per pixel for creating densitymap.
    /// @param max_feat the maximum features allowed.
    void LoadMainFeatures(
        objects::SAnnotSelector& lm_feat_sel,
        objects::SAnnotSelector& m_feat_sel,
        TJobToken lm_token, TJobToken main_token,
        int merge_style, bool landmark_feat,
        int min_overview_feat, const TSeqRange& range,
        TModelUnit window, int max_feat,
        bool show_histogram, int highlight_mode);

    void GetAnnotNames(objects::SAnnotSelector& sel,
        const TSeqRange& range, TAnnotNameTitleMap& names) const;

    void GetAnnotNames_var(objects::SAnnotSelector& sel,
        const TSeqRange& range, TAnnotNameTitleMap& names) const;

    vector<string> GetFilters(const string& annot_name,
        const TSeqRange& range) const;

    void SetFilter(const string& filter);
    const string& GetFilter() const;

    void SetSortBy(const string& sortby);
    const string& GetSortBy() const;

    void SetGraphCacheKey(const string& graph_cache_key);
    void SetRemoteDataType(const string& data_type);

    bool IsBigBed() const;
    bool IsVcfTabix() const;

private:

    string m_Filter;
    string m_SortBy;
    string m_GraphCacheKey;
    string m_RmtDataType;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDSType
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGFeatureDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// create an instance of the layout track type using default settings.
    virtual ISGDataSource* CreateDS(SConstScopedObject& object) const;

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    /// check if the data source can be shared.
    virtual bool IsSharable() const;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGFeatureDS inline method
///
inline
void CSGFeatureDS::SetFilter(const string& filter)
{ m_Filter = filter; }

inline
const string& CSGFeatureDS::GetFilter() const
{ return m_Filter; }

inline
void CSGFeatureDS::SetSortBy(const string& sortby)
{ m_SortBy = sortby; }

inline
const string& CSGFeatureDS::GetSortBy() const
{ return m_SortBy; }

inline
void CSGFeatureDS::SetGraphCacheKey(const string& graph_cache_key)
{
    m_GraphCacheKey = graph_cache_key;
}

inline
void CSGFeatureDS::SetRemoteDataType(const string& data_type)
{
    m_RmtDataType = data_type;
}

inline
bool CSGFeatureDS::IsBigBed() const
{
    return NStr::EqualNocase(m_RmtDataType, "bigBed");
}

inline
bool CSGFeatureDS::IsVcfTabix() const
{
    return NStr::EqualNocase(m_RmtDataType, "vcfTabix");
}


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_DS__HPP */
