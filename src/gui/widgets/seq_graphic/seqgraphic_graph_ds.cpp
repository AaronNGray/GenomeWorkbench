/*  $Id: seqgraphic_graph_ds.cpp 41588 2018-08-28 14:39:07Z shkeda $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/wig_graph.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

#include <gui/objutils/utils.hpp>
// for graph job
#include <objmgr/seq_table_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/table_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <gui/widgets/seq_graphic/graph_utils.hpp>

#include <gui/opengl/glpoint.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CSGGraphDS
CSGGraphDS::CSGGraphDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
    , m_SeqTable(false)
{}


void CSGGraphDS::LoadSeqGraphs(const string& annotName,
                               objects::SAnnotSelector& sel,
                               CRenderingContext& ctx,
                               bool fixed_scale, 
                               const TAxisLimits& y_limits,
                               CConstRef<CSeqGraphicConfig> config)
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CRef<CSGSeqGraphJob> job(new CSGSeqGraphJob("Graph", m_Handle, annotName,
        sel, ctx.GetVisSeqRange(), ctx.GetScale(), fixed_scale, y_limits,
        config,  m_SeqTable));
    job->SetGraphCacheKey(m_GraphCacheKey);
    x_LaunchJob(*job);
}


void CSGGraphDS::GetAnnotNames(objects::SAnnotSelector& sel,
                               const TSeqRange& range,
                               TAnnotNameTitleMap& names, bool seq_table) const
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CSGSeqGraphJob::GetAnnotNames(m_Handle, range, sel, names, seq_table);
}

void CSGGraphDS::GetSeqtableAnnots(objects::SAnnotSelector& sel,
                                   const TSeqRange& range,
                                   TAnnotNameTitleMap& names) const
{
    CSeqUtils::SetResolveDepth(sel, m_Adaptive, m_Depth);
    CSGSeqGraphJob::GetSeqtableAnnots(m_Handle, range, sel, names);
}


///////////////////////////////////////////////////////////////////////////////
/// CSGGraphDSType

ISGDataSource* CSGGraphDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSGGraphDS(object.scope.GetObject(), id);
}


string CSGGraphDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_graph_ds_type");
    return sid;
}


string CSGGraphDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View Graph Data Source Type");
    return slabel;
}


bool CSGGraphDSType::IsSharable() const
{
    return false;
}


///////////////////////////////////////////////////////////////////////////////
/// CSGSeqGraphJob

void CSGSeqGraphJob::GetAnnotNames(const objects::CBioseq_Handle& handle,
                                     const TSeqRange& range,
                                     objects::SAnnotSelector& sel,
                                     TAnnotNameTitleMap& names, bool seq_table)
{
    sel.SetCollectNames();
    if (seq_table) { /// search seq-table
        CAnnotTypes_CI annot_it(CSeq_annot::C_Data::e_Seq_table, handle,
            TSeqRange::GetWhole(), eNa_strand_unknown, &sel);
        ITERATE (CAnnotTypes_CI::TAnnotNames, iter, annot_it.GetAnnotNames()) {
            if (iter->IsNamed()) {
                if (iter->GetName().find("@@") == string::npos) {
                    names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
                }
            } else {
                names.insert(TAnnotNameTitleMap::value_type(
                    CSeqUtils::GetUnnamedAnnot(), ""));
            }
        }
    } else {
//        LOG_POST(Trace << "Creating CGraph_CI");
        CGraph_CI graph_iter(handle, range, sel);
        ITERATE (CGraph_CI::TAnnotNames, iter, graph_iter.GetAnnotNames()) {
//            LOG_POST(Trace << "checking graph_iter");
            if (iter->IsNamed()) {
//                LOG_POST(Trace << "it is named: " << iter->GetName());
                // IMPORTANT: exclude any alignment pileup graph
                // That is used for BAM/cSRA pileup display in alignment track
                if (iter->GetName().find("@@") == string::npos  &&
                    iter->GetName().find("pileup graphs") == string::npos) {
//                    LOG_POST(Trace << "name was added)");
                    names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
                }
            } else {
//                LOG_POST(Trace << "it is unnamed: " << CSeqUtils::GetUnnamedAnnot());
                names.insert(TAnnotNameTitleMap::value_type(
                    CSeqUtils::GetUnnamedAnnot(), ""));
            }
        }
    }
}


void CSGSeqGraphJob::GetSeqtableAnnots(const objects::CBioseq_Handle& handle,
                                       const TSeqRange& range,
                                       objects::SAnnotSelector& sel,
                                       TAnnotNameTitleMap& names)
{
    CConstRef<CSeq_loc> loc =
        handle.GetRangeSeq_loc(range.GetFrom(), range.GetTo());
    CAnnot_CI it(handle.GetScope(), *loc, sel);

    for (; it; ++it) {
        const CSeq_annot_Handle& annot = *it;
        bool valid_graph = false;
        if (annot.Seq_annot_CanGetDesc()) {
            const CSeq_annot::TDesc::Tdata& descs = annot.Seq_annot_GetDesc().Get();
            CSeq_annot::TDesc::Tdata::const_iterator d_iter = descs.begin();
            for (; d_iter != descs.end(); ++d_iter) {
                if ((*d_iter)->IsUser()) {
                    CConstRef<CUser_field> type_f = (*d_iter)->GetUser().GetFieldRef("track type");
                    if (type_f  &&  type_f->GetData().IsStr()  &&
                        type_f->GetData().GetStr() == "graph") {
                        valid_graph = true;
                    }
                }
            }
        }
        if (!valid_graph && annot.IsSeq_table()) {
            CTableFieldHandle<double> col_val("values");
            CTableFieldHandle<int> col_pos(CSeqTable_column_info::eField_id_location_from);
            CTableFieldHandle<int> col_span("span");
            try {
                valid_graph = col_span.IsSet(annot, 0)
                    && col_pos.IsSet(annot, 0)
                    && col_val.IsSet(annot, 0);
            } catch (const exception&) {
                // not a valid graph
            }
        }
        if (valid_graph) {
            if (annot.IsNamed()) {
                if (it->GetName().find("@@") == string::npos) {
                    names.insert(TAnnotNameTitleMap::value_type(it->GetName(), ""));
                }
            } else {
                names.insert(TAnnotNameTitleMap::value_type(
                    CSeqUtils::GetUnnamedAnnot(), ""));
            }
        }
    }
}


IAppJob::EJobState CSGSeqGraphJob::x_Execute()
{
    try {
        if (!m_GraphCacheKey.empty()) {
            auto data = CGraphCache<CWigGraph>::GetInstance().GetData(m_GraphCacheKey);
            if (data) {
                SetTaskName("Loading graphs...");
                CHistogramGlyph::TMap the_map(m_Range.GetFrom(),
                                              m_Range.GetTo(),
                                              (float)m_Window,
                                              new CHistogramGlyph::max_func);
                data->GetData(the_map);
                CSGJobResult* result = new CSGJobResult();
                m_Result.Reset(result);
                bool is_empty = the_map.GetMax() == the_map.GetMin() && the_map.GetMax() == 0.;
                if (!is_empty) {
                    auto hist = Ref(new CHistogramGlyph(the_map, m_AnnotName));
                    hist->SetAnnotName(m_AnnotName);
                    hist->SetFixedScale(m_FixedScale);
                    hist->SetAxisMaxFixed(max<double>(the_map.GetMax(), m_YLimits.max));
                    hist->SetAxisMinFixed(min<double>(the_map.GetMin(), m_YLimits.min));
                    result->m_ObjectList.emplace_back(hist.GetPointer());
                }
                result->m_Token = m_Token;
                SetTaskCompleted(1);
                return eCompleted;
            }
        }
        
        CRef<CSeq_loc> seq_loc =
            m_Handle.GetRangeSeq_loc(0, m_Handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac).size());

        
        set<int> levels;
        CGraphUtils::CalcGraphLevels(m_AnnotName, GetScope(), *seq_loc, levels);
        m_GraphLevel = CGraphUtils::GetNearestLevel(levels, m_Window);

        // simulate a data loading exception
        // NCBI_USER_THROW("Simulated problem");

        if (GetGraphLevel() > 0) {
            return x_LoadGraphFromCoverageGraph();
        }

        if (m_SeqTable) {
            return x_LoadSeqTable();
        }

        return x_LoadGraph();
    }
    catch(const CException &err) {
        m_Error.Reset(new CAppJobError(err.GetMsg()));
    }
    return eFailed;
}


enum EValueColType {
    eVal_Byte,
    eVal_Int,
    eVal_Real,
    eVal_Invalid
};


class CPosMapper : public CObject
{
public:
    CPosMapper(TSeqPos src_f, TSeqPos src_t, TSeqPos target_f, bool reversed)
        : m_SrcFrom(src_f)
        , m_SrcTo(src_t)
        , m_TargetFrom(target_f)
        , m_Reversed(reversed)
    {}

    void Map(int& from, int& to)
    {
        if (m_Reversed) {
            from = m_SrcTo + m_TargetFrom - from;
            to = m_SrcTo + m_TargetFrom - to;
            swap(from, to);
        } else {
            from += m_TargetFrom - m_SrcFrom;
            to += m_TargetFrom - m_SrcFrom;
        }
    }
    bool IsReversed() const
    {
        return m_Reversed;
    }

private:
    TSeqPos m_SrcFrom;
    TSeqPos m_SrcTo;
    TSeqPos m_TargetFrom;
    bool    m_Reversed;
};

class CSeqTableReader
{
public:
    CSeqTableReader(CSeq_annot_Handle& annot) :
        m_annot(annot)
        , m_col_val_real("values")
        , m_col_val_int("values")
        , m_col_pos(CSeqTable_column_info::eField_id_location_from)
        , m_col_span("span")
        , m_val_type(eVal_Invalid)
        , m_val_step(1.0)
        , m_val_min(0.0)
        , m_mapper(0)
        , m_GraphStyle("histogram")
        , m_InterpolationMethod(CHistogramData::eInterpolation_None)
    {
    }
    bool Init()
    {

        // check if the given seq-table indeed contains graph data
        bool valid_graph = false;
        if (m_annot.Seq_annot_CanGetDesc()) {
            const CSeq_annot::TDesc::Tdata& descs =
                m_annot.Seq_annot_GetDesc().Get();
            CSeq_annot::TDesc::Tdata::const_iterator d_iter = descs.begin();
            for (; d_iter != descs.end(); ++d_iter) {
                if (!(*d_iter)->IsUser())
                    continue;
                const CUser_object& uo = (*d_iter)->GetUser();
                if (!valid_graph) {
                    CConstRef<CUser_field> type_f = uo.GetFieldRef("track type");
                    if (type_f  &&  type_f->GetData().IsStr())
                        valid_graph = type_f->GetData().GetStr() == "graph";
                    if (valid_graph) {
                        type_f = uo.GetFieldRef("style");
                        if (type_f  &&  type_f->GetData().IsStr())
                            m_GraphStyle = type_f->GetData().GetStr();

                        type_f = uo.GetFieldRef("interpolation");
                        if (type_f  &&  type_f->GetData().IsStr()) {
                            const string& s = type_f->GetData().GetStr();
                            if (NStr::EqualNocase(s, "linear"))
                                m_InterpolationMethod = CHistogramData::eInterpolation_Linear;
                            else if (NStr::EqualNocase(s, "piecewise"))
                                m_InterpolationMethod = CHistogramData::eInterpolation_Piecewise;
                        }
                        break;
                    }
                }
            }
        }
        if (!valid_graph && m_annot.IsSeq_table()) {
            valid_graph = m_col_span.IsSet(m_annot, 0)
                && m_col_pos.IsSet(m_annot, 0)
                && m_col_val_real.IsSet(m_annot, 0);
        }

        if (!valid_graph) {
            ERR_POST(Error << "Invalid seq table");
            return false;
        }

        // There is no way to tell the value column is
        // stored as BYTE, INTEGER or REAL. Only way to
        // find out is try and see.
            {
                double val_r = 0.0;
                int val_i = 0;
                try {
                    if (m_col_val_real.TryGet(m_annot, 0, val_r)) {
                        m_val_type = eVal_Real;
                    }
                } catch (CException&) {
                }

                if (m_val_type == eVal_Invalid) {
                    try {
                        if (m_col_val_int.TryGet(m_annot, 0, val_i)) {
                            m_val_type = eVal_Int;
                        }
                    } catch (CException&) {
                    }
                }
                if (m_val_type == eVal_Invalid) {
                    try {
                        if (m_col_val_int.TryGet(m_annot, 0, m_byte_vals)) {
                            m_val_type = eVal_Byte;
                        }
                    } catch (CException&) {
                    }
                }
            }

        if (m_val_type == eVal_Invalid) {
            ERR_POST(Error << "Can't load the value column");
            return false;
        }

        /// value_step and value_min serve as 'a' and 'b' as stored
        /// in seq-graph. If they are not set, use the default values.
        CTableFieldHandle<double> col_step("value_step");
        CTableFieldHandle<double> col_min("value_min");
        col_step.TryGet(m_annot, 0, m_val_step);
        col_min.TryGet(m_annot, 0, m_val_min);
        return true;
    }

    const string& GetGraphStyle() const
    {
        return m_GraphStyle;
    }

    const CHistogramData::EInterpolationMethod GetInterpolationMethod() const
    {
        return m_InterpolationMethod;
    }

    double GetValue(int row)
    {
        if (m_val_type == eVal_Invalid)
            throw runtime_error("SeqTable is not initialized");
        int r = row;
        if (m_mapper && m_mapper->IsReversed())
            r = (m_annot.GetSeq_tableNumRows() - 1) - row;
        double value = 0.0;
        if (m_val_type == eVal_Real) {
            m_col_val_real.TryGet(m_annot, r, value);
        } else if (m_val_type == eVal_Int) {
            int val_i = 0;
            m_col_val_int.TryGet(m_annot, r, val_i);
            value = (double)val_i;
        } else {
            _ASSERT(m_byte_vals.size() > (size_t)row);
            value = (double)m_byte_vals[r];
        }
        return value * m_val_step + m_val_min;
    }
    bool GetRange(int row, TSeqRange& range)
    {
        int r = row;
        if (m_mapper && m_mapper->IsReversed()) {
            r = (m_annot.GetSeq_tableNumRows() - 1) - row;
        }
        int from;
        if (!m_col_pos.TryGet(m_annot, r, from))
            return false;
         int span = 1;
         m_col_span.TryGet(m_annot, r, span);
         int to = from + span - 1;
         if (m_mapper) {
             m_mapper->Map(from, to);
         }
         range.Set(from, to);
         return true;
    }
    void SetMapper(CPosMapper* mapper)
    {
        m_mapper.Reset(mapper);
    }
private:
    CSeq_annot_Handle m_annot;
    CTableFieldHandle<double> m_col_val_real;
    CTableFieldHandle<int> m_col_val_int;
    CTableFieldHandle<int> m_col_pos;
    CTableFieldHandle<int> m_col_span;

    EValueColType m_val_type;
    vector<char> m_byte_vals;
    double m_val_step;
    double m_val_min;
    CRef<CPosMapper> m_mapper;

    string m_GraphStyle;
    CHistogramData::EInterpolationMethod m_InterpolationMethod;
};

IAppJob::EJobState CSGSeqGraphJob::x_LoadSeqTable()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);

    CSeq_table_CI it(m_Handle,  m_Range, m_Sel);
    if ( !it ) return eCompleted;

    string name = CSeqUtils::GetAnnotName(it.GetAnnot());
    typedef map<string, SGraphObject> TDMaps;
    TDMaps d_maps;

    map<string, string> graph_styles;

    for (; it; ++it) {
        CSeq_annot_Handle annot = it.GetAnnot();

        CSeqTableReader table_reader(annot);
        if (!table_reader.Init())
            continue;
        if (it.IsMapped()) {
            const CSeq_loc& loc1 = it.GetOriginalLocation();
            const CSeq_loc& loc2 = it.GetMappedLocation();
            TSeqRange range = loc1.GetTotalRange();
            CRef<CPosMapper> mapper(new CPosMapper(range.GetFrom(), range.GetTo(),
                loc2.GetTotalRange().GetFrom(), loc1.GetStrand() != loc2.GetStrand()));
            table_reader.SetMapper(mapper.Release());
        }

        // Separate and show graph in a separate track based on seq-annot::desc::title
        // CSeqUtils::GetAnnotName(annot) returns title if title is set.
        string g_title = CSeqUtils::GetAnnotName(annot);
        string g_name = CSeqUtils::GetUnnamedAnnot();
        if (annot.IsNamed()) {
            g_name = annot.GetName();
        }
        TDMaps::iterator iter = d_maps.find(g_title);
        if (iter == d_maps.end()) {
            string g_desc = CSeqUtils::GetAnnotComment(annot);
            iter = d_maps.insert(
                TDMaps::value_type(
                    g_title,
                    SGraphObject(
                        m_Range.GetFrom(),
                        m_Range.GetTo(),
                        (float)m_Window,
                        new CHistogramGlyph::max_func,
                        g_desc, g_name))).first;

            if (!table_reader.GetGraphStyle().empty()) {
                graph_styles[g_title] = table_reader.GetGraphStyle();
            }
        }

        SGraphObject& g_obj = iter->second;
        CHistogramGlyph::TMap* max_data = &g_obj.m_Map;

        size_t rows = annot.GetSeq_tableNumRows();

        TSeqPos from = m_Range.GetFrom();
        TSeqPos to = m_Range.GetTo();

        TSeqRange curr_range;

        if (!table_reader.GetRange(0, curr_range))
            continue;
        if (curr_range.GetFrom() > to)
            continue;
        if (!table_reader.GetRange(rows - 1, curr_range))
            continue;
        if (curr_range.GetTo() < from)
            continue;
        // find the start row
        /* --------------- */
        int row = 0;
        int r_start = 0;
        int r_end = rows - 1;
        int pos = 0;
        do {
            row = (r_start + r_end) / 2;
            TSeqRange r;
            table_reader.GetRange(row, r);
            pos = r.GetFrom();
            if ((TSeqPos)pos < from )
                r_start = row;
            else
                r_end = row;
        } while ((TSeqPos)pos != from  &&  r_start < r_end - 1);
        row = (r_start + r_end) / 2 ;

        int first_row = row;
        for (; (size_t)row < rows; ++row) {
            TSeqRange curr_range;
            if (table_reader.GetRange(row, curr_range)) {
                if (curr_range.GetFrom() > m_Range.GetTo())
                    break;
                double value = table_reader.GetValue(row);
                max_data->AddRange(curr_range, (float)value);
            }
        }
        int last_row = min(row, (int)rows - 1);

        if (table_reader.GetInterpolationMethod() != CHistogramData::eInterpolation_None) {
            TModelPoint lb;
            TModelPoint rb;
            TSeqRange r;
            if (table_reader.GetRange(first_row, r)) {
                lb.Init(r.GetTo(), table_reader.GetValue(first_row));
            }
            if (table_reader.GetRange(last_row, r)) {
                rb.Init(r.GetFrom(), table_reader.GetValue(last_row));
            }
            max_data->SetInterpolationMethod(table_reader.GetInterpolationMethod());
            max_data->InitDataPoints(lb, rb);
        }
    }

    CSeqGlyph::TObjects tmp;
    NON_CONST_ITERATE (TDMaps, iter, d_maps) {
        SGraphObject& g_obj = iter->second;
        CHistogramGlyph *hist = new CHistogramGlyph(g_obj.m_Map, iter->first);
        CRef<CSeqGlyph> fref(hist);
        hist->SetDesc(g_obj.m_Desc);
        hist->SetAnnotName(g_obj.m_AnnotName);
        hist->SetAxisMaxFixed(max(g_obj.m_Max, m_YLimits.max));
        hist->SetAxisMinFixed(min(g_obj.m_Min, m_YLimits.min));
        hist->SetFixedScale(m_FixedScale);
        hist->SetConfig(*m_Config);
        // if graph style is default, see if the one from seq_table can be applied
        if (hist->GetHistParams()->m_Type == CHistParams::eDefaultType && graph_styles.count(iter->first) != 0)
         try {
            hist->GetHistParams()->m_Type = CHistParams::TypeStrToValue(graph_styles[iter->first]);
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
        tmp.push_back(fref);
    }

    result->m_ObjectList.swap(tmp);
    result->m_Token = m_Token;

    return eCompleted;
}


IAppJob::EJobState CSGSeqGraphJob::x_LoadGraph()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);
    SetTaskName("Loading graphs...");
    CSeqGlyph::TObjects glyphs;
    EJobState state =
        x_CreateHistFromGraph(glyphs, m_Sel, m_Window, m_FixedScale, m_YLimits);
    if (state == eCompleted  &&  !glyphs.empty()) {
        result->m_ObjectList.swap(glyphs);
    }
    result->m_Token = m_Token;
    return state;
}


IAppJob::EJobState CSGSeqGraphJob::x_LoadGraphFromCoverageGraph()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);
    SetTaskName("Loading graphs...");

    CSeqGlyph::TObjects glyphs;
    EJobState state =
        x_LoadCoverageGraph(glyphs, GetGraphLevel(), m_Window, "", m_FixedScale, m_YLimits);
    if (state == eCompleted  &&  !glyphs.empty()) {
        result->m_ObjectList.swap(glyphs);
    }
    return state;
}


END_NCBI_SCOPE
