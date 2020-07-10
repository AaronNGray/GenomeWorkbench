/*  $Id: trace_data.cpp 18995 2009-03-18 19:28:03Z wuliangs $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <algorithm>

#include <gui/objutils/utils.hpp>
#include <gui/widgets/aln_multiple/trace_data.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/graph_ci.hpp>
#include <objects/seqres/Byte_graph.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////////////////////////////////////////////////////////////////////////////////
/// CTraceDataProxy

CTraceDataProxy::CTraceDataProxy(const CBioseq_Handle& handle, bool b_neg_strand)
:   m_Handle(handle),
    m_bNegativeStrand(b_neg_strand),
    m_Status(eUnknown)
{
    if(m_TitleToType.empty()) {
        m_TitleToType["Phrap Quality"] = CTraceData::eA - 1;
        m_TitleToType["Phred Quality"] = CTraceData::eA - 1;
        m_TitleToType["A-channel Trace Chromatogram"] = CTraceData::eA;
        m_TitleToType["C-channel Trace Chromatogram"] = CTraceData::eC;
        m_TitleToType["T-channel Trace Chromatogram"] = CTraceData::eT;
        m_TitleToType["G-channel Trace Chromatogram"] = CTraceData::eG;
    }
}

bool    CTraceDataProxy::HasData() const
{
    switch(m_Status)    {
    case eNoData: return false;
    case eHasData: return true;
    case eUnknown:  {
        CConstRef<CSeq_id> id = m_Handle.GetSeqId();
        if(id  &&  id->IsGeneral()  &&  id->GetGeneral().GetTag().IsId()  &&
            (id->GetGeneral().GetDb() == "ti"
              ||  id->GetGeneral().GetDb() == "TRACE"))     {
            m_Status = eHasData;
            return true;
        }
        m_Status = eNoData;
    }; break; // case eUnknown
    };
    return false;
}

CTraceData* CTraceDataProxy::LoadData()
{
    CTraceData* p_data = NULL;

    // first - look for CSeq_graph-s with familiar titles
    const CSeq_graph* raw_graphs[5] = { 0, 0, 0, 0, 0 };

    CBioseq_Handle chgr_handle; // empty for now

    CConstRef<CSeq_id> id = m_Handle.GetSeqId();
    if (id  &&  id->IsGeneral()  &&  id->GetGeneral().GetTag().IsId()  &&
        (id->GetGeneral().GetDb() == "ti"  ||  id->GetGeneral().GetDb() == "TRACE"))    {
        string sid = string("gnl|TRACE_CHGR|") + NStr::IntToString(id->GetGeneral().GetTag().GetId());
        CSeq_id trace_chgr_id(sid);

        // load satellite sequence with chromatograms into Object Manager
        chgr_handle = m_Handle.GetScope().GetBioseqHandle(trace_chgr_id);
    }

    // now using CGraph_CI(handle) we are able to iterate quality and chromatogram graphs
    if(chgr_handle) {
        CGraph_CI graph_iter(m_Handle);
        while(graph_iter)   {
            const CSeq_graph& graph = graph_iter->GetOriginalGraph();
            if(graph.CanGetTitle()  &&  graph.CanGetLoc()) {
                string sTitle = graph.GetTitle();

                TTitleToType::iterator it = m_TitleToType.find(sTitle);
                if(it != m_TitleToType.end())  {
                    _ASSERT(graph.GetGraph().Which() == CSeq_graph::C_Graph::e_Byte);

                    if(m_TitleToType.find(sTitle) != m_TitleToType.end())  {
                        int type = it->second;
                        raw_graphs[type + 1] = &graph; // store pointer to graph
                    }
                }
            }
            ++graph_iter;
        }
    }

    if(chgr_handle) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Graph);
        sel.SetSearchExternal(chgr_handle);
        for ( CGraph_CI graph_iter(chgr_handle, sel);  graph_iter;  ++graph_iter ) {
        const CSeq_graph& graph = graph_iter->GetOriginalGraph();
            if(graph.CanGetTitle()  &&  graph.CanGetLoc()) {
                string sTitle = graph.GetTitle();

                TTitleToType::iterator it = m_TitleToType.find(sTitle);
                if(it != m_TitleToType.end())  {
                    _ASSERT(graph.GetGraph().Which() == CSeq_graph::C_Graph::e_Byte);
                    //_TRACE("\nTitle: "<< sTitle);

                    if(m_TitleToType.find(sTitle) != m_TitleToType.end())  {
                        int type = it->second;
                        raw_graphs[type + 1] = &graph; // store pointer to graph
                    }
                }
            }
            //++graph_iter;
        }
   }

    // now check what we've got
    int len = 0, sig_len = 0;

    bool b_conf = raw_graphs[0] != NULL;
    if(b_conf)  {  // if we have confidence graph len = conf len
        const CSeq_graph::C_Graph& gr = raw_graphs[0]->GetGraph();
        const CByte_graph::TValues& values = gr.GetByte().GetValues();
        len = (int) values.size();
    }

    int i = 1;
    for( ; i < 5  &&  raw_graphs[i] == NULL;  i++ )  {};
    bool b_ch = (i < 5);
    if(b_ch)  { // if we have chromatograms
        double A = raw_graphs[i]->GetA();
        const CSeq_graph::C_Graph& gr = raw_graphs[i]->GetGraph();
        const CByte_graph::TValues& values = gr.GetByte().GetValues();
        sig_len = (int) values.size();
        if(len == 0)    {
            len  = (int) (sig_len / A); // seq length of graph
        }
    }

    if(len > 0) { // we have at least one graph
        p_data = new  CTraceData();
        p_data->Init(0, len - 1, sig_len, m_bNegativeStrand);

        if(b_conf)  { // copy confidence values to CTraceData
            const CSeq_graph::C_Graph& gr = raw_graphs[0]->GetGraph();
            const CByte_graph::TValues& values = gr.GetByte().GetValues();
            for( i = 0; i < len; i++ ) {
                p_data->SetConfidence(i, values[i]);
            }
        }
        for( i = 1 ; i < 5  &&  b_ch; i++)  {
            bool b_calc_pos = false;
            if(raw_graphs[i])   {
                const CSeq_graph::C_Graph& gr = raw_graphs[i]->GetGraph();
                const CByte_graph::TValues& values = gr.GetByte().GetValues();
                CTraceData::TSignalValue A = (CTraceData::TSignalValue) raw_graphs[i]->GetA();
                CTraceData::TSignalValue K = A / 255;

                // calculate positions on sequnce for chromatogram samples
                if(! b_calc_pos)    {
                    CTraceData::TPositions& positions = p_data->GetPositions();
                    CTraceData::TFloatSeqPos K_pos =
                                    ((CTraceData::TFloatSeqPos) len) / sig_len;
                    for( int j = 0; j < sig_len; j++ )  {
                        positions[j] = K_pos * j;
                    }
                    b_calc_pos = true;
                }
                // copy chromatogram data
                CTraceData::TValues& data_values = p_data->GetValues((CTraceData::EChannel) (i - 1));
                for( int j = 0; j < sig_len; j++ )  {
                    data_values[j] = K * ((unsigned char) values[j]);
                }
            }
        }
    }
    return p_data;
}

////////////////////////////////////////////////////////////////////////////////
/// CTraceData

void    CTraceData::Init(TSignedSeqPos from, TSignedSeqPos to, int samples, bool negative)
{
    _ASSERT(to >= from  &&  samples >= 0);
    m_From = from;
    m_To = to;
    m_Confs.resize(GetSeqLength());

    m_Positions.resize(samples);
    m_ASig.resize(samples);
    m_CSig.resize(samples);
    m_TSig.resize(samples);
    m_GSig.resize(samples);

    m_bNegative = negative;
}

const CTraceData::TValues&      CTraceData::GetValues(EChannel channel) const
{
    switch(channel)  {
    case eA: return m_ASig;
    case eC: return m_CSig;
    case eT: return m_TSig;
    case eG: return m_GSig;
    }
    _ASSERT(false);
    NCBI_THROW(CException, eUnknown, "unhandled channel in CTraceData");
}

CTraceData::TValues&      CTraceData::GetValues(EChannel channel)
{
    switch(channel)  {
    case eA: return m_ASig;
    case eC: return m_CSig;
    case eT: return m_TSig;
    case eG: return m_GSig;
    }
    _ASSERT(false);
    NCBI_THROW(CException, eUnknown, "unhandled channel in CTraceData");
}

void    CTraceData::CalculateMax()
{
    if(m_Confs.size())  {
        vector<TConfidence>::const_iterator it = max_element(m_Confs.begin(), m_Confs.end());
        m_MaxConfidence = *it;
    } else m_MaxConfidence = 0.0f;

    if(m_ASig.size())   {
        m_MaxA = * max_element(m_ASig.begin(), m_ASig.end());
        m_MaxC = * max_element(m_CSig.begin(), m_CSig.end());
        m_MaxT = * max_element(m_TSig.begin(), m_TSig.end());
        m_MaxG = * max_element(m_GSig.begin(), m_GSig.end());
    } else {
        m_MaxA = m_MaxC = m_MaxT = m_MaxG = 0.0f;
    }
}

CTraceData::TConfidence     CTraceData::GetMaxConfidence() const
{
    return m_MaxConfidence;
}

CTraceData::TSignalValue    CTraceData::GetMax(EChannel channel) const
{
    switch(channel) {
    case eA: return m_MaxA;
    case eC: return m_MaxC;
    case eT: return m_MaxT;
    case eG: return m_MaxG;
    }
    _ASSERT(false);
    NCBI_THROW(CException, eUnknown, "unhandled channel in CTraceData");
}

END_NCBI_SCOPE
