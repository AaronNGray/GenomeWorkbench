/*  $Id: na_utils.cpp 43703 2019-08-15 17:43:26Z katargir $
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
* Authors:  Liangshou Wu, Dmitry Rudnev
*
* File Description:
*    Name annotation utility class
*/

#include <ncbi_pch.hpp>
#include <gui/objutils/na_utils.hpp>
#include <gui/objutils/utils.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/annot_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

// needed for NAA meta data processing
#include <gui/objutils/gui_eutils_client.hpp>
#include <misc/xmlwrapp/xmlwrapp.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kRetMax = 5000;
static const char* kSnpSubtypeStr = "variation_snp";

DEFINE_CLASS_STATIC_FAST_MUTEX(CNAUtils::sm_NAAtoUidCacheMutex);
CNAUtils::TNAAtoUidCache CNAUtils::sm_NAAtoUidCache;

DEFINE_CLASS_STATIC_FAST_MUTEX(CNAUtils::sm_UidtoMetaDataCacheMutex);
CNAUtils::TUidtoMetaDataCache CNAUtils::sm_UidtoMetaDataCache;

DEFINE_CLASS_STATIC_FAST_MUTEX(CNAUtils::sm_UidtoGiCacheMutex);
CNAUtils::TUidtoGiCache CNAUtils::sm_UidtoGiCache;


enum EAlignType {
    fAlign_DNA        = 0x01,
    fAlign_Protein    = 0x02,
    fAlign_Mixed      = 0x04,
    fAlign_Invalid    = 0x80000000
};



string CDataTrackUtils::GetAlignType(const CSeq_align& align,
                                     CScope& scope)
{
    string aln_type = kEmptyStr;
    int num_row = 0;
    try {
        num_row = align.CheckNumRows();
    }
    catch (const CException&) {
    }

    if (num_row < 2) return aln_type;

    // check align type
    EAlignType type = fAlign_Invalid;
    for (int row = 0;  row < num_row;  ++row) {
        EAlignType this_type = fAlign_Mixed;
        CBioseq_Handle handle = scope.GetBioseqHandle(align.GetSeq_id(row));
        if ( !handle ) continue;

        switch (handle.GetBioseqCore()->GetInst().GetMol())
        {
        case CSeq_inst::eMol_dna:
        case CSeq_inst::eMol_rna:
        case CSeq_inst::eMol_na:
            this_type = fAlign_DNA;
            break;

        case CSeq_inst::eMol_aa:
            this_type = fAlign_Protein;
            break;

        default:
            break;
        }

        if (this_type == fAlign_Mixed) {
            type = this_type;
            break;
        }

        if (row == 0) {
            type = this_type;
        } else if (this_type != type) {
            type = fAlign_Mixed;
            break;
        }
    }
    switch (type) {
        case fAlign_Protein:
            aln_type = "protein";
            break;
        case fAlign_Mixed:
            aln_type = "protein-to-nucleotide";
            break;
        case fAlign_DNA:
        default:
            aln_type = "nucleotide";
            break;
    }
    return aln_type;
}


inline
static string s_GetFeatSubtypeStr(int subtype)
{
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    if (subtype == (int)CSeqFeatData::eSubtype_variation) {
        return kSnpSubtypeStr;
    }
    return feats.GetStoragekey(subtype);
}


void CDataTrackUtils::GetFeatSubtypes(const CSeq_annot& annot,
                                      set<string>& subtypes)
{
    if ( !annot.GetData().IsFtable() ) return;

    subtypes.clear();
    ITERATE (CSeq_annot::TData::TFtable, feat_it, annot.GetData().GetFtable()) {
        int f_subtype = (*feat_it)->GetData().GetSubtype();
        subtypes.insert(s_GetFeatSubtypeStr(f_subtype));
    }
}


void CDataTrackUtils::GetColumnHeader(set<string>& headers,
                                      const CSeq_annot& annot)
{
    if ( !annot.IsSeq_table() ) return;

    const CSeq_table::TColumns& cols =
        annot.GetData().GetSeq_table().GetColumns();
    const CEnumeratedTypeValues* type_val =
        CSeqTable_column_info::GetTypeInfo_enum_EField_id();

    ITERATE(CSeq_table::TColumns, iter, cols) {
        const CSeqTable_column::THeader& header = (*iter)->GetHeader();
        if (header.CanGetField_name()) {
            headers.insert(header.GetField_name());
        } else if (header.CanGetField_id()  &&  type_val) {
            headers.insert(type_val->FindName(header.GetField_id(), true));
        }
    }
}


// This function used in filtering out gene model features from all
// the other feature type. If you modify it, modify gene model also
// at src/gui/widgets/seq_graphic/gene_model_track.cpp
// CGeneModelTrack::x_AdjustSelector
bool CDataTrackUtils::IsGeneModelFeature(int type, int subtype)
{
    return (subtype == CSeqFeatData::eSubtype_exon      ||
            subtype == CSeqFeatData::eSubtype_misc_RNA  ||
            subtype == CSeqFeatData::eSubtype_C_region  ||
            subtype == CSeqFeatData::eSubtype_V_segment ||
            subtype == CSeqFeatData::eSubtype_D_segment ||
            subtype == CSeqFeatData::eSubtype_J_segment ||
// Temporary moved back out of gene model track, see SV-2107
//            subtype == CSeqFeatData::eSubtype_enhancer  ||
            type == CSeqFeatData::e_Gene  ||
            type == CSeqFeatData::e_Cdregion  ||
            type == CSeqFeatData::e_Rna);
}


void CDataTrackUtils::FeatSubtypes2TrackSubtypes(set<string>& subtypes,
                                                 const set<string>& feat_subtypes)
{
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    string gene_subtype = feats.GetStoragekey(CSeqFeatData::e_Gene);

    set<string> temp_types = feat_subtypes;
    set<string>::iterator type_iter = temp_types.find(gene_subtype);
    if (type_iter != temp_types.end()) {
        subtypes.insert("gene_model");
        temp_types.erase(type_iter);
        // It contains genes, so we treat it as a gene model track.
        // All other RNA, CDS and Exon features will be shown in
        // the same track.
        ITERATE (CFeatList, subtype_iter, feats) {
            if (IsGeneModelFeature(subtype_iter->GetType(), subtype_iter->GetSubtype())) {
                type_iter = temp_types.find(subtype_iter->GetStoragekey());
                if (type_iter != temp_types.end()) {
                    temp_types.erase(type_iter);
                }
            }
        }
    }
    ITERATE(set<string>, iter, temp_types) {
        if (*iter == "variation") {
            subtypes.insert("dbVar");
        } else if (*iter == kSnpSubtypeStr) {
            subtypes.insert("dbSNP");
        } else {
            subtypes.insert(*iter);
        }
    }
}


static bool s_MatchHeaders(const char** header_names,
                           const set<string>& headers)
{
    for ( ;  header_names  &&  *header_names;  ++header_names) {
        if (headers.count(string(*header_names)) == 0) {
            return false;
        }
    }
    return true;
}


string CDataTrackUtils::GetSeqTableSubtype(const set<string>& headers)
{
    static const char* graph_headers[] = {
        "location-from", "span", "values", NULL
    };
    if (s_MatchHeaders(graph_headers, headers)) {
        return "graph";
    }

    static const char* bins_headers[] = {
        "pos", "pvalue", "trait", "pmids", "reportedGenes",
        "mappedGenes", "snpId", "trackType", "clinSigID", NULL
    };
    if (s_MatchHeaders(bins_headers, headers)) {
        return "SNP_bins";
    }

    static const char* GWAS_headers[] = {
        "pos", "pvalue", "trackType", NULL
    };
    if (s_MatchHeaders(GWAS_headers, headers)) {
        return "SNP_bins";
    }

    static const char* HapMap_headers[] = {
        "pos", "value", "trackType", NULL
    };
    if (s_MatchHeaders(HapMap_headers, headers)) {
        return "HapMap";
    }

    return kEmptyStr;
}


CTrackInfo::TTrackInfoList
CDataTrackUtils::GetTrackInfo(CSeq_annot_Handle annot_handle)
{
    CTrackInfo::TTrackInfoList info_list;
    CConstRef<CSeq_annot> annot = annot_handle.GetCompleteSeq_annot();
    if (annot) {
        info_list =  GetTrackInfo(*annot, annot_handle.GetScope());
    }
    return info_list;
}


CTrackInfo::TTrackInfoList
    CDataTrackUtils::GetTrackInfo(const CSeq_annot& annot, CScope& scope)
{
    CTrackInfo::TTrackInfoList info_list;

    CRef<CTrackInfo> info(new CTrackInfo());

    info->m_DataId = CSeqUtils::GetUnnamedAnnot();
    if (annot.IsSetDesc()) {
        ITERATE (CAnnot_descr::Tdata, descrIter, annot.GetDesc().Get()) {
            if ((*descrIter)->IsName()) {
                info->m_DataId = (*descrIter)->GetName();
                break;
            }
        }
    }

    info->m_Title = CSeqUtils::GetAnnotName(annot);
    info->m_Descr = CSeqUtils::GetAnnotComment(annot);
    info->m_AnnotType = CSeq_annot::TData::SelectionName(annot.GetData().Which());
    if (annot.IsFtable()) {
        set<string> feat_subtypes;
        GetFeatSubtypes(annot, feat_subtypes);
        set<string> subtypes;
        FeatSubtypes2TrackSubtypes(subtypes, feat_subtypes);
        ITERATE (set<string>, subtype_iter, subtypes) {
            CRef<CTrackInfo> feat_info(new CTrackInfo(*info));
            feat_info->m_Subtype = *subtype_iter;
            info_list.push_back(feat_info);
        }
    } else if (annot.IsAlign()) {
        const CSeq_annot::TData::TAlign& aligns = annot.GetData().GetAlign();
        if ( !aligns.empty() ) {
            string aln_type =
                CDataTrackUtils::GetAlignType(*aligns.front(), scope);
            if ( !aln_type.empty() ) {
                info->m_Subtype = aln_type;
                info_list.push_back(info);
            }
        }
    } else if (annot.IsGraph()) {
        info->m_Subtype = "graph";
        info_list.push_back(info);
    } else if (annot.IsSeq_table()) {
        if (annot.GetData().GetSeq_table().IsSetFeat_subtype()  &&
            annot.GetData().GetSeq_table().GetFeat_subtype() != 0) {
            int f_subtype = annot.GetData().GetSeq_table().GetFeat_subtype();
            set<string> subtypes;
            set<string> feat_subtypes;
            feat_subtypes.insert(s_GetFeatSubtypeStr(f_subtype));
            FeatSubtypes2TrackSubtypes(subtypes, feat_subtypes);
            if ( !subtypes.empty() ) {
                info->m_Subtype = *subtypes.begin();
                info_list.push_back(info);
            }
        } else {
            set<string> headers;
            CDataTrackUtils::GetColumnHeader(headers, annot);
            info->m_Subtype = CDataTrackUtils::GetSeqTableSubtype(headers);
            if ( !info->m_Subtype.empty() ) {
                info_list.push_back(info);
            }
        }
    }

    return info_list;
}


void CDataTrackUtils::DataType2TrackType(const string& annot_type,
                                         const string& subtype,
                                         string& track_key,
                                         string& subkey)
{
    string ftable =
        CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Ftable);
    string align =
        CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Align);
    string seq_table =
        CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Seq_table);
    string graph =
        CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Graph);

    if (annot_type == align) {
        track_key = "alignment_track";
        subkey = subtype;
    } else if (annot_type == ftable) {
        if (subtype == "gene_model") {
            track_key = "gene_model_track";
        } else if (subtype == "dbVar") {
            track_key = "dbvar_track";
        } else if (subtype == "dbSNP") {
            track_key = "SNP_track";
        } else {
            track_key = "feature_track";
            subkey = subtype;
        }
    } else if (annot_type == graph) {
        if (subtype == "graph") {
            track_key = "graph_track";
        } else if (subtype == "GWAS") {
            track_key = "SNP_Bins_track";
        }
    } else if (annot_type == seq_table) {
        if (subtype == "GWAS") {
            track_key = "SNP_Bins_track";
        } else if (subtype == "HapMap") {
            track_key = "HapMapRR_track";
        } else if (subtype == "SNP_bins") {
            track_key = "SNP_bins_track";
        } else if (subtype == "graph") {
            track_key = "graph_track";
        } else {
            track_key = "feature_track";
            subkey = subtype;
        }
    } else {
        if (subtype == "sequence") {
            track_key = "sequence_track";
        } else if (subtype == "six_frame") {
            track_key = "six_frames_translation";
        } else if (subtype == "segment_map") {
            track_key = "segment_map_track";
        }
    }
}


/// help class for iterating through the NA DocSum one by one.
class CNADocSumIterator
{
public:
    CNADocSumIterator(const CSeqUtils::TUids &uids, const int max_nas)
        : m_Valid(false)
    {
        if ( uids.empty() )
            return;

        // prepare eSummary request
        CGuiEutilsClient ecli;
        ecli.SetMaxReturn(max_nas);
        m_Doc.reset(new xml::document());

        // Get and show the results
        try {
            ecli.Summary("seqannot", uids, *m_Doc);
        }
        catch (const CException& e) {
            LOG_POST(Error << "Get error when trying to retrieve NA meta-data. Error: " << e.GetMsg());
            return;
        }

        xml::node& root = m_Doc->get_root_node();
        m_DocSumRoot = root.find("DocumentSummarySet", root.begin());
        if (m_DocSumRoot != root.end()) {
            m_Valid = true;
            m_Iter = m_DocSumRoot->begin();
        }

    }

    bool is_valid() const
    {
        return m_Valid  &&  m_Iter != m_DocSumRoot->end();
    }

    operator bool() const
    {
        return is_valid();
    }

    void operator++()
    {
        // please do check the validity before call this method
        _ASSERT(this->is_valid());
        ++m_Iter;
    }

    const xml::node& operator*() const
    {
        // please do check the validity before call this method
        _ASSERT(this->is_valid());
        return *m_Iter;
    }

    xml::node::const_iterator operator->() const
    {
        // please do check the validity before call this method
        _ASSERT(this->is_valid());
        return m_Iter;
    }

private:
    auto_ptr<xml::document> m_Doc;
    xml::node::const_iterator m_Iter;
    xml::node::const_iterator m_DocSumRoot;
    bool m_Valid;
};


CNAUtils::CNAUtils()
{
    x_Init();
}


CNAUtils::CNAUtils(const CSeq_id& id)
    : m_TargetSeq(&id)
{
    m_Scope.Reset(new CScope(*CObjectManager::GetInstance()));
    m_Scope->AddDefaults();
    x_Init();
}


CNAUtils::CNAUtils(const CSeq_id& id, CScope& scope)
    : m_TargetSeq(&id)
    , m_Scope(&scope)
{
    x_Init();
}


void CNAUtils::GetAllNAAs(TNAAs& naas, const string& context) const
{
    naas.clear();
    CSeqUtils::TUids uids;
    x_GetNAIds(uids, context);
    x_GetNAAs(naas, uids);
}

void CNAUtils::GetNAMetaData(TNAMetaDataSet& md_set,
                             const string& na,
                             bool filtering,
                             EMetaDataSource* pMDSource,
                             EUidsSource* pUidsSource,
                             bool isGetLinks) const
{
    if (na.empty()) {
        if(pUidsSource) {
            *pUidsSource = EUidsSource_EmptyRequest;
        }
        return;
    }

    md_set.clear();
    CSeqUtils::TUids uids;
    TNAAs naas;
    naas.push_back(na);
    GetNAMetaData(md_set, naas, filtering, pMDSource, pUidsSource, isGetLinks);
}


void CNAUtils::GetNAMetaData(TNAMetaDataSet& md_set,
                             const TNAAs& naas,
                             bool filtering,
                             EMetaDataSource* pMDSource,
                             EUidsSource* pUidsSource,
                             bool isGetLinks) const
{
    // LOG_POST("<<<<");
    if (naas.empty()) {
        if(pUidsSource) {
            *pUidsSource = EUidsSource_EmptyRequest;
        }
        return;
    }

    md_set.clear();
    CSeqUtils::TUids uids;
    x_SearchNAIds(uids, naas, filtering, pUidsSource);
    if(uids.empty()) {
        if(pMDSource) {
            *pMDSource = EMetaDataSource_EmptyAfterFilteringRequest;
        }
        return;
    }
    x_GetNAMetaData(md_set, uids, isGetLinks, pMDSource);
    // LOG_POST(">>>>");
}

const size_t kChunkSize = 256;

void CNAUtils::GetAllNAMetaData(
    TNAMetaDataSet& md_set,
    const string& context,
    ICanceled* canceledCallback) const
{
    CSeqUtils::TUids uids;
    x_GetNAIds(uids, context);
    for (CSeqUtils::TUids::const_iterator it = uids.begin(); it != uids.end();) {
        CSeqUtils::TUids tmp;
        for (size_t i = kChunkSize; i != 0 && it != uids.end(); ++it, --i)
            tmp.push_back(*it);
        if (canceledCallback && canceledCallback->IsCanceled())
            break;
        x_GetNAMetaData(md_set, tmp, false);
    }
}


void CNAUtils::GetNATrackInfo(TTrackInfoList& track_info,
                              const string& naa) const
{
    TNAMetaDataSet md_set;
    GetNAMetaData(md_set, naa);
    GetNATrackInfo(track_info, naa, md_set);
}


void CNAUtils::GetNATrackInfo(TTrackInfoList& track_info,
                              const string& naa,
                              const TNAMetaDataSet& md_set) const
{
    if (md_set.empty()) return;

    TNAMetaDataSet::const_iterator iter = md_set.find(naa);
    if (iter != md_set.end()) {
        string ftable =
            CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Ftable);
        string align =
            CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Align);
        string seq_table =
            CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Seq_table);
        string graph =
            CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Graph);

        const CAnnotMetaData& data = *iter->second;
        CRef<CTrackInfo> info(new CTrackInfo);
        info->m_DataId = data.m_Name;
        info->m_Title = data.m_Title;
        info->m_Descr = data.m_Descr;
        info->m_Group = data.m_xClass;
        info->m_Dbname = "SADB";
        info->m_AnnotType = data.m_AnnotType;

        if (data.m_AnnotType == ftable) {
            set<string> subtypes;
            CDataTrackUtils::FeatSubtypes2TrackSubtypes(subtypes, data.m_Subtypes);
            ITERATE (set<string>, subtype_iter, subtypes) {
                CRef<CTrackInfo> feat_info(new CTrackInfo(*info));
                feat_info->m_Subtype = *subtype_iter;
                track_info.push_back(feat_info);
            }
        } else if (data.m_AnnotType == align) {
            if (data.m_Subtypes.empty()) {
                // try to see if we could find out what type
                // of alignments it contains
                if (m_TargetSeq) {
                    if (m_BioseqHandle) {
                        SAnnotSelector sel;
                        sel.IncludeAnnotType(CSeq_annot::C_Data::e_Align);
                        sel.SetMaxSize(1);
                        sel.SetCollectNames(false);
                        sel.AddNamedAnnots(data.m_Name);
                        sel.IncludeNamedAnnotAccession(data.m_Name);
                        CAlign_CI aln_iter(m_BioseqHandle, TSeqRange::GetWhole(), sel);
                        if (aln_iter) {
                            string aln_type =
                                CDataTrackUtils::GetAlignType(*aln_iter, const_cast<CScope&>(*m_Scope));
                            if ( !aln_type.empty() ) {
                                info->m_Subtype = aln_type;
                                track_info.push_back(info);
                            }
                        }
                    }
                }
            } else if (data.m_Subtypes.size() == 1) {
                info->m_Subtype = *data.m_Subtypes.begin();
                track_info.push_back(info);
            }
            // otherwise, too many subtype, give up
        } else if (data.m_AnnotType == graph  &&  data.m_Subtypes.size() < 2) {
            if (data.m_Subtypes.empty()) {
                info->m_Subtype = "graph";
            } else {
                info->m_Subtype = *data.m_Subtypes.begin();
            }
            track_info.push_back(info);
        } else if (data.m_AnnotType == seq_table) {
            if (data.m_Subtypes.size() == 1) {
                info->m_Subtype = *data.m_Subtypes.begin();
                track_info.push_back(info);
            } else if (data.m_Subtypes.empty()  &&  m_TargetSeq  &&  m_BioseqHandle) {
                SAnnotSelector sel;
                sel.IncludeAnnotType(CSeq_annot::C_Data::e_Seq_table);
                sel.SetMaxSize(1);
                sel.SetCollectNames(false);
                sel.AddNamedAnnots(data.m_Name);
                sel.IncludeNamedAnnotAccession(data.m_Name);
                CAnnot_CI annot_iter(m_BioseqHandle, sel);
                if (annot_iter) {
                    const CSeq_annot& annot = *annot_iter->GetCompleteSeq_annot();
                    // check if it stores features
                    if (annot.GetData().GetSeq_table().IsSetFeat_subtype()  &&
                        annot.GetData().GetSeq_table().GetFeat_subtype() != 0) {
                        int f_subtype = annot.GetData().GetSeq_table().GetFeat_subtype();
                        info->m_Subtype = s_GetFeatSubtypeStr(f_subtype);

                        set<string> subtypes;
                        set<string> feat_subtypes;
                        feat_subtypes.insert(s_GetFeatSubtypeStr(f_subtype));
                        CDataTrackUtils::FeatSubtypes2TrackSubtypes(subtypes, feat_subtypes);
                        if ( !subtypes.empty() ) {
                            info->m_Subtype = *subtypes.begin();
                            track_info.push_back(info);
                        }
                    } else {
                        set<string> headers;
                        CDataTrackUtils::GetColumnHeader(headers, annot);
                        info->m_Subtype = CDataTrackUtils::GetSeqTableSubtype(headers);
                        if ( !info->m_Subtype.empty() ) {
                            track_info.push_back(info);
                        }
                    }
                }
            }
        }
    }
}


void CNAUtils::GetNATrackInfo(TTrackInfoList& track_info,
                              const TNAAs& naas) const
{
    TNAMetaDataSet md_set;
    GetNAMetaData(md_set, naas);
    GetNATrackInfo(track_info, naas, md_set);
}


void CNAUtils::GetNATrackInfo(TTrackInfoList& track_info,
                              const TNAAs& naas,
                              const TNAMetaDataSet& md_set) const
{
    if (md_set.empty()) return;

    ITERATE(TNAAs, iter, naas) {
        GetNATrackInfo(track_info, *iter, md_set);
    }
}


void CNAUtils::SetMaxNAs(int size)
{
    if (size > 0) {
        m_MaxNAMeta = size;
    }
}

static string s_EEDBToString(CNAUtils::EEntrezDB eedb)
{
    switch(eedb) {
    case CNAUtils::EEDB_Nucleotide:
        return "nucleotide";
    case CNAUtils::EEDB_Protein:
        return "protein";
    default:
        ;
    }
    return "";
}

void CNAUtils::x_Init()
{
    m_Gi = ZERO_GI;

    m_MaxNAMeta = kRetMax;

    if (m_TargetSeq) {
        m_BioseqHandle = m_Scope->GetBioseqHandle(*m_TargetSeq);
        m_SeqIdHandle = m_BioseqHandle.GetAccessSeq_id_Handle();
        CSeq_id_Handle shdl = sequence::GetId(m_SeqIdHandle, *m_Scope, sequence::eGetId_ForceGi);
        if (shdl) {
            m_Gi = shdl.GetGi();
        }
        m_SeqDB = m_BioseqHandle.IsNa() ? EEDB_Nucleotide : EEDB_Protein;
    }
}


void CNAUtils::x_GetNAIds(CSeqUtils::TUids &uids, const string& context) const
{
    uids.clear();
    if (!m_BioseqHandle)
        return;

    list<string> ids;
    if (context.empty()  ||  NStr::EqualNocase(context, "all")) {
        x_GetAllNAIds(uids);
    } else {
        // Get NAA ids based on a viewer context
        // Related JIRA tickets: ID-544 and SV-831
        x_GetAllNAIdsWithContext(uids, context);
    }
}


void CNAUtils::x_GetAllNAIds(CSeqUtils::TUids &uids) const
{
    CSeqUtils::TSeqIdHandles uids_from;
    uids_from.push_back(m_SeqIdHandle);

    try {
        CStopWatch sw;
        sw.Start();
        CSeqUtils::ELinkQuery(s_EEDBToString(m_SeqDB), "seqannot", uids_from, uids);
        sw.Stop();
        string info = "Timing: " + sw.AsSmartString(CTimeSpan::eSSP_Millisecond)
                      + ". elink.fcgi query from " + s_EEDBToString(m_SeqDB) + " db. Seq-Annots for ";
        bool first = true;
        for (auto i : uids_from) {
            if (first) first = false;
            else info += ", ";
            info += i.AsString();
        }
        info += ".";
//        LOG_POST(Info << info);
    }
    catch (const CException& e) {
        LOG_POST(Error << "Get error when trying to get NA ids for seq-id: " << m_SeqIdHandle.AsString() << ". Error: " << e.GetMsg());
    }
}


void CNAUtils::x_GetAllNAIdsWithContext(CSeqUtils::TUids &uids, const string& context) const
{
    CSeqUtils::TSeqIdHandles uids_from;
    uids_from.push_back(m_SeqIdHandle);

    xml::document linkset;

    try {
        CSeqUtils::ELinkQuery(s_EEDBToString(m_SeqDB), "seqannot", uids_from, linkset, "neighbor_history");
    }
    catch (const CException& e) {
        LOG_POST(Error << "Got error when trying to get NA list for seq-id: " << m_SeqIdHandle.AsString() << " and context: " << context << ". Error: " << e.GetMsg());
        return;
    }

    xml::node_set nodes ( linkset.get_root_node().run_xpath_query("//LinkSetDbHistory/QueryKey/text()") );
    if (nodes.empty())
        return;
    string query_key_str = nodes.begin()->get_content();

    nodes = linkset.get_root_node().run_xpath_query("//WebEnv/text()");
    if (nodes.empty())
        return;
    string web_env = nodes.begin()->get_content();
    if (query_key_str.empty() || web_env.empty())
        return;

    // Get NA ids using esearch
	size_t count(0);
    try {
		CSeqUtils::ESearchQuery("seqannot", context + "[viewer_context]", web_env, query_key_str, uids, count);
    }
    catch (const CException& e) {
        LOG_POST(Error << "Got error when trying to get NA id list for viewer_context: " << context << " and query_key: " << query_key_str << ". Error: " << e.GetMsg());
    }
}


void CNAUtils::x_GetNAAs(TNAAs& naas, const CSeqUtils::TUids &uids) const
{
    CNADocSumIterator ds_iter(uids, m_MaxNAMeta);
    for (; ds_iter; ++ds_iter) {
        xml::node::const_iterator c_i = ds_iter->find("Caption", ds_iter->begin());
        xml::node::const_iterator sum_i = ds_iter->find("ExpXml", ds_iter->begin());
        if (c_i != ds_iter->end()  &&  sum_i != ds_iter->end()) {
            string naa(c_i->get_content());
            naas.push_back(naa);
        }
    }
}


void CNAUtils::x_GetNAMetaData(TNAMetaDataSet& md_set,
                               const CSeqUtils::TUids &uids,
                               bool isGetLinks,
                               EMetaDataSource* pSource) const
{
    // LOG_POST("<<<<");
    if(uids.size() == 0) {
        if(pSource) {
            *pSource = EMetaDataSource_EmptyRequest;
        }
        return;
    }
    // first check the cache and get information from it for those uids that are cached
    // set aside uids that are not in the cache and later retrieve them using eutils
    CSeqUtils::TUids non_cached_uids;
    {
        CFastMutexGuard lock(sm_UidtoMetaDataCacheMutex);
        ITERATE(CSeqUtils::TUids, iuids, uids) {
            TUidtoMetaDataCache::const_iterator iCacheHit(sm_UidtoMetaDataCache.find(*iuids));
            if(iCacheHit != sm_UidtoMetaDataCache.end()) {
                // LOG_POST(Trace << "CNAUtils::x_GetNAMetaData() cache hit on uid: " << *iuids);
                md_set[iCacheHit->second->m_Name] = iCacheHit->second;
            } else {
                // LOG_POST(Trace << "CNAUtils::x_GetNAMetaData() cache miss on uid: " << *iuids);
                non_cached_uids.push_back(*iuids);
            }
        }
    }
    if(pSource) {
        if(uids.size() == 0) {
            *pSource = EMetaDataSource_EmptyRequest;
        }
        if(non_cached_uids.size() > 0) {
            if(non_cached_uids.size() == uids.size()) {
                *pSource = EMetaDataSource_Eutils;
            } else  {
                *pSource = EMetaDataSource_Mixed;
            }
        } else {
            *pSource = EMetaDataSource_Cache;
        }
    }
    if (non_cached_uids.size() > 0)  {
        CStopWatch sw;
        sw.Start();
        CNADocSumIterator ds_iter(non_cached_uids, m_MaxNAMeta);
        sw.Stop();
        string info = "Timing: " + sw.AsSmartString(CTimeSpan::eSSP_Millisecond)
            + ". esummary.fcgi query for ";
        bool first = true;
        for (auto i : non_cached_uids) {
            if (first) first = false;
            else info += ", ";
            info += NStr::NumericToString(i);
        }
        info += ".";
//        LOG_POST(Info << info);

        for (; ds_iter; ++ds_iter) {
            xml::node::const_iterator c_i = ds_iter->find("Caption", ds_iter->begin());
            xml::node::const_iterator sum_i = ds_iter->find("ExpXml", ds_iter->begin());
            if (c_i != ds_iter->end() && sum_i != ds_iter->end()) {
                CRef<CAnnotMetaData> data(new CAnnotMetaData);
                data->m_Name = c_i->get_content();

                ITERATE(xml::attributes, a_i, ds_iter->get_attributes()) {
                    if (NStr::Equal(a_i->get_name(), "uid")) {
                        data->m_Id = a_i->get_value();
                        break;
                    }
                }

                try {
                    x_ParseNAMetaData(*data, sum_i->get_content());
                    // always get links so they can be cached even when the are not requested by caller
                    md_set[data->m_Name] = data;
                    if (!data->m_Id.empty())
                    {
                        CSeqUtils::TUid uid(NStr::StringToNonNegativeInt(data->m_Id));
                        if (!errno) {
                            // add retrieved data to cache
                            CFastMutexGuard lock(sm_UidtoMetaDataCacheMutex);
                            sm_UidtoMetaDataCache[uid] = data;
                            // LOG_POST(Trace << "CNAUtils::x_GetNAMetaData() added to cache uid: " << uid);
                        }
                    }
                }
                catch (const CException&) {
                    // ignore this NA
                }
                catch (const exception&) {
                    // ignore this NA
                }
            }
        }
    }
    // if links are requested, check all prepared entries for presence of links and get them
    // from eutils if some are missing
    if (isGetLinks) {
        NON_CONST_ITERATE(TNAMetaDataSet, i_md_set, md_set) {
            if (i_md_set->second->m_LinksStatus == CAnnotMetaData::ELinksStatus_Undef) {
                CRef<CAnnotMetaData> data(i_md_set->second);
                x_GetNAEntrezLinks(*data);
                if (pSource && *pSource == EMetaDataSource_Cache){
                    *pSource = EMetaDataSource_Mixed;
                }
                // LOG_POST(Trace << "CNAUtils::x_GetNAMetaData() links added to cache uid: " << (*i_md_set)->m_Id);
            }
        }
    }
    // LOG_POST(">>>>");
}


void CNAUtils::x_FilterNAIds(CSeqUtils::TUids &uids,
                             EUidsSource* pUidsSource) const
{
    // LOG_POST("<<<<");
    CSeqUtils::TUids filtered_ids;
    ITERATE(CSeqUtils::TUids, i_uids, uids) {
        if(x_NAIdMatchesGI(*i_uids, pUidsSource)) {
            filtered_ids.push_back(*i_uids);
        }
    }
    uids.swap(filtered_ids);
    // LOG_POST(">>>>");
}

#define USE_GI_CACHING
#ifdef USE_GI_CACHING
bool CNAUtils::x_NAIdMatchesGI(CSeqUtils::TUid uid,
                               EUidsSource* pUidsSource) const
{
    // LOG_POST("<<<<" << uid);
    bool isMatch(false);
    if(m_Gi != ZERO_GI) {
        TGis gis;
        x_GetAllGIs(uid, gis, pUidsSource);
        isMatch = (gis.find(m_Gi) != gis.end());
    } else {
        // if a target sequence was not given, it's always a match
        // otherwise, it looks like a GI-less sequence and is always a mismatch (at least for now, while eutils can't truly work with
        // GI-less accessions, EU-2741)
        isMatch = !m_TargetSeq;
    }
    // LOG_POST(">>>>" << isMatch);
    return isMatch;
}

#else

// Use XPath request that does not return large result set
// The drawback - hard to implement GIs caching
bool CNAUtils::x_NAIdMatchesGI(CSeqUtils::TUid uid,
                               EUidsSource* pUidsSource) const
{
    CSeqUtils::TUids uids_from;
    uids_from.push_back(uid);
    CSeqUtils::TGis gis_to;

    if(m_Gi != ZERO_GI) {
        try {
            // LOG_POST("Calling eutils to get all GI IDs for NA Id " << uid);
            if(pUidsSource) {
                *pUidsSource = (*pUidsSource == EUidsSource_Cache  || *pUidsSource == EUidsSource_Mixed) ? EUidsSource_Mixed : EUidsSource_Eutils;
            }
            CSeqUtils::ELinkQuery("seqannot", s_EEDBToString(m_SeqDB), uids_from, gis_to, "neighbor",
                "/eLinkResult/LinkSet/LinkSetDb/Link[Id=" + NStr::IntToString(m_Gi) + "]/Id/text()");
            // LOG_POST("Calling eutils done");
            return !gis_to.empty() && gis_to[0] == m_Gi;
        }
        catch (const CException& e) {
            LOG_POST(Error << "Get error when trying to get GIs for NA uid: " << uid << ". Error: " << e.GetMsg());
            return false;
        }
    } else {
        // if a target sequence was not given, it's always a match
        // otherwise, it looks like a GI-less sequence and is always a mismatch (at least for now, while eutils can't truly work with
        // GI-less accessions, EU-2741)
        return !m_TargetSeq
    }
}

#endif

void CNAUtils::GetAllGIs(const TNAA& naa, EEntrezDB eedb, TGis& gis,
        EUidsSource* pUidsSource)
{
    CSeqUtils::TUid na_uid;

    if(!x_GetNAId(na_uid, naa, pUidsSource)) {
        return;
    }
    if(eedb == EEDB_All) {
        x_GetAllGIs(na_uid, EEDB_Nucleotide, gis, pUidsSource);
        x_GetAllGIs(na_uid, EEDB_Protein, gis, pUidsSource, true);
    } else {
        x_GetAllGIs(na_uid, eedb, gis, pUidsSource);
    }
}


void CNAUtils::x_GetAllGIs(CSeqUtils::TUid uid, TGis& gis,
                           EUidsSource* pUidsSource) const
{
    x_GetAllGIs(uid, m_SeqDB, gis, pUidsSource);
}

static string s_MakeUidtoGiCacheKey(CSeqUtils::TUid uid, CNAUtils::EEntrezDB eedb)
{
    return NStr::NumericToString(uid) + "|" + s_EEDBToString(eedb);
}

void CNAUtils::x_GetAllGIs(CSeqUtils::TUid uid, EEntrezDB eedb, TGis& gis,
                           EUidsSource* pUidsSource,
                           bool isIncremental)
{
    // LOG_POST("<<<<");
    if(!isIncremental) {
        gis.clear();
    }
    NCBI_ASSERT(eedb != EEDB_Undef, "Entrez database not defined!");
    {
        // first check the cache
        CFastMutexGuard lock(sm_UidtoGiCacheMutex);
        TUidtoGiCache::const_iterator iCacheHit(sm_UidtoGiCache.find(s_MakeUidtoGiCacheKey(uid, eedb)));
        if(iCacheHit != sm_UidtoGiCache.end()) {
            gis.insert(iCacheHit->second.begin(), iCacheHit->second.end());
            // LOG_POST(Trace << "CNAUtils::x_GetAllGIs() cache hit on uid: " << uid);
            if(pUidsSource) {
                *pUidsSource = (*pUidsSource == EUidsSource_Eutils || *pUidsSource == EUidsSource_Mixed) ? EUidsSource_Mixed : EUidsSource_Cache;
            }
            return;
        }
    }
    // LOG_POST(Trace << "CNAUtils::x_GetAllGIs() cache miss on uid: " << uid);
    CSeqUtils::TUids uids_from;
    uids_from.push_back(uid);
    CSeqUtils::TGis gis_to;

    try {
        // LOG_POST("Calling eutils to get all GI IDs for NA Id " << uid);
        if(pUidsSource) {
            *pUidsSource = (*pUidsSource == EUidsSource_Cache  || *pUidsSource == EUidsSource_Mixed) ? EUidsSource_Mixed : EUidsSource_Eutils;
        }
#if 0
        // This call uses XPath inside and appears to be very slow, in the order of
        // minutes if the result set is in tens of thousands of entries. It is deficiency
        // of XPath library - extremely inefficient sorting of result set which,
        // in addition sorts incorrectly
        CSeqUtils::ELinkQuery("seqannot", s_EEDBToString(eedb), uids_from, gis_to);
        // More efficient query
//        CSeqUtils::ELinkQuery("seqannot", s_EEDBToString(m_SeqDB), uids_from, gis_to, "neighbor",
//            "/eLinkResult/LinkSet/LinkSetDb/Link/Id/text()");

// DEBUG
//        CNcbiOfstream ofs;
//        ofs.open("debug_xpath_expression.txt", std::ofstream::out);
//        ITERATE(CSeqUtils::TGis, i_gis_to, gis_to) {
//            ofs << NStr::NumericToString<TGi>(*i_gis_to) << endl;
//        }
//        ofs.close();
//        gis_to.clear();
#else

        // Run "/eLinkResult/LinkSet/LinkSetDb/Link/Id/text()" manually

        xml::document xmldoc;
        CSeqUtils::ELinkQuery("seqannot", s_EEDBToString(eedb), uids_from, xmldoc);

        // LOG_POST("XML doc for " << uid << " " << xmldoc);

        string container_path[] = { "eLinkResult", "LinkSet", "LinkSetDb" };
        const int container_path_length = 3;
        string content_path[] = { "Link", "Id" };
        const int content_path_length = 2;

        const xml::node *container = &xmldoc.get_root_node();
        // Find actual container
        bool found = false;
        // Check the root for name match
        int cp = 0;
        xml::node::const_iterator it, it1, it2;
        if (container->get_name() == container_path[cp++]) {
            // LOG_POST("XDoc root matched eLinkResult");
            for (; cp < container_path_length; ++cp) {
                found = false;
                for (it = container->begin(); it != container->end(); ++it) {
                    if (!it->is_text() && it->get_name() == container_path[cp]) {
                        // Found container path part
                        found = true;
                        container = &*it;
                        break;
                    }
                }
                if (!found) break;
            }
        }
        if (found) {
            // LOG_POST(container->get_name() << " found");
            for (it1 = container->begin(); it1 != container->end(); ++it1) {
                if (it1->is_text()) continue;

                const xml::node *content = &*it1;
                bool found = false;
                // Check the node for name match
                int cp = 0;
                if (content->get_name() == content_path[cp++]) {
                    // LOG_POST("Verified node root as " << content->get_name());
                    for (; cp < content_path_length; ++cp) {
                        found = false;
                        for (it2 = content->begin(); it2 != content->end(); ++it2) {
                            if (!it2->is_text() && it2->get_name() == content_path[cp]) {
                                // Found container path part
                                found = true;
                                content = &*it2;
                                break;
                            }
                        }
                        if (!found) break;
                    }
                }
                if (found) {
                    // LOG_POST("content found \"" << *content << '"');
                    string id(content->get_content());
                    if (!id.empty()) {
                        gis_to.push_back(NStr::StringToNumeric<TGi>(id));
                    }
                }
            }
        }
        std::sort(gis_to.begin(), gis_to.end());

// DEBUG dump
//        CNcbiOfstream ofs;
//        ofs.open("debug_" + NStr::IntToString(uid) + "_manual.txt", std::ofstream::out);
//        ITERATE(CSeqUtils::TGis, i_gis_to, gis_to) {
//            ofs << *i_gis_to << endl;
//        }
//        ofs.close();
#endif
        // LOG_POST("Calling eutils done");
    }
    catch (const CException& e) {
        LOG_POST(Error << "Get error when trying to get GIs for uid: " << uid << ". Error: " << e.GetMsg());
    }
    {
        // LOG_POST(Trace << "CNAUtils::x_GetAllGIs() added to cache uid: " << uid);
        // update the cache and return the values
        CFastMutexGuard lock(sm_UidtoGiCacheMutex);

        // create a map element even if a list of GIs is empty
        TGis& cached_gis(sm_UidtoGiCache[s_MakeUidtoGiCacheKey(uid, eedb)]);

        ITERATE(CSeqUtils::TGis, i_gis_to, gis_to) {
            gis.insert(*i_gis_to);
            cached_gis.insert(*i_gis_to);
        }
    }
    // LOG_POST(">>>>");
}


void CNAUtils::x_SearchNAIds(CSeqUtils::TUids &uids, const TNAAs& naas, bool filtering,
                             EUidsSource* pUidsSource) const
{
    ITERATE(TNAAs, inaas, naas) {
        CSeqUtils::TUid uid;
        if(x_GetNAId(uid, *inaas, pUidsSource)) {
            uids.push_back(uid);
        }
    }
    if(filtering  &&  m_BioseqHandle) {
        // getting all NA IDs for a molecule proved to be extremely slow
        // doing the inverse -- check GIs for each of all presented NA IDs and select the ones that have matching GIs
        x_FilterNAIds(uids, pUidsSource);
    }
}

bool CNAUtils::x_GetNAId(CSeqUtils::TUid &uid, const string& naa, EUidsSource* pUidsSource)
{
    // LOG_POST("<<<<");
    // check for presence of a given naa in cache
    {
        CFastMutexGuard lock(sm_NAAtoUidCacheMutex);
        TNAAtoUidCache::const_iterator iCacheHit(sm_NAAtoUidCache.find(naa));
        if(iCacheHit != sm_NAAtoUidCache.end()) {
            uid = iCacheHit->second;
            // LOG_POST(Trace << "CNAUtils::x_GetNAId() cache hit on NAA: " << naa);
            if(pUidsSource) {
                *pUidsSource = (*pUidsSource == EUidsSource_Eutils  || *pUidsSource == EUidsSource_Mixed) ? EUidsSource_Mixed : EUidsSource_Cache;
            }
            return true;
        }
    }
    // LOG_POST(Trace << "CNAUtils::x_GetNAId() cache miss on NAA: " << naa);
    // prepare eSearch request
    CGuiEutilsClient ecli;
    // should normally be one, so using a dynamic setting does not make sense hier
    ecli.SetMaxReturn(kRetMax);
    CSeqUtils::TUids uids;

    try {
        // LOG_POST("Calling eutils with term " << term);
        if(pUidsSource) {
            *pUidsSource = (*pUidsSource == EUidsSource_Cache  || *pUidsSource == EUidsSource_Mixed) ? EUidsSource_Mixed : EUidsSource_Eutils;
        }
        ecli.Search("seqannot", naa, uids);
        // LOG_POST("eutils call done");
    }
    catch (const CException& e) {
        LOG_POST(Error << "Get error when trying to search NAAs for: " << naa << ". Error: " << e.GetMsg());
        return false;
    }
    // if found by eutils, update the cache and return the value
    if(uids.size()) {
        NCBI_ASSERT(uids.size() == 1, "More that one uid is returned for NAA");
        uid = uids[0];
        // LOG_POST(Trace << "CNAUtils::x_GetNAId() added to cache NAA: " << naa);
        CFastMutexGuard lock(sm_NAAtoUidCacheMutex);
        sm_NAAtoUidCache[naa] = uid;
        return true;
    }
    return false;
    // LOG_POST(">>>>");
}


// child node 'Descriptor'
// child node 'Provider'
// child node 'XClass'
// child node 'Content'
// child node 'Properties'

void CNAUtils::x_ParseNAMetaData(CAnnotMetaData& data, const char* xml_str) const
{
    xml::document doc(xml_str, strlen(xml_str), NULL);
    xml::node& root = doc.get_root_node();
    xml::node::const_iterator i;
    // parse descriptor
    ITERATE (xml::node, i, root) {
        if (NStr::Equal(i->get_name(), "Algorithm-List")) continue;

        string node_name = i->get_name();
        if (node_name == "Descriptor") {
            ITERATE (xml::node, e_i, *i) {
                if (NStr::Equal(e_i->get_name(), "Title")) {
                    data.m_Title = e_i->get_content();
                } else if (NStr::Equal(e_i->get_name(), "Comment")) {
                    string tmp_str = e_i->get_content();
                    tmp_str = NStr::Replace(tmp_str, "&amp;", "&");
                    tmp_str = NStr::Replace(tmp_str, "&#xA;", "\n");
                    tmp_str = NStr::Replace(tmp_str, "&#xa;", "\n");
                    data.m_Descr = tmp_str;
                }
            }
            ITERATE (xml::attributes, a_i, i->get_attributes()) {
                if (NStr::Equal(a_i->get_name(), "name")) {
                    data.m_OtherName = a_i->get_value();
                } else if (NStr::Equal(a_i->get_name(), "scope")) {
                    data.m_Scope = a_i->get_value();
                }
            }
            if (data.m_Title.empty()) {
                data.m_Title = data.m_OtherName;
            }
        }
        else if (node_name == "Provider") {
            ITERATE(xml::node, e_i, *i) {
                if (!NStr::Equal(e_i->get_name(), "Annot-chain"))
                    continue;
                data.m_AnnotChain = e_i->get_content();
                break;
            }
        } else if (node_name == "XClass") {
            ITERATE (xml::attributes, a_i, i->get_attributes()) {
                if (NStr::Equal(a_i->get_name(), "type")) {
                    data.m_xClass = a_i->get_value();
                }
            }

        } else if (node_name == "Properties") {
            string keywords = kEmptyStr;
            ITERATE (xml::node, e_i, *i) {
                string e_name = e_i->get_name();
                if (e_name == "TargetAssembly") {
                    ITERATE (xml::attributes, a_i, e_i->get_attributes()) {
                        if (NStr::Equal(a_i->get_name(), "idstr")) {
                            data.m_AssmAcc = a_i->get_value();
                        }
                    }
                } else if (e_name == "LinkList") {
                    ITERATE (xml::node, ln_i, *e_i) {
                        if ( !NStr::Equal(ln_i->get_name(), "Link") ) {
                            continue;
                        }
                        ITERATE (xml::node, url_entrez_i, *ln_i) {
                            string label, url;
                            if (NStr::Equal(url_entrez_i->get_name(), "URL_Link")) {
                                ITERATE (xml::node, lln_i, *url_entrez_i) {
                                    if (NStr::Equal(lln_i->get_name(), "Label")) {
                                        label = lln_i->get_content();
                                    } else if (NStr::Equal(lln_i->get_name(), "URL")) {
                                        url = lln_i->get_content();
                                    }
                                }
                            } else if (NStr::Equal(url_entrez_i->get_name(), "Entrez_Link")) {
                                // we shouldn't retrieve Entrez links from NAA's meta-data since
                                // they might not be up-to-date.  Instead, we will retrieve them
                                // through elink.fcgi.
                                ITERATE (xml::node, lln_i, *url_entrez_i) {
                                    if (NStr::Equal(lln_i->get_name(), "DB")) {
                                        label = lln_i->get_content();
                                    } else if (NStr::Equal(lln_i->get_name(), "ID")) {
                                        url = lln_i->get_content();
                                    }
                                }
                                if ( !label.empty()  &&  !url.empty() ) {
                                    url = "https://www.ncbi.nlm.nih.gov/" +
                                        label + "/" + url;
                                }
                            }
                            if ( !label.empty()  &&  !url.empty() ) {
                                data.m_Links[label] = url;
                            }
                        }
                    }
                }
            }

        } else if (node_name == "Content") {
            // It is possible there are more than one annotation types
            // and feature (sub)types set in meta-data. We need to
            // collect them all
            set<string> annot_types;
            ITERATE (xml::node, e_i, *i) {
                string e_name = e_i->get_name();
                if (e_name == "Annot") {
                    ITERATE (xml::attributes, a_i, e_i->get_attributes()) {
                        if (NStr::Equal(a_i->get_name(), "type")) {
                            annot_types.insert(a_i->get_value());
                        }
                    }
                    ITERATE (xml::node, t_i, *e_i) {
                        if (NStr::Equal(t_i->get_name(), "Feature")) {
                            ITERATE (xml::attributes, a_i, t_i->get_attributes()) {
                                if (NStr::Equal(a_i->get_name(), "type")  &&
                                    !NStr::Equal(a_i->get_value(), "bad")) {
                                    data.m_Subtypes.insert(string(a_i->get_value()));
                                }
                            }
                        }
                    }
                }
            }

            if ( !data.m_Subtypes.empty() ) {
                // It is feature table
                // We will use all feature subtypes
                string ftable =
                    CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Ftable);
                data.m_AnnotType = ftable;
            } else {
                vector<string> priorytized_types;
                priorytized_types.push_back(
                    CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Align));
                priorytized_types.push_back(
                    CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Seq_table));
                priorytized_types.push_back(
                    CSeq_annot::TData::SelectionName(CSeq_annot::TData::e_Graph));

                ITERATE(vector<string>, type_iter, priorytized_types) {
                    set<string>::const_iterator at_iter = annot_types.find(*type_iter);
                    if (at_iter != annot_types.end()) {
                        data.m_AnnotType = *type_iter;
                        break;
                    }
                }
            }
        }
    }
}


void CNAUtils::x_GetNAEntrezLinks(CAnnotMetaData& data) const
{
    // LOG_POST("<<<<");
    CSeqUtils::TUids uids_from;
    uids_from.push_back(NStr::StringToNonNegativeInt(data.m_Id));

    xml::document linkset;

    data.m_LinksStatus = CAnnotMetaData::ELinksStatus_Absent;
    try {
        // LOG_POST("Calling eutils for linkset/acheck");
        CSeqUtils::ELinkQuery("seqannot", "all", uids_from, linkset, "acheck");
        // LOG_POST("Call done");
    }
    catch (const CException& e) {
        LOG_POST(Error << "Get error when trying to get Entrez links for NA id: " << data.m_Id << ". Error: " << e.GetMsg());
        return;
    }

    xml::node_set::const_iterator itLinkInfo;
    xml::node_set nodes ( linkset.get_root_node().run_xpath_query("//LinkInfo[DbTo/text()!=\"\"][LinkName/text()!=\"\"][HtmlTag/text()!=\"\"]") );
    for (itLinkInfo = nodes.begin(); itLinkInfo != nodes.end(); ++itLinkInfo) {
        const string html_tag(itLinkInfo->find("HtmlTag")->get_content());
        const string db_to(itLinkInfo->find("DbTo")->get_content());
        const string link_name(itLinkInfo->find("LinkName")->get_content());

        if (db_to == "nuccore")
            continue;

        CSeqUtils::TUids uids;
        try {
            // LOG_POST("Calling eutils for linkset in db_to: " << db_to << ", html_tag: " << html_tag << ", link_name: " << link_name);
            CSeqUtils::ELinkQuery("seqannot", db_to, uids_from, uids);
            // LOG_POST("Call done");
        }
        catch (const CException& e) {
            LOG_POST(Error << "Get error when trying to get Entrez links for link name: " << link_name << ". Error: " << e.GetMsg());
            continue;
        }

        string link("https://www.ncbi.nlm.nih.gov/");
        // Generate a link from all UIDs
        link += db_to + "/" + CSeqUtils::CreateIdStr(uids);
        NON_CONST_ITERATE(CAnnotMetaData::TLinks, c_link, data.m_Links) {
            if (c_link->second == link) {
                data.m_Links.erase(c_link);
                break;
            }
        }
        data.m_Links[html_tag] = link;
        data.m_LinksStatus = CAnnotMetaData::ELinksStatus_Present;
    }
    // LOG_POST(">>>>");
}


END_NCBI_SCOPE
