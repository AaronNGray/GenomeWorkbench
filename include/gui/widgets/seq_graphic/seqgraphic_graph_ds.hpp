#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_DS__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_DS__HPP

/*  $Id: seqgraphic_graph_ds.hpp 41371 2018-07-18 15:24:05Z shkeda $
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

#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/utils/extension.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CSGGraphDS
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGGraphDS : public CSGGenBankDS
{
public:

    typedef CSGAnnotJob::TAxisLimits TAxisLimits;
    CSGGraphDS(objects::CScope& scope, const objects::CSeq_id& id);

    void SetSeqTable(bool f);
    bool IsSeqTable() const;

    void LoadSeqGraphs(
        const string& annotName,
        objects::SAnnotSelector& sel,
        CRenderingContext& ctx, 
        bool fixed_scale,
        const TAxisLimits& y_limits,
        CConstRef<CSeqGraphicConfig> config);

    void GetAnnotNames(objects::SAnnotSelector& sel,
        const TSeqRange& range, TAnnotNameTitleMap& names,
        bool seq_table = false) const;

    void GetSeqtableAnnots(objects::SAnnotSelector& sel,
        const TSeqRange& range,
        TAnnotNameTitleMap& names) const;

    void SetGraphCacheKey(const string& graph_cache_key);
    
private:
    bool m_SeqTable;  ///< create graph from seq-table
    string      m_GraphCacheKey;

};


///////////////////////////////////////////////////////////////////////////////
/// CSGGraphDSType
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGGraphDSType :
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
/// CSGSeqGraphJob
///

class CSGSeqGraphJob : public CSGAnnotJob
{
public:

    CSGSeqGraphJob(const string& desc,
        objects::CBioseq_Handle handle,
        const string& annotName,
        const objects::SAnnotSelector& sel, const TSeqRange& range,
        TModelUnit window, bool fixed_scale, const TAxisLimits& y_limits, 
        CConstRef<CSeqGraphicConfig> config,
        bool seq_table = false)
    : CSGAnnotJob(desc, handle, sel, range)
    , m_AnnotName(annotName)
    , m_Window(window)
    , m_FixedScale(fixed_scale)
    , m_YLimits(y_limits)
    , m_Config(config)
    , m_SeqTable(seq_table)
    {}

    static void GetAnnotNames(const objects::CBioseq_Handle& handle,
        const TSeqRange& range, objects::SAnnotSelector& sel,
        TAnnotNameTitleMap& names, bool seq_table = false);

    static void GetSeqtableAnnots(const objects::CBioseq_Handle& handle,
        const TSeqRange& range, objects::SAnnotSelector& sel,
        TAnnotNameTitleMap& names);

    void SetGraphCacheKey(const string& graph_cache_key);
    
protected:
    virtual EJobState x_Execute();

private:
    IAppJob::EJobState x_LoadSeqTable();

    IAppJob::EJobState x_LoadGraph();

    IAppJob::EJobState x_LoadGraphFromCoverageGraph();

private:
    string      m_AnnotName;
    TModelUnit  m_Window;       ///< current window for smear bars
    bool        m_FixedScale;   ///< use global scale or dynamic scale
    TAxisLimits m_YLimits;
    CConstRef<CSeqGraphicConfig> m_Config;
    bool        m_SeqTable;     ///< create graph from seq-table
    string      m_GraphCacheKey;

};



///////////////////////////////////////////////////////////////////////////////
///    inline methods
inline
void CSGGraphDS::SetSeqTable(bool f)
{
    m_SeqTable = f;
}

inline
bool CSGGraphDS::IsSeqTable() const
{
    return m_SeqTable;
}

inline
void CSGGraphDS::SetGraphCacheKey(const string& graph_cache_key)
{
    m_GraphCacheKey = graph_cache_key;
}

inline
void CSGSeqGraphJob::SetGraphCacheKey(const string& graph_cache_key)
{
    m_GraphCacheKey = graph_cache_key;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_DS__HPP
