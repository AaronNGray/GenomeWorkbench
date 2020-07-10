/*  $Id: seq_text_ds.cpp 43822 2019-09-05 20:06:29Z katargir $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_text/seq_text_ds.hpp>

#include <gui/objutils/label.hpp>  // TODO
#include <gui/objutils/utils.hpp>

#include <objects/seq/MolInfo.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <objmgr/util/feature.hpp>

#include <gui/objutils/utils.hpp>
#include <objects/seqfeat/Gb_qual.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


CSeqTextDataSource::CSeqTextDataSource(CSeq_entry& sep,
                     CScope& scope)
{
    m_Sep = &sep;
    m_Scope = &scope;

    CSeq_entry_Handle handle = scope.GetSeq_entryHandle(sep);
    CBioseq_CI it(handle);

    m_SubjectHandle = *it;
    m_ID = m_SubjectHandle.GetSeqId();
    CSeq_id_Handle idh = sequence::GetId(m_SubjectHandle, sequence::eGetId_Best);
    if (idh) {
        m_ID = idh.GetSeqId();
    }
    m_Loc.Reset ();

    m_Loc = new CSeq_loc(const_cast<CSeq_id&>(*CBioseq_Handle(*it).
                 GetSeqId()),
                 0,CBioseq_Handle(*it).GetBioseqLength() - 1);
    x_PopulateFeatureIterators();
}


CSeqTextDataSource::CSeqTextDataSource(CBioseq_Handle handle,
                     CScope& scope)
{
    m_Scope = &scope;

    m_SubjectHandle = handle;

    m_ID = handle.GetSeqId();
    CSeq_id_Handle idh = sequence::GetId(handle, sequence::eGetId_Best);
    if (idh) {
        m_ID = idh.GetSeqId();
    }

    m_Loc = new CSeq_loc(const_cast<CSeq_id&>(*CBioseq_Handle(handle).
                   GetSeqId()),
                   0,CBioseq_Handle(handle).GetBioseqLength() - 1);
    x_PopulateFeatureIterators();
}

CSeqTextDataSource::CSeqTextDataSource(CSeq_loc &loc, CScope& scope)
{
    m_Scope = &scope;
    m_SubjectHandle = scope.GetBioseqHandle(*loc.GetId());

    if (!m_SubjectHandle)
        NCBI_THROW(CException, eUnknown, "failed to retrieve sequence");

    m_ID = m_SubjectHandle.GetSeqId();
    CSeq_id_Handle idh = sequence::GetId(m_SubjectHandle, sequence::eGetId_Best);
    if (idh) {
        m_ID = idh.GetSeqId();
    }

    m_Loc = new CSeq_loc();
    m_Loc->Add(loc);
    x_PopulateFeatureIterators();
}


CSeqTextDataSource::~CSeqTextDataSource()
{
}


CScope& CSeqTextDataSource::GetScope()
{
    return m_Scope.GetObject();
}


TSeqPos CSeqTextDataSource::SequencePosToSourcePos (TSeqPos sequence_pos, bool *found_in_source)
{
    bool    found = false;
    bool    in_source = false;
    TSeqPos source_pos = 0;

    if (m_Intervals.size() < 1) {
        return -1;
    }

    ITERATE(TIntervalList, it, m_Intervals) {
        if (sequence_pos >= it->GetSequenceStart() && sequence_pos <= it->GetSequenceStop()) {
            source_pos = it->GetSourceStart() + sequence_pos - it->GetSequenceStart();
            found = true;
            in_source = true;
            break;
        } else if (sequence_pos < it->GetSequenceStart()) {
            source_pos = it->GetSourceStart();
            found = true;
            break;
        }
    }
    if (!found) {
        source_pos = m_Intervals.back().GetSourceStart() + m_Intervals.back().GetLength() - 1;
    }

    if (found_in_source != NULL) {
        *found_in_source = in_source;
    }
    return source_pos;
}


TSeqPos CSeqTextDataSource::SourcePosToSequencePos (TSeqPos source_pos)
{
    TSeqPos sequence_pos = 0;
    bool    found = false;

    ITERATE(TIntervalList, it, m_Intervals) {
        if (source_pos >= it->GetSourceStart() && source_pos <= it->GetSourceStop()) {
            sequence_pos = it->GetSequenceStart() + source_pos - it->GetSourceStart();
            found = true;
        }
    }

    return sequence_pos;
}


void CSeqTextDataSource::GetSeqString (TSeqPos start, TSeqPos stop, string &buffer)
{
    string  seq_piece;

    // initialize buffer to empty
    buffer.erase();

    ITERATE(TIntervalList, it, m_Intervals) {
        if (it->GetSourceStart() > stop) {
            break;
        }
        it->GetSeqString(start, stop, buffer);
    }
}


void InvertCase(string& seq)
{
    for (unsigned int i = 0; i < seq.length(); i++) {
        if (islower (seq[i])) {
            seq[i] = toupper(seq[i]);
        } else {
            seq[i] = tolower(seq[i]);
        }
    }
}


void CSeqTextDataSource::GetSeqData (TSeqPos start, TSeqPos stop, string& buffer, SAnnotSelector *feat_sel, bool showFeatAsLower)
{
    buffer.clear();

    ITERATE(TIntervalList, it, m_Intervals) {
        if (it->GetSourceStart() > stop) {
            break;
        }
        it->GetSeqData(start, stop, buffer, feat_sel /*CSeqFeatData::eSubtype_any*/);
    }

    if (showFeatAsLower) {
        InvertCase(buffer);
    }
}


int CSeqTextDataSource::ChooseBetterSubtype (int subtype1, int subtype2)
{
    if (subtype1 == CSeqFeatData::eSubtype_cdregion || subtype2 == CSeqFeatData::eSubtype_cdregion) {
        return CSeqFeatData::eSubtype_cdregion;
    } else if (subtype1 == CSeqFeatData::eSubtype_mRNA || subtype2 == CSeqFeatData::eSubtype_mRNA) {
        return CSeqFeatData::eSubtype_mRNA;
    } else if (subtype1 == CSeqFeatData::eSubtype_gene || subtype2 == CSeqFeatData::eSubtype_gene) {
        return CSeqFeatData::eSubtype_gene;
    } else if (subtype1 == CSeqFeatData::eSubtype_bad) {
        return subtype2;
    } else if (subtype2 == CSeqFeatData::eSubtype_bad) {
        return subtype1;
    } else if (subtype1 < subtype2) {
        return subtype1;
    } else {
        return subtype2;
    }
}


void
CSeqTextDataSource::GetFeatureData
(TSeqPos start_offset,
 TSeqPos stop_offset,
 CSeqTextConfig *cfg,
 ISeqTextGeometry* pParent,
 CSeqTextDefs::TSubtypeVector &subtypes,
 CSeqTextDefs::TSpliceSiteVector &splice_sites,
 CSeqTextDefs::TVariationGraphVector &variations)
{

    subtypes.clear();
    splice_sites.clear();
    variations.clear();

    // no configuration - leave the feature data blank
    if (cfg == NULL) {
        return;
    }

    // output_range_offset contains the lengths of output data that have already
    // been filled in.  data for a range should be filled in based on the
    // distance from the start of the range plus the output_range_offset
    //TSeqPos output_range_offset = 0;

    int l = (stop_offset - start_offset) + 1;
    splice_sites.reserve(l);
    splice_sites.resize(l, false);
    subtypes.reserve(l); 
    subtypes.resize(l, CSeqFeatData::eSubtype_bad);


    // don't look for features after the end of the data
    stop_offset = min (stop_offset, GetDataLen() - 1);

    // examine each interval in the source data
    NON_CONST_ITERATE(TIntervalList, it, m_Intervals) {
        if (it->GetSourceStart() > stop_offset) {
            break;
        }
        it->GetFeatureData(start_offset, stop_offset, cfg, pParent, subtypes, splice_sites, variations);

    }

}


bool CSeqTextDataSource::IsmRNASequence ()
{
    bool is_mRNA = true;

    for (CSeq_loc_CI seq_loc_it (*m_Loc); seq_loc_it && is_mRNA; ++ seq_loc_it) {
        CBioseq_Handle handle = GetScope().GetBioseqHandle(*seq_loc_it.GetEmbeddingSeq_loc().GetId());
        if (!handle.CanGetInst_Mol() || handle.GetInst_Mol() != CSeq_inst::eMol_rna) {
            is_mRNA = false;
        } else {
            CSeqdesc_CI di (handle, CSeqdesc::e_Molinfo);
            if (!di || di->GetMolinfo().GetBiomol() != CMolInfo::eBiomol_mRNA) {
                is_mRNA = false;
            }
        }
    }
    return is_mRNA;
}


void
CSeqTextDataSource::GetSubtypesForAlternatingExons
(TSeqPos start_offset,
 TSeqPos stop_offset,
 ISeqTextGeometry* pParent,
 CSeqTextDefs::TSubtypeVector &subtypes)
{
    bool         even = false;

    // only do this when sequence is mRNA

    if (!IsmRNASequence ()) return;

    subtypes.clear();
    // this fills in the output range
    //for (int i = 0; i < stop_offset - start_offset + 2; i++) {
        //subtypes.push_back (CSeqFeatData::eSubtype_bad);
    //}
    if (!pParent) return;
    subtypes.reserve((stop_offset - start_offset) + 1);
    subtypes.resize((stop_offset - start_offset) + 1, CSeqFeatData::eSubtype_bad);

    // examine each interval in the source data
    NON_CONST_ITERATE(TIntervalList, it, m_Intervals) {
        if (it->GetSourceStart() > stop_offset) {
            break;
        }
        it->GetSubtypesForAlternatingExons(start_offset, stop_offset, pParent, subtypes, even);
    }

}


void CSeqTextDataSource::GetVariations (TSeqPos start_offset, TSeqPos stop_offset, CSeqTextDefs::TVariationGraphVector &variations)
{
    variations.clear();

    // examine each interval in the source data
    NON_CONST_ITERATE(TIntervalList, it, m_Intervals) {
        if (it->GetSourceStart() > stop_offset) {
            break;
        }
        it->GetVariations(start_offset, stop_offset, variations);
    }

}


void CSeqTextDataSource::RenderFeatureExtras (ISeqTextGeometry* pParent, CGlPane &pane, TSeqPos start_offset, TSeqPos stop_offset)
{
    if (pParent == NULL) return;

    // open pane for drawing
    pane.OpenOrtho();

    // examine each interval in the source data
    NON_CONST_ITERATE(TIntervalList, it, m_Intervals) {
        it->RenderFeatureExtras (pParent, start_offset, stop_offset);
    }

    pane.Close();
}


void
CSeqTextDataSource::GetIntervalBreaks
(TSeqPos start_offset,
 TSeqPos stop_offset,
 CSeqTextDefs::TSeqPosVector &breaks)
{
    bool    is_first = true;
    TSeqPos offset = 0;

    breaks.clear();


    for (CSeq_loc_CI seq_loc_it (*m_Loc); seq_loc_it && offset < stop_offset; ++ seq_loc_it) {
        if (is_first) {
            is_first = false;
        }
        else if (offset >= start_offset)
        {
            breaks.push_back (offset);
        }
        CSeq_loc_CI::TRange seq_range = seq_loc_it.GetRange();
        TSeqPos seq_start = seq_range.GetFrom();
        TSeqPos seq_stop = seq_range.GetTo();
        offset += seq_stop - seq_start + 1;
    }
}


TSeqPos CSeqTextDataSource::GetDataLen()
{
    return sequence::GetLength(*m_Loc, m_Scope);
}

string CSeqTextDataSource::GetTitle ()
{
    string s = "Sequence : ";
    auto_ptr<sequence::CDeflineGenerator> gen(new sequence::CDeflineGenerator());
    s += gen->GenerateDefline(m_SubjectHandle);
    return s;
}


CSeqTextDataSource::TIdRef  CSeqTextDataSource::GetId() const
{
    return m_ID;
}


vector<CConstRef<CSeq_feat> > CSeqTextDataSource::GetFeaturesAtPosition(TSeqPos source_pos)
{
    vector<CConstRef<CSeq_feat> > features;

    const CSeq_id *seq_id = m_Loc->GetId();
    if (!seq_id) return features;

    if (source_pos > GetDataLen() - 1) {
        return features;
    }

#if 1
    NON_CONST_ITERATE(TIntervalList, it, m_Intervals) {
        it->GetFeaturesAtPosition(source_pos, features);
    }
    return features;
#else
    TSeqPos seq_pos = SourcePosToSequencePos(source_pos);
    CBioseq_Handle handle = GetScope().GetBioseqHandle(*seq_id);

    TSeqRange range (seq_pos, seq_pos);
    if (feat_sel) {
        return new CFeat_CI(handle, range, *feat_sel);
    }

    objects::SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    return new CFeat_CI(handle, range, sel);
#endif
}

string CSeqTextDataSource::GetToolTipForSourcePos (TSeqPos source_pos)
{
    string  tooltip_text = "";

    NON_CONST_ITERATE(TIntervalList, it, m_Intervals) {
        it->GetToolTipForPosition(source_pos, tooltip_text);
    }
    return tooltip_text;
}


const CSeq_loc* CSeqTextDataSource::GetLoc() const
{
    return m_Loc.GetNCPointer();
}

int CSeqTextDataSource::FindSequenceFragment (const string& fragment, TSeqPos start_search)
{
    TSeqPos data_len = GetDataLen();
    TSeqPos search_buffer_len;
    TSeqPos end_search;
    string  search_buffer;

    if (fragment.length() > data_len - start_search) {
        return -1;
    }

    search_buffer_len = max ((int) 1000, (int)(3 * fragment.length()));
    search_buffer_len = min (search_buffer_len, data_len - start_search);

    while (start_search < data_len - fragment.length() + 1) {
        end_search = start_search + search_buffer_len;
        GetSeqString (start_search, end_search, search_buffer);
        string::size_type pos = NStr::FindNoCase(search_buffer, fragment);
        if (pos != string::npos) {
            return pos + start_search;
        }
        start_search = end_search - fragment.length() + 1;
    }
    return -1;
}


void CSeqTextDataSource::FindSequenceFragmentList (const string& fragment,
                                                   CSeqTextDefs::TSeqPosVector &locations,
                                                   ICanceled* cancel)
{
    if (cancel->IsCanceled()) {
        return;
    }
    TSeqPos data_len = GetDataLen();
    TSeqPos search_buffer_len;
    TSeqPos start_search = 0, end_search;
    string  search_buffer;

    locations.clear();

    if (fragment.length() > data_len - start_search) {
        return;
    }

    search_buffer_len = max ((int) 1000, (int)(3 * fragment.length()));
    search_buffer_len = min (search_buffer_len, data_len - start_search);

    while (!cancel->IsCanceled() && start_search < data_len - fragment.length() + 1) {
        end_search = start_search + search_buffer_len;
        GetSeqString (start_search, end_search, search_buffer);
        string::size_type pos = NStr::FindNoCase(search_buffer, fragment);
        while (pos != string::npos) {
            locations.push_back (pos + start_search);
            if (end_search > pos + fragment.length()) {
                pos = NStr::FindNoCase(search_buffer, fragment, pos + 1);
            } else {
                pos = string::npos;
            }
        }
        start_search = end_search - fragment.length() + 1;
    }

    if (cancel->IsCanceled()) {
        locations.clear();
    }
}


void CSeqTextDataSource::x_PopulateFeatureIterators()
{
    m_Intervals.clear();
    TSeqPos offset = 0;
    for (CSeq_loc_CI seq_loc_it (*m_Loc); seq_loc_it; ++ seq_loc_it) {
        m_Intervals.push_back(CSeqTextDataSourceInterval(*(seq_loc_it.GetRangeAsSeq_loc()), *m_Scope, offset));
        offset += seq_loc_it.GetRange().GetLength();
    }
}


CSeqTextDataSourceInterval::CSeqTextDataSourceInterval(const objects::CSeq_loc& loc, objects::CScope& scope, TSeqPos offset)
    : m_Feat(scope, loc), m_Offset(offset)
{
    m_Loc.Reset(new CSeq_loc());
    m_Loc->Assign(loc);
    m_Seq = scope.GetBioseqHandle(*(loc.GetId()));
    m_Length = sequence::GetLength(loc, &scope);
    m_Vect = m_Seq.GetSeqVector (CBioseq_Handle::eCoding_Iupac);
}


CSeqTextDataSourceInterval::~CSeqTextDataSourceInterval()
{
}


bool CSeqTextDataSourceInterval::SourcePosToIntervalPos(TSeqPos& pos)const
{
    bool rval = true;
    if (pos < m_Offset) {
        pos = 0;
        rval = false;
    } else if (pos > m_Offset + m_Length - 1) {
        pos = m_Offset + m_Length - 1;
        rval = false;
    } else {
        pos -= m_Offset;
        rval = true;
    }
    return rval;
}


bool CSeqTextDataSourceInterval::SequencePosToIntervalPos(TSeqPos& pos) const
{
    bool rval = true;
    if (pos < GetSequenceStart()) {
        pos = 0;
        rval = false;
    } else if (pos > GetSequenceStop()) {
        pos = GetSequenceStop();
        rval = false;
    } else {
        pos -= GetSequenceStart();
    }
    return rval;
}


bool CSeqTextDataSourceInterval::IntersectingSourceInterval(TSeqPos& src_start, TSeqPos& src_stop) const
{
    bool rval = true;
    if (src_start >= m_Offset + m_Length || src_stop < m_Offset) {
        rval = false;
    }
    SourcePosToIntervalPos(src_start);
    SourcePosToIntervalPos(src_stop);
    return rval;
}


void CSeqTextDataSourceInterval::GetSeqString(TSeqPos src_start, TSeqPos src_stop, string& buffer) const
{
    if (IntersectingSourceInterval(src_start, src_stop)) {
        //CSeqVector vect = m_Seq.GetSeqVector (CBioseq_Handle::eCoding_Iupac);
        string seq_piece;
        m_Vect.GetSeqData (src_start + GetSequenceStart(), src_stop + GetSequenceStart(), seq_piece);
        buffer.append (seq_piece);
    }
}


void CSeqTextDataSourceInterval::GetSeqData (TSeqPos src_start, TSeqPos src_stop, string& buffer, SAnnotSelector *feat_sel) const
    //CSeqFeatData::ESubtype subtype) const
{
    if (IntersectingSourceInterval(src_start, src_stop)) {
        //CSeqVector vect = m_Seq.GetSeqVector (CBioseq_Handle::eCoding_Iupac);
        string seq_piece;
        m_Vect.GetSeqData (src_start + GetSequenceStart(), src_stop + GetSequenceStart() + 1, seq_piece);
//        CFeat_CI feat_it = m_Feat;
        SAnnotSelector sel;
        CFeat_CI feat_it(m_Seq.GetScope(), *m_Loc, feat_sel ? *feat_sel : sel);

        while (feat_it) {
            //if (subtype != CSeqFeatData::eSubtype_any && subtype != feat_it->GetData().GetSubtype()) {
            //    continue;
            //}
            const CSeq_loc& loc = feat_it->GetLocation();

            for(CSeq_loc_CI loc_it(loc); loc_it; ++loc_it) {
                CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                TSeqPos feat_start = feat_range.GetFrom();
                TSeqPos feat_stop = feat_range.GetTo();
                if (feat_stop < GetSequenceStart() || feat_start > GetSequenceStop()) {
                    // if the section of the feature is not in this sequence piece,
                    // don't draw it
                    continue;
                }
                feat_start = max(0, int(feat_start - GetSequenceStart()));
                feat_stop = max(0, int(feat_stop - GetSequenceStart()));

                if (feat_stop < src_start || feat_start > src_stop) {
                    // if the section of the feature is not in the viewed area, don't draw it
                    continue;
                }                

                feat_start = (feat_start < src_start) ? 0 : feat_start - src_start;
                feat_stop = (feat_stop - src_start > seq_piece.size() - 1) ? seq_piece.size() - 1 : feat_stop - src_start;

                while (feat_start <= feat_stop)
                {
                    seq_piece [feat_start] = tolower(seq_piece [feat_start]);
                    feat_start ++;
                }
            }

            ++feat_it;
        }

        buffer.append (seq_piece);
    }
    
}


void CSeqTextDataSourceInterval::GetToolTipForPosition(TSeqPos src_pos, string& tooltip_text)
{
    if (SourcePosToIntervalPos(src_pos)) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();    
        CFeat_CI f(m_Seq, TSeqRange(src_pos + GetSequenceStart(), src_pos + GetSequenceStart()), sel);
        while (f) {
            string feat_title = "";
            CLabel::GetLabel(f->GetOriginalFeature(), &feat_title, 
                    CLabel::eUserTypeAndContent, &m_Seq.GetScope());
            if (!feat_title.empty()) {
                if (!tooltip_text.empty()) {
                    tooltip_text.append ("\n");
                }
                tooltip_text.append (feat_title);
            }
            ++f;
        }
    }
}


void CSeqTextDataSourceInterval::RenderFeatureExtras (ISeqTextGeometry* pParent, TSeqPos start_offset, TSeqPos stop_offset)
{
    if (pParent == NULL) return;

    // if this interval contains locations between start_offset and stop_offset, draw feature extras
    if (IntersectingSourceInterval(start_offset, stop_offset)) {
        CScope * scope = &(m_Seq.GetScope());
        CRef<CSeq_loc> cmp = x_GetSeqLocForInterval(start_offset, stop_offset);
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();    
        CFeat_CI f(*scope, *cmp, sel);
        while (f) {
            pParent->STG_RenderFeatureExtras(*f);
            ++f;
        }
    }

}


void
CSeqTextDataSourceInterval::GetFeatureData
(TSeqPos start_offset,
 TSeqPos stop_offset,
 CSeqTextConfig *cfg,
 ISeqTextGeometry* pParent,
 CSeqTextDefs::TSubtypeVector &subtypes,
 CSeqTextDefs::TSpliceSiteVector &splice_sites,
 CSeqTextDefs::TVariationGraphVector &variations)
{
    if (IntersectingSourceInterval(start_offset, stop_offset)) {
        CScope * scope = &(m_Seq.GetScope());
        CRef<CSeq_loc> cmp = x_GetSeqLocForInterval(start_offset, stop_offset);
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();    
        CFeat_CI f(*scope, *cmp, sel);
        while (f) {
            // feature subtypes
            int subtype = f->GetFeatSubtype();
            if (cfg->GetFeatureColorationChoice() == CSeqTextPaneConfig::eAll && pParent && cfg->GetShow (subtype)) {
                pParent->STG_SetSubtypesForFeature (subtypes, f->GetLocation(), f->GetFeatSubtype(), start_offset, stop_offset);
            }

            if (subtype == CSeqFeatData::eSubtype_cdregion) {
                LookForSpliceJunctions (*f, splice_sites, start_offset, stop_offset);
            }

            if (subtype == CSeqFeatData::eSubtype_variation) {
                x_AddVariationsFromFeature (f->GetOriginalFeature(), f->GetLocation(), variations);
            }
            ++f;
        }
    }

}


void CSeqTextDataSourceInterval::GetVariations (TSeqPos start_offset, TSeqPos stop_offset, CSeqTextDefs::TVariationGraphVector &variations)
{
    if (IntersectingSourceInterval(start_offset, stop_offset)) {
        CRef<CSeq_loc> cmp = x_GetSeqLocForInterval(start_offset, stop_offset);
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_variation);    
        CFeat_CI f(m_Seq.GetScope(), *cmp, sel);
        while (f) {
            x_AddVariationsFromFeature (f->GetOriginalFeature(), f->GetLocation(), variations);
            ++f;
        }
    }
}


void
CSeqTextDataSourceInterval::x_AddVariationsFromFeature
(const CSeq_feat &feat,
 const CSeq_loc  &loc,
 CSeqTextDefs::TVariationGraphVector &variations) const
{
    TSeqPos feat_left = loc.GetStart(eExtreme_Positional);
    TSeqPos feat_right = loc.GetStop(eExtreme_Positional);
    bool    replace_found = false;

    // get contents of sequence at this location, so we know which /replace
    // represents the original text (and we can display the other instead)

    string  seq_piece;
    //CSeqVector vect = m_Seq.GetSeqVector (CBioseq_Handle::eCoding_Iupac);
    m_Vect.GetSeqData (feat_left, feat_right + 1, seq_piece);

    // needed sequence coordinates for getting actual sequence text
    // store source coordinates in variation holder

    feat_left -= GetSequenceStart();
    feat_right -= GetSequenceStart();

    if (feat.CanGetQual()) {
        ITERATE( CSeq_feat::TQual, it, feat.GetQual()) {
            if (!NStr::CompareNocase((*it)->GetQual(),"replace")
                && NStr::CompareNocase((*it)->GetVal(),seq_piece))
            {
                variations.push_back (CSeqTextVariationGraph(feat_left, feat_right, (*it)->GetVal()));
                replace_found = true;
            }
        }
    }
    if (!replace_found) {
        variations.push_back (CSeqTextVariationGraph(feat_left, feat_right, "-"));
    }
}


void
CSeqTextDataSourceInterval::GetSubtypesForAlternatingExons
(TSeqPos start_offset,
 TSeqPos stop_offset,
 ISeqTextGeometry* pParent,
 CSeqTextDefs::TSubtypeVector &subtypes,
 bool& even)
{
    if (!pParent) return;

    if (IntersectingSourceInterval(start_offset, stop_offset)) {
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_exon);    
        CFeat_CI f(m_Seq, TSeqRange(start_offset + GetSequenceStart(), stop_offset + GetSequenceStart()), sel);
        while (f) {
            if (f->GetData().GetSubtype() == CSeqFeatData::eSubtype_exon) {
                TSeqPos feat_stop = f->GetLocation().GetStop(eExtreme_Positional);
                TSeqPos feat_start = f->GetLocation().GetStart(eExtreme_Positional);
                if (feat_stop < GetSequenceStart() || feat_stop - GetSequenceStart() < start_offset) {
                    // just alternate
                    even = !even;
                } else if (feat_start - GetSequenceStart() > stop_offset) {
                    // we're done
                    break;
                } else {
                    // color the portion of this exon in the view range
                    // only color odd exons
                    if (!even) {
                        pParent->STG_SetSubtypesForFeature (subtypes, f->GetLocation(), f->GetFeatSubtype(), start_offset, stop_offset);
                    }
                    even = !even;
                }
            }
            ++f;
        }
    }

}


void CSeqTextDataSourceInterval::GetFeaturesAtPosition(TSeqPos source_pos, vector<CConstRef<CSeq_feat> >& features)
{
    TSeqPos start_offset = source_pos;
    TSeqPos stop_offset = start_offset;
    if (IntersectingSourceInterval(start_offset, stop_offset)) {
        start_offset += GetSequenceStart();
        stop_offset += GetSequenceStart();
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();    
        CFeat_CI f(m_Seq, TSeqRange(start_offset, stop_offset), sel);
        while (f) {
            for(CSeq_loc_CI loc_it(f->GetLocation()); loc_it; ++loc_it) {
                CSeq_loc_CI::TRange this_range = loc_it.GetRange();
                if (this_range.GetFrom() <= start_offset && this_range.GetTo() >= start_offset) {
                    features.push_back(f->GetSeq_feat());
                    break;
                }
            }
            ++f;
        }
    }

}


// adds splice site information in the visible range start_offset to stop_offset (source coordinates)
// splice_sites is an array with positions from start_offset to stop_offset
void CSeqTextDataSourceInterval::LookForSpliceJunctions (const CMappedFeat& feat, CSeqTextDefs::TSpliceSiteVector &splice_sites, TSeqPos start_offset, TSeqPos stop_offset) const
{
    const CSeq_loc& loc = feat.GetLocation();
    string          splice_buffer;
    bool            is_start = true;
    TSeqPos         unset_pos[2];
    bool            need_unset[2];

    need_unset[0] = false;
    need_unset[1] = false;
    unset_pos[0] = 0;
    unset_pos[1] = 0;

    for(CSeq_loc_CI loc_it(loc); loc_it; ++loc_it) {
        CSeq_loc_CI::TRange this_range = loc_it.GetRange();

        TSeqPos feat_start = this_range.GetFrom();
        TSeqPos feat_stop = this_range.GetTo();
        if (feat_stop <= start_offset || feat_start >= stop_offset)
            continue;
        feat_start -= start_offset;
        feat_stop -= start_offset;

        bool is_minus = false;

        if (loc_it.GetStrand() == eNa_strand_minus) {
            is_minus = true;
        }

        CBioseq_Handle handle = m_Seq.GetScope().GetBioseqHandle(*loc_it.GetEmbeddingSeq_loc().GetId());
        CSeqVector vect = handle.GetSeqVector (CBioseq_Handle::eCoding_Iupac);
        if (feat_start > 2  &&  ! is_start && feat_start < stop_offset && feat_start - 2 >= start_offset) {
            // check the two positions before the start of this location
            // to see if they contain A and G
            vect.GetSeqData (feat_start - 2, feat_start, splice_buffer);
            if (x_IsSpliceSite(splice_buffer, true, is_minus)) {
                TSeqPos interval_pos = feat_start - 2;
                if (SequencePosToIntervalPos(interval_pos)) {
                    splice_sites[interval_pos + m_Offset] = true;
                }
                interval_pos = feat_start - 1;
                if (SequencePosToIntervalPos(interval_pos)) {
                    splice_sites[interval_pos + m_Offset] = true;
                }
            }
        }
        need_unset [0] = false;
        need_unset [1] = false;
        if (feat_stop < splice_sites.size() - 2 && feat_stop <= handle.GetBioseqLength() - 2 
            && feat_stop > start_offset && feat_stop + 3 < stop_offset) {
            // check the two positions after the start of this location
            // to see if they contain G and T
            vect.GetSeqData (feat_stop + 1, feat_stop + 3, splice_buffer);
            if (x_IsSpliceSite(splice_buffer, false, is_minus)) {
                TSeqPos interval_pos = feat_stop + 1;
                if (SequencePosToIntervalPos(interval_pos)) {
                    splice_sites[interval_pos] = true;
                    need_unset [0] = true;
                    unset_pos [0] = interval_pos;
                }
                interval_pos = feat_stop + 2;
                if (SequencePosToIntervalPos(interval_pos)) {
                    splice_sites[interval_pos] = true;
                    need_unset [1] = true;
                    unset_pos [1] = interval_pos;
                }
            }
        }
        is_start = false;
    }

    /* unset splice site after last feature interval */
    if (need_unset [0]) {
        splice_sites[unset_pos[0]] = false;
    }
    if (need_unset[1]) {
        splice_sites[unset_pos[1]] = false;
    }

}


CRef<CSeq_loc> CSeqTextDataSourceInterval::x_GetSeqLocForInterval(TSeqPos interval_start, TSeqPos interval_stop) const
{
    CRef<CSeq_loc> cmp(new CSeq_loc());
    CRef<CSeq_id> id(new CSeq_id());
    id->Assign(*(m_Seq.GetId().front().GetSeqId()));
    cmp->SetInt().SetId(*id);
    cmp->SetInt().SetFrom(interval_start + GetSequenceStart());
    cmp->SetInt().SetTo(interval_stop + GetSequenceStart());
    return cmp;
}


bool CSeqTextDataSourceInterval::x_IsSpliceSite(string splice_buffer, bool before_loc, bool is_minus) const
{
    bool is_splice = false;

    if (splice_buffer.length() < 2) {
        is_splice = false;
    } else if (before_loc) {
        if (is_minus) {
            if ((splice_buffer[0] == 'A' && (splice_buffer[1] == 'C' || splice_buffer[1] == 'G'))
                || (splice_buffer[0] == 'G' && splice_buffer[1] == 'C')) {
                is_splice = true;
            } else {
                is_splice = false;
            }
        } else {
            if (splice_buffer[0] == 'A' && (splice_buffer[1] == 'G' || splice_buffer[1] == 'C')) {
                is_splice = true;
            } else {
                is_splice = false;
            }
        }
    } else {
        if (is_minus) {
            if ((splice_buffer[0] == 'C' || splice_buffer[0] == 'G') && splice_buffer[1] == 'T') {
                is_splice = true;
            } else {
                is_splice = false;
            }
        } else {
            if ((splice_buffer[0] == 'G' && (splice_buffer[1] == 'T' || splice_buffer[1] == 'C'))
                || (splice_buffer[0] == 'C' && splice_buffer[1] == 'T')) {
                is_splice = true;
            } else {
                is_splice = false;
            }
        }
    }
    return is_splice;
}


END_NCBI_SCOPE
