/*  $Id: table_data_aln_summary.cpp 32240 2015-01-22 21:53:03Z katargir $
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
 * Authors: Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/table_data_aln_summary.hpp>

#include <algo/align/util/score_builder.hpp>
#include <objmgr/util/create_defline.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seq/Seq_annot.hpp>

//#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableDataAlnSummary::CTableDataAlnSummary(TConstScopedObjects& objects)
{
    _ASSERT( !objects.empty() );
    m_Scope = objects.front().scope;
    ITERATE (TConstScopedObjects, iter, objects) {
        const CObject& obj = *iter->object;
        const type_info& type = typeid(obj);

        if (typeid(CSeq_align) == type) {
            const CSeq_align& align = dynamic_cast<const CSeq_align&>(obj);
            m_Aligns.push_back(CConstRef<CSeq_align>(&align));

        } else if (typeid(CSeq_align_set) == type) {
            const CSeq_align_set& align_set = dynamic_cast<const CSeq_align_set&>(obj);
            std::copy(align_set.Get().begin(), align_set.Get().end(), std::back_inserter(m_Aligns));

        } else if (typeid(CSeq_annot) == type) {
            const CSeq_annot& annot = dynamic_cast<const CSeq_annot&>(obj);
            if (annot.IsAlign()) {
                const CSeq_annot::TData::TAlign& aligns = annot.GetData().GetAlign();
                std::copy(aligns.begin(), aligns.end(), std::back_inserter(m_Aligns));
            }        
        }
    }

    int prev_total = 0;
    ITERATE( vector< CConstRef<CSeq_align> >, aix, m_Aligns ){
        int num_rows  = (*aix)->CheckNumRows();
        int total = prev_total + num_rows -1;
        prev_total = total;

        m_RowsUpTo.push_back( total );
    }
}

void CTableDataAlnSummary::LoadData()
{
    // data is already loaded in the ctor, nothing else to do here
}

static ITableData::ColumnType s_ColTypes[CTableDataAlnSummary::eMaxColNum] = {
    ITableData::kString,// query accession
    ITableData::kString,// subject accession
    ITableData::kInt,   // query start
    ITableData::kInt,   // query stop
    ITableData::kString,// query strand
    ITableData::kInt,   // subject start
    ITableData::kInt,   // subject stop
    ITableData::kString,// subject strand
    ITableData::kReal,  // percent identity
    ITableData::kReal,  // percent coverage
    ITableData::kInt,   // mumber of mismatches
    ITableData::kInt,   // number of gaps
    ITableData::kInt,   // number of gapped bases
    ITableData::kInt,   // alignment score
    ITableData::kReal,  // blast-style 'e_value'
    ITableData::kString,// query defline
    ITableData::kString // subject defline
};


static const string s_ColNames[CTableDataAlnSummary::eMaxColNum] = {
    "Query",
    "Subject",
    "Query Start",
    "Query Stop",
    "Query Strand",
    "Subject Start",
    "Subject Stop",
    "Subject Strand",
    "Identity",
    "Coverage",
    "Mismatches",
    "Gaps",
    "Gap Bases",
    "Score",
    "E-Value",
    "Query Defline",
    "Subject Defline"
};


ITableData::ColumnType CTableDataAlnSummary::GetColumnType(size_t col) const
{
    if (col < CTableDataAlnSummary::eMaxColNum)
        return s_ColTypes[col];

    return ITableData::kNone;
}


string CTableDataAlnSummary::GetColumnLabel(size_t col) const
{
    if (col < CTableDataAlnSummary::eMaxColNum)
        return s_ColNames[col];

    return kEmptyStr;
}


size_t CTableDataAlnSummary::GetRowsCount() const
{
    return m_RowsUpTo.empty() ? 0 : m_RowsUpTo[ m_RowsUpTo.size() -1 ];
}


size_t CTableDataAlnSummary::GetColsCount() const
{
    return (size_t)CTableDataAlnSummary::eMaxColNum;
}


void CTableDataAlnSummary::GetStringValue(size_t row, size_t col, string& value) const
{
    value.resize(0);
    if( row >= GetRowsCount() ) return;

    const CAlnSummary& summary = x_GetAlnSummary(row);
    EColType type = (EColType)(col);
    switch (type) {
        case eQuery:
            value = summary.m_Query;
            break;
        case eSubject:
            value = summary.m_Subject;
            break;
        case eQStrand:
            value = x_StrandToStr(summary.m_QStrand);
            break;
        case eSStrand:
            value = x_StrandToStr(summary.m_SStrand);
            break;
        case eQDefline:
            value = summary.m_QDefline;
            break;
        case eSDefline:
            value = summary.m_SDefline;
            break;
        default:
            {{
                ITableData::ColumnType type = GetColumnType(col);
                if (type == ITableData::kInt)
                    value = NStr::NumericToString(GetIntValue(row, col));
                else if (type == ITableData::kReal)
                    value = NStr::DoubleToString(GetRealValue(row, col));
            }}
            break;
    }

}

long CTableDataAlnSummary::GetIntValue(size_t row, size_t col) const
{
    long value = 0;
    if( row >= GetRowsCount() ) return value;

    const CAlnSummary& summary = x_GetAlnSummary(row);
    EColType type = (EColType)(col);
    switch (type) {
        case eQStart:
            value = (long)summary.m_QStart;
            break;
        case eQStop:
            value = (long)summary.m_QStop;
            break;
        case eSStart:
            value = (long)summary.m_SStart;
            break;
        case eSStop:
            value = (long)summary.m_SStop;
            break;
        case eMismatches:
            value = (long)summary.m_Mismatches;
            break;
        case eGaps:
            value = (long)summary.m_Gaps;
            break;
        case eGappedBases:
            value = (long)summary.m_GappedBases;
            break;
        case eScore:
            value = (long)summary.m_Score;
            break;
        default:
            break;
    }

    return value;
}


double CTableDataAlnSummary::GetRealValue(size_t row, size_t col) const
{
    double value = 0.0;
    if( row >= GetRowsCount() ) return value;

    const CAlnSummary& summary = x_GetAlnSummary(row);
    EColType type = (EColType)(col);
    switch (type) {
        case ePctIdentity:
            value = summary.m_PctIdentity;
            break;
        case ePctCoverage:
            value = summary.m_PctCoverage;
            break;
        case eEValue:
            value = summary.m_EValue;
            break;
        default:
            break;
    }

    return value;
}


SConstScopedObject CTableDataAlnSummary::GetObjectValue(size_t row, size_t col) const
{
    SConstScopedObject value;
    return value;
}


SConstScopedObject CTableDataAlnSummary::GetRowObject(size_t row) const
{
    SConstScopedObject value;
    if( row >= GetRowsCount() ) return value;

    value.scope = m_Scope;

    int alrow = 0; 
    while( alrow < m_RowsUpTo.size() ){
        if( row < m_RowsUpTo[alrow] ){
            break;
        }
        alrow++;
    }

    value.object = m_Aligns[alrow];

    return value;
}


const CTableDataAlnSummary::CAlnSummary&
CTableDataAlnSummary::x_GetAlnSummary( size_t row ) const
{
    TAlnSummary::const_iterator iter = m_CachedSummary.find(row);
    if (iter != m_CachedSummary.end()) {
        return *iter->second;
    }

    CScope& scope = const_cast<CScope&>(*m_Scope);
    CRef<CAlnSummary> sum( new CAlnSummary() );
    m_CachedSummary[row] = sum;

    int alrow = 0; 
    while( alrow < m_RowsUpTo.size() ){
        if( row < m_RowsUpTo[alrow] ){
            break;
        }
        alrow++;
    }

    int seqrow = row - (alrow == 0 ? 0 : m_RowsUpTo[alrow-1]);

    // update cached summary
    const CSeq_align& align = *m_Aligns[alrow];

    // retrieve alignment label
    //CLabel::GetLabel(align, &sum->m_Label, CLabel::eContent);

    // retrieve sequence accession
    CSeq_id_Handle idh = sequence::GetId(align.GetSeq_id(0), scope, sequence::eGetId_Best);
    if (idh) {
        CLabel::GetLabel(*idh.GetSeqId(), &sum->m_Query, CLabel::eContent, &scope);
    } else {
        CLabel::GetLabel(align.GetSeq_id(0), &sum->m_Query, CLabel::eContent, &scope);
    }
    idh = sequence::GetId(align.GetSeq_id(seqrow+1), scope, sequence::eGetId_Best);
    if (idh) {
        CLabel::GetLabel(*idh.GetSeqId(), &sum->m_Subject, CLabel::eContent, &scope);
    } else {
        CLabel::GetLabel(align.GetSeq_id(seqrow+1), &sum->m_Subject, CLabel::eContent, &scope);
    }

    // get start and stop
    sum->m_QStart = align.GetSeqStart(0) + 1;
    sum->m_QStop = align.GetSeqStop(0) + 1;
    sum->m_SStart = align.GetSeqStart(seqrow+1) + 1;
    sum->m_SStop = align.GetSeqStop(seqrow+1) + 1;

    // get strands
    sum->m_QStrand = align.GetSeqStrand(0);
    sum->m_SStrand = align.GetSeqStrand(seqrow+1);

    CScoreBuilder builder;
    TSeqPos align_length = builder.GetAlignLength(align);

    // get percent coverage
    double coverage = 0.0;
    int mismatches = 0;
    double identity = 0.0;
    size_t gaps = 0;
    size_t gap_bases = 0;

    try {
        if ( !align.GetNamedScore(CSeq_align::eScore_PercentCoverage, coverage) ) {
            coverage = builder.GetPercentCoverage(scope, align);
        }

        // get percent identity and number of mismatches
        if (align_length < 1000000) {
            int identities = 0;
            if ( !align.GetNamedScore(CSeq_align::eScore_IdentityCount, identities) ||
                !align.GetNamedScore(CSeq_align::eScore_MismatchCount, mismatches) ) {
                    builder.GetMismatchCount(scope, align, identities, mismatches);
            }

            if ( !align.GetNamedScore(CSeq_align::eScore_PercentIdentity, identity) ) {
                identity = identities * 100.0 / align_length;
            }
        }

        // get number of gaps
        gaps = builder.GetGapCount(align);

        // get number of gapped bases
        gap_bases = builder.GetGapBaseCount(align);

    } catch (CException&) {
        // ignore
    }

    sum->m_PctCoverage = coverage;
    sum->m_PctIdentity = identity;
    sum->m_Mismatches = mismatches;
    sum->m_Gaps = gaps;
    sum->m_GappedBases = gap_bases;

    // score
    {
        int score = 0;
        align.GetNamedScore(CSeq_align::eScore_Score, score);
        sum->m_Score = score;
    }

    // evalue
    {
        double e_value = 0.0;
        align.GetNamedScore(CSeq_align::eScore_EValue, e_value);
        sum->m_EValue = e_value;
    }

    // deflines
    {
        sequence::CDeflineGenerator gen;
        CBioseq_Handle bsh = scope.GetBioseqHandle(align.GetSeq_id(0));
        if (bsh) {
            sum->m_QDefline = gen.GenerateDefline(bsh);
        } else {
            sum->m_QDefline = sum->m_Query;
        }
        bsh = scope.GetBioseqHandle(align.GetSeq_id(1));
        if (bsh) {
            sum->m_SDefline = gen.GenerateDefline(bsh);
        } else {
            sum->m_SDefline = sum->m_Subject;
        }

    }

    return *sum;
}


string CTableDataAlnSummary::x_StrandToStr(ENa_strand strand) const
{
    switch (strand) {
    case eNa_strand_plus:
        return "+";
    case eNa_strand_minus:
        return "-";
    case eNa_strand_both:
        return "Both";
    case eNa_strand_both_rev:
        return "Both reverse";
    case eNa_strand_other:
        return "Other";
    case eNa_strand_unknown:
    default:
        return "Unknown";
    }
}


END_NCBI_SCOPE

