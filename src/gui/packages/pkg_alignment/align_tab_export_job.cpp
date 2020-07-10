/*  $Id: align_tab_export_job.cpp 37334 2016-12-23 20:41:42Z katargir $
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
* Authors: Roman Katargin
*
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include <objmgr/feat_ci.hpp>

#include <gui/packages/pkg_alignment/align_tab_export_job.hpp>

#include <objmgr/align_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqalign/Spliced_seg.hpp>

#include <algo/align/util/score_builder.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

CAlignTabExportJob::CAlignTabExportJob(const CAlignTabExportParams& params)
: CAppJob("Align Tab Export"), m_Params(params)
{
}


static CSeq_align::TDim sFindAnchorRow(CBioseq_Handle handle, const CSeq_align& align)
{
    vector<CSeq_align::TDim> anchors;
    CSeq_align::TDim num_row = align.CheckNumRows();
    if (num_row == 0) {
        // empty alignment
        NCBI_THROW(CException, eUnknown, "Get empty alignment!");
    }
    CSeq_align::TDim row = 0;
    for (row = 0;  row < num_row;  ++row) {
        CSeq_id_Handle idh = sequence::GetId(align.GetSeq_id(row),
            handle.GetScope(), sequence::eGetId_Best);
        if ( !idh ) {
            idh = sequence::GetId(align.GetSeq_id(row),
                handle.GetScope(), sequence::eGetId_Canonical);
        }
        if ( handle.IsSynonym(idh) ) {
            anchors.push_back(row);
        }
    }

    if (anchors.empty()) {
        /// try a more aggressive matching approach
        for (size_t level = 0;  level <= 5  &&  anchors.empty();  ++level) {
            for (row = 0;  row < num_row;  ++row) {
                CSeq_id_Handle idh = sequence::GetId(align.GetSeq_id(row),
                    handle.GetScope(), sequence::eGetId_Best);
                if ( !idh ) {
                    idh = sequence::GetId(align.GetSeq_id(row),
                        handle.GetScope(), sequence::eGetId_Canonical);
                }
                if ( handle.ContainsSegment(idh, level) ) {
                    anchors.push_back(row);
                }
            }
        }
    }

    _ASSERT( !anchors.empty() );
    if (anchors.empty()) {
        NCBI_THROW(CException, eUnknown,
            "Can find the anchor sequence in the alignment!");
    }

    return anchors[0];
}

static size_t s_CountSegments(const CSeq_align& align)
{
    switch (align.GetSegs().Which()) {
    default:
        return 1;

    case CSeq_align::TSegs::e_Disc:
        return align.GetSegs().GetDisc().Get().size();

    case CSeq_align::TSegs::e_Spliced:
        return align.GetSegs().GetSpliced().GetExons().size();
    }
}

static CFastMutex s_DFLock;
static sequence::CDeflineGenerator s_DefGen; ///< This class seems to be non-thread safe even as a stack var

static
inline void s_GetDefline(CBioseq_Handle bsh, string& defline )
{
    CFastMutexGuard lock(s_DFLock);
    defline = s_DefGen.GenerateDefline(bsh);
}


static string s_ShortenIdLabel(const string& label)
{
    string out_label = label;
    // make title/label shorter in case it is a cSRA alignment on panfs
    if (out_label.length() > 20  &&  out_label.find("\\panfs\\") != string::npos) {
        size_t pos = out_label.find_last_of("\\");
        if (pos != string::npos) {
            out_label = out_label.substr(pos + 1);
            if (out_label.length() > 20) {
                pos = out_label.rfind('.');
                if (pos != string::npos  &&  pos > 0) {
                    pos = out_label.rfind('.', pos - 1);
                    if (pos != string::npos) {
                        out_label = out_label.substr(pos + 1);
                    }
                }
            }
        }
    }
    return out_label;
}

static string s_RemovePanfsBasePath(const string& label)
{
    string out_label = label;
    // make title/label shorter in case it is a cSRA alignment on panfs
    size_t pos = out_label.find("\\panfs\\");
    if (pos != string::npos) {
        pos = out_label.find("\\", pos + 7);
        if (pos != string::npos) {
            out_label = out_label.substr(pos);
        }
    }
    return out_label;
}

static map<string, string> sGenerateFields(CBioseq_Handle handle, const CSeq_align& align, CSeq_align::TDim anchorRow)
{
    map<string, string> fieldData;

    const CSeq_align::TDim num_rows_limit = 10;
    ///
    /// first, format our seq-ids
    ///
    vector<bool> strands;
    vector<string> ids;
    CSeq_align::TDim num_rows = align.CheckNumRows();
    string curr_text;
    {{
        for(CSeq_align::TDim i = 0;  i < num_rows  &&  i < num_rows_limit;  ++i ){
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( align.GetSeq_id(i) );
            CSeq_id_Handle idh_best = sequence::GetId(idh, handle.GetScope(), sequence::eGetId_Best);
            if( idh_best ){
                idh = idh_best;
            }

            if ( !curr_text.empty() ) {
                curr_text += " x ";
            }
            CConstRef<CSeq_id> seq_id(idh.GetSeqIdOrNull());
            string label;
            if (!seq_id) {
                label = "UNK";
            } else {
                idh.GetSeqId()->GetLabel(&label, CSeq_id::eContent);
            }
            ids.push_back(label);
            curr_text += s_ShortenIdLabel(label);
            try {
                strands.push_back(align.GetSeqStrand(i) == eNa_strand_plus);
            } catch (CException&) {
                // ignore the exception
            }
        }
    }}

    if (num_rows > ids.size()) {
        curr_text += " x ... [total ";
        curr_text += NStr::IntToString(num_rows, NStr::fWithCommas);
        curr_text += "]";
    }

    fieldData["Alignment"] = curr_text;

    if (num_rows > ids.size()) { // early escape?
        return fieldData;
    }

    bool is_protein = false;
    if (ids.size() == 2  &&  num_rows == 2  &&  anchorRow >= 0  &&  anchorRow < 2) {
        TSeqRange range = align.GetSeqRange(anchorRow);
        curr_text = ids[anchorRow] + " (";
        curr_text += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
        curr_text += "..";
        curr_text += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);
        curr_text += ")";
        fieldData["Anchor"] = curr_text;

        range = align.GetSeqRange(1 - anchorRow);
        //t_title = s_ShortenIdLabel(ids[1 - anchorRow]);
        curr_text = s_RemovePanfsBasePath(ids[1 - anchorRow]) + " (";
        curr_text += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
        curr_text += "..";
        curr_text += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);
        curr_text += ")";
        fieldData["Query"] = curr_text;

        CBioseq_Handle bsh = handle.GetScope().GetBioseqHandle(align.GetSeq_id(1 - anchorRow));
        if (bsh) {
            if (bsh.IsProtein()) {
                is_protein = true;
            }
            s_GetDefline(bsh, curr_text);
        }
    }

    /// only report strand for pair-wise alignments
    if (!is_protein  &&  num_rows == 2  &&  strands.size() == 2) {
        fieldData["Strand"] = (strands[0] == strands[1]) ? "forward" : "reverse";
    }

    string tag_name;
    ///
    /// next, add a remark about the total aligned range
    ///
    CScoreBuilder builder;
    // new method
    TSeqPos align_length = builder.GetAlignLength(align);
    tag_name = "Aligned ";
    tag_name += is_protein ? "residues" : "bases";

    fieldData[tag_name] = NStr::IntToString(align_length, NStr::fWithCommas);

    size_t segs = s_CountSegments(align);
    fieldData["Segments"] = NStr::SizetToString(segs, NStr::fWithCommas);

    char buf[255];
    double coverage = -1.0;
    if ( !align.GetNamedScore(CSeq_align::eScore_PercentCoverage, coverage) ){
        try {
            coverage = builder.GetPercentCoverage( handle.GetScope(), align );
        } catch (CException&) {
            // ignore
        }
    }
    if (coverage >= 0.0) {
        if (coverage < 100.0  &&  coverage > 99.9) {
            curr_text = "99.9+";
        } else {
            sprintf(buf, "%2.1f", coverage);
            curr_text = buf;
        }
        curr_text += "%";
        fieldData["Coverage"] = curr_text;
    }

    if (align_length < 1000000) {
        //int identities = builder.GetIdentityCount(*m_Scope, align);
        //int mismatches = builder.GetMismatchCount(*m_Scope, align);
        int identities = -1;
        int mismatches = -1;
        if (!align.GetNamedScore(CSeq_align::eScore_IdentityCount, identities)  ||
            !align.GetNamedScore(CSeq_align::eScore_MismatchCount, mismatches)) {
                try {
                    builder.GetMismatchCount(handle.GetScope(), align, identities, mismatches);
                } catch (CException&) {
                    // ignore
                }
        }

        if (identities >= 0) {
            double identity = 0.0;
            if ( !align.GetNamedScore(CSeq_align::eScore_PercentIdentity, identity) ) {
                identity = identities * 100.0 / align_length;
            }
            if (identity < 100.0  &&  identity > 99.9) {
                curr_text = "99.9+";
            } else {
                sprintf(buf, "%2.1f", identity);
                curr_text = buf;
            }
            curr_text += "%";
            fieldData["Identity"] = curr_text;
        } 
        if (mismatches >= 0) {
            fieldData["Mismatches"] = NStr::NumericToString(mismatches, NStr::fWithCommas);
        }
    }

    try {
        int gap_count  = builder.GetGapCount(align);
        fieldData["Gaps"] = NStr::IntToString(gap_count, NStr::fWithCommas);
    } catch (CException&) {
        // ignore
    }

//    if (at_p != (TSeqPos)-1) {
//        fieldData["Position"] = NStr::UIntToString(at_p + 1, NStr::fWithCommas);
//    }

    return fieldData;
}

IAppJob::EJobState CAlignTabExportJob::Run()
{
    string err_msg;

    try {
		SConstScopedObject seqLoc = m_Params.GetObject();
		const CSeq_loc* loc =
			dynamic_cast<const CSeq_loc*> (seqLoc.object.GetPointer());
		CScope& scope = seqLoc.scope.GetObject();

        CBioseq_Handle handle = scope.GetBioseqHandle(*loc);

        CRange<TSeqPos> range = loc->GetTotalRange();
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Align);
        CSeqUtils::SetAnnot(sel, m_Params.GetAlignmentName());
        CSeqUtils::SetResolveDepth(sel, true, -1);

        CNcbiOfstream ostr(m_Params.GetFileName().fn_str());
        CCSVExporter exporter(ostr, '\t', '"');

        vector<string> fields = m_Params.GetFields();
        ITERATE(vector<string>, it, fields)
            exporter.Field(*it);
        exporter.NewRow();

        for (CAlign_CI align_iter(handle, range, sel);  align_iter;  ++align_iter) {
            if (IsCanceled())
                return eCanceled;

            const CSeq_align& align = *align_iter;
            CSeq_align::TDim anchorRow = sFindAnchorRow(handle, align);
            map<string, string> fieldData = sGenerateFields(handle, align, anchorRow);

            ITERATE(vector<string>, it, fields)
                exporter.Field(fieldData[*it]);
            exporter.NewRow();
        }
	}
    catch (CException& e) {
        err_msg = "Failed to save file:\n";
        err_msg += e.GetMsg();
    }

    if (err_msg.empty())  {
        LOG_POST(Info << "CAlignTabExportJob::Run()  Finished  " << m_Descr);
        return eCompleted;
    } else {
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }
}

END_NCBI_SCOPE
