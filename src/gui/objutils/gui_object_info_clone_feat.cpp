/*  $Id: gui_object_info_clone_feat.cpp 43655 2019-08-13 19:11:44Z katargir $
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
*     Gui Object Info class specifically for clone features.
*
*/

#include <ncbi_pch.hpp>
#include <corelib/tempstr.hpp>
#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/Clone_seq_set.hpp>
#include <objects/seqfeat/Clone_seq.hpp>
#include <gui/utils/string_utils.hpp>
#include <gui/objutils/tooltip.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoClone : public CGuiObjectInfoSeq_feat
{
public:
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const;
private:
    enum TAccInfo {
        eAcc_GSS    = 0,    // GSS sequences
        eAcc_Nuc,           // Nucleotide sequences(e.g.HTG records)
        eAcc_Trace          // Trace
    };
    typedef map<string, TAccInfo> TAccessionInfoMap;
    string x_FormatAccessionLinks(const ITooltipFormatter& tooltip, const vector<string> &strings, const TAccessionInfoMap &acc_info, bool occurences = true) const;
    void x_IdentifyAccession(const CSeq_id &id, const string &id_str, TAccessionInfoMap &acc_info) const;
private:
    typedef map<CSeq_id::EAccessionInfo, string> TBaseURLforAccession;
    const static string     m_BaseURLs[];
};

const string CGuiObjectInfoClone::m_BaseURLs[] = {
    "/nucgss/",
    "/nuccore/",
    "/Traces/trace.cgi?&cmd=retrieve&retrieve=Submit&val="
};

void CGuiObjectInfoClone::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());

    const CSeqFeatData::TClone& clone = m_Feat->GetData().GetClone();
    if (m_Location) {
        tooltip.AddRow("Total length:", NStr::IntToString(m_Location->GetTotalRange().GetLength(), NStr::fWithCommas));

        string range = NStr::IntToString(
            m_Location->GetStart(eExtreme_Positional) + 1, NStr::fWithCommas);
        range += "..";
        range += NStr::IntToString(
            m_Location->GetStop(eExtreme_Positional) + 1, NStr::fWithCommas);
        tooltip.AddRow("Total range:", range);
    }

    if (m_Feat->IsSetComment()  &&  m_Feat->GetComment() != t_title) {
        tooltip.AddRow("Comment:", m_Feat->GetComment());
    }

    if (clone.IsSetPlacement_method()) {
        const CEnumeratedTypeValues* type_val =
            CClone_ref::GetTypeInfo_enum_EPlacement_method();
        if (type_val) {
            string method = type_val->FindName(clone.GetPlacement_method(), true);
            if ( !method.empty() ) {
                tooltip.AddRow("Placement method:", method);
            }
        }
    }
    if (clone.IsSetConcordant()) {
        tooltip.AddRow("Clone placement:", clone.GetConcordant() ? "concordant" : "discordant");
    }
    if (clone.IsSetUnique()) {
        tooltip.AddRow("Clone placement:", clone.GetUnique() ? "unique" : "multiple");
    }
    if (clone.IsSetClone_seq()) {
        typedef map<int, vector<string> > TEndMap;
        TEndMap end_map;
        TAccessionInfoMap info_map;
        const CClone_ref::TClone_seq::Tdata& ends = clone.GetClone_seq().Get();
        ITERATE (CClone_ref::TClone_seq::Tdata, e_iter, ends) {
            const CClone_seq& seq = **e_iter;
            if (seq.CanGetSeq()  &&  seq.CanGetSupport()) {
                const CSeq_id* id = seq.GetSeq().GetId();
                if (id) {
                    string id_str;
                    CSeq_id_Handle idh = sequence::GetId(*id, *m_Scope);
                    if (idh) {
                        CSeq_id_Handle idhb =
                            sequence::GetId(idh, *m_Scope, sequence::eGetId_Best);
                        if( idhb ){
                            idh = idhb;
                        }
                    }
                    if (idh) {
                        CConstRef<CSeq_id> seq_id_ref = idh.GetSeqId();
                        seq_id_ref->GetLabel( &id_str, CSeq_id::eContent );
                        // Identify the accession
                        if (info_map.end() == info_map.find(id_str))
                            x_IdentifyAccession(*seq_id_ref, id_str, info_map);
                    } else {
                        id->GetLabel(&id_str);
                        // Identify the accession
                        if (info_map.end() == info_map.find(id_str))
                            x_IdentifyAccession(*id, id_str, info_map);
                    }
                    end_map[seq.GetSupport()].push_back(id_str);
                }
            }
        }

        if (end_map.count(CClone_seq::eSupport_prototype) > 0) {
            tooltip.AddRow("Prototype:", x_FormatAccessionLinks(tooltip, end_map[CClone_seq::eSupport_prototype], info_map, false), 200, true);
        }

        if (end_map.count(CClone_seq::eSupport_supporting) > 0) {
            tooltip.StartRow();
            tooltip.AddTagCol("Supporting (" +
                NStr::NumericToString(end_map[CClone_seq::eSupport_supporting].size()) +
                "):");
            tooltip.AddValueCol(x_FormatAccessionLinks(tooltip, end_map[CClone_seq::eSupport_supporting], info_map));
            tooltip.FinishRow();
        }

        if (end_map.count(CClone_seq::eSupport_non_supporting) > 0) {
            tooltip.StartRow();
            tooltip.AddTagCol("Non-supporting (" +
                NStr::NumericToString(end_map[CClone_seq::eSupport_non_supporting].size()) +
                "):");
            tooltip.AddValueCol(x_FormatAccessionLinks(tooltip, end_map[CClone_seq::eSupport_non_supporting], info_map));
            tooltip.FinishRow();
        }

        if (end_map.count(CClone_seq::eSupport_supports_other) > 0) {
            tooltip.StartRow();
            tooltip.AddTagCol("Supports other (" +
                NStr::NumericToString(end_map[CClone_seq::eSupport_supports_other].size()) +
                "):");
            tooltip.AddValueCol(x_FormatAccessionLinks(tooltip, end_map[CClone_seq::eSupport_supports_other], info_map));
            tooltip.FinishRow();
        }
    }
}

string CGuiObjectInfoClone::x_FormatAccessionLinks(const ITooltipFormatter& tooltip, const vector<string> &strings, const TAccessionInfoMap &acc_info, bool occurences) const
{
    string out_str;
    bool first = true;
    typedef map<string, unsigned> TStrCountMap;
    TStrCountMap count;
    ITERATE(vector<string>, itStr, strings) {
        if (0 == count.count(*itStr))
            count[*itStr] = 1;
        else
            ++count[*itStr];
    }
    ITERATE(TStrCountMap, iter, count) {
        if (first) {
            first = false;
        }
        else {
            out_str += ',';
        }
        string link_name(iter->first);
        if (occurences) {
            link_name += '(';
            link_name += NStr::NumericToString(iter->second);
            link_name += ')';
        }
        TAccessionInfoMap::const_iterator itAccInfo = acc_info.find(iter->first);
        if (acc_info.end() == itAccInfo) {
            out_str += link_name;
            continue;
        }
        _ASSERT((unsigned)(itAccInfo->second) < (sizeof(m_BaseURLs) / sizeof(m_BaseURLs[0])));
        string url(m_BaseURLs[itAccInfo->second]);
        CTempString accession(itAccInfo->first);
        if (eAcc_Trace == itAccInfo->second) {
            NStr::TrimPrefixInPlace(accession, "ti:", NStr::eNocase);
        }
        url += accession;
        out_str += tooltip.CreateNcbiLink(link_name, url);
    }
    return out_str;
}

void CGuiObjectInfoClone::x_IdentifyAccession(const CSeq_id &id, const string &id_str, TAccessionInfoMap &acc_info) const
{
    size_t info = id.IdentifyAccession();
    if (!(info & CSeq_id::fAcc_nuc))
        return;

    switch (info & CSeq_id::eAcc_division_mask) {
        case CSeq_id::eAcc_gss:
            acc_info[id_str] = eAcc_GSS;
            return;
        case CSeq_id::eAcc_htgs:
            acc_info[id_str] = eAcc_Nuc;
            return;
        default:
            break;
    }

    switch (info & CSeq_id::eAcc_type_mask) {
        case CSeq_id::e_General: {
            acc_info[id_str] = eAcc_Trace;
            return;
        }
        case CSeq_id::e_Embl:
            acc_info[id_str] = eAcc_Nuc;
            return;
        default:
            break;
    }
}

void CGuiObjectInfoClone::GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const
{
}

END_NCBI_SCOPE

