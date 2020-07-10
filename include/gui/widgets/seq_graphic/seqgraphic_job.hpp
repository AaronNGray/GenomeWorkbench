#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_JOB__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_JOB__HPP

/*  $Id: seqgraphic_job.hpp 44110 2019-10-28 21:45:38Z shkeda $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *
 */



#include <gui/gui.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/objutils/density_map.hpp>
#include <gui/opengl/gltypes.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicJob -- the base class of seqgraphic job for handling the job
/// status such as reporting the progress and returning the result.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqGraphicJob
    : public CJobCancelable
    , public ISeqTaskProgressCallback
{
public:
    typedef int     TJobToken;

    /// List of annotations with the corresponding titles
    typedef map<string, string> TAnnotNameTitleMap;

    CSeqGraphicJob(const string& desc = kEmptyStr);

    virtual ~CSeqGraphicJob(){};

    void SetToken(TJobToken token);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

    /// @name ISeqTaskProgressCallback implementation. 
    /// @{
    virtual void SetTaskName(const string& name);
    virtual void SetTaskCompleted(int completed);
    virtual void AddTaskCompleted(int delta);
    virtual void SetTaskTotal(int total);
    virtual bool StopRequested() const;
    /// @}

protected:
    /// method truly doing the job.
    virtual EJobState    x_Execute() = 0;

    /// @name Job results and status.
    /// @{
    CRef<CAppJobError>    m_Error;
    CRef<CObject>         m_Result;
    string                m_Desc;            ///< Job description or name
    /// @}

    TJobToken    m_Token;        ///< Job token recognizable by job listener

private:
    CFastMutex            m_Mutex;           ///< mutex for guarding state access
    int                   m_TaskTotal;       ///< Total amount of task for a job
    int                   m_TaskCompleted;   ///< The amount of task finished
    string                m_TaskName;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGAnnotJob  -- Job based on SAnnotSelector for loading data using
/// ObjectManager.
///

BEGIN_SCOPE(objects);
    class CMappedGraph;
END_SCOPE(objects);

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAnnotJob : public CSeqGraphicJob
{
public:
    typedef struct { double min; double max; } TAxisLimits;

    struct SGraphObject
    {
        SGraphObject(TSeqPos start, TSeqPos stop, float window,
            CHistogramGlyph::TMap::accum_functor* func,
            const string& desc, const string& annot_name)
            : m_Map(start, stop, window, func, std::numeric_limits<CHistogramGlyph::TDataType>::min())
            , m_Max(std::numeric_limits<double>::min())
            , m_Min(std::numeric_limits<double>::max())
            , m_Desc(desc)
            , m_AnnotName(annot_name)
        {
            m_Map.SetMax(m_Max);
            m_Map.SetMin(m_Min);
        }

        CHistogramGlyph::TMap m_Map;
        double m_Max;
        double m_Min;
        string m_Desc;
        string m_AnnotName;
    };

    CSGAnnotJob (const string& desc, objects::CBioseq_Handle handle,
        const objects::SAnnotSelector& sel, const TSeqRange& range);

    void SetGraphLevel(int level);
    int  GetGraphLevel() const;

    objects::CScope& GetScope(void) const;

protected:
    IAppJob::EJobState x_LoadCoverageGraph(CSeqGlyph::TObjects& glyphs,
        int level, TModelUnit window, const string& title, bool fixed_scale, const TAxisLimits& y_limits);

    IAppJob::EJobState x_CreateHistFromGraph(CSeqGlyph::TObjects& glyphs,
        const objects::SAnnotSelector& sel, TModelUnit window, bool fixed_scale, const TAxisLimits& y_limits);

    template <typename TGraphType>
    void x_AddGraphToMap(CDensityMap<float>& the_map,
        const objects::CMappedGraph& gr,
        const TGraphType& graph,
        double& g_max, double& g_min,
        bool fixed_scale);
    
    void x_AddGraphObject(map<string, SGraphObject>& d_maps,
                          const objects::CMappedGraph& gr,
                          TModelUnit window,
                          bool fixed_scale);

    /// Get a name for a graph.
    /// We should favor title over annotation name.
    string x_GetGraphName(const objects::CMappedGraph& gr) const;
    
    /// Check the graph type.
    /// @return true only if the given graph is a percentile graph
    /// and not a 100 percentile. 
    /// A percentile graph is indicated in seq-annot::desc asn
    /// as a user-object:
    /// Seq-annot ::= {
    ///   desc {
    ///     user {
    ///       type str "AnnotationTrack",
    ///       data {
    ///         {
    ///           label str "StatisticsType",
    ///           data str "Percentiles"
    ///         }
    ///       }
    ///     }
    ///   },
    /// And the specific percentile is indicated in seq-graph::comment
    ///   data graph {
    ///     {
    ///       title "Feature density graph",
    ///       comment "90%",
    ///       ...
    ///     }
    ///   }
    bool x_PercentileButNotMax(const objects::CMappedGraph& gr) const;

protected:
    objects::CBioseq_Handle m_Handle;           ///< target sequence
    objects::SAnnotSelector m_Sel;              ///< our annotation selector
    TSeqRange               m_Range;            ///< target range
    int                     m_GraphLevel;       ///< coverate graph level
};


///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicJob inline methods
///
inline
void CSeqGraphicJob::SetToken(TJobToken token)
{   m_Token = token; }

///////////////////////////////////////////////////////////////////////////////
/// CSGAnnotJob inline methods
///
inline
objects::CScope& CSGAnnotJob::GetScope(void) const
{   return m_Handle.GetScope(); }

inline
void CSGAnnotJob::SetGraphLevel(int level)
{ m_GraphLevel = level; }

inline
int CSGAnnotJob::GetGraphLevel() const
{ return m_GraphLevel; }


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_JOB__HPP
