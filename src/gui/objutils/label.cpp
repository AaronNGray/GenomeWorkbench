/*  $Id: label.cpp 45023 2020-05-08 20:34:57Z grichenk $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <serial/enumvalues.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/user_type.hpp>
#include <gui/objutils/taxid_sel_set.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/utils/string_utils.hpp>
#include <map>
#include <corelib/ncbimtx.hpp>
#include <util/static_map.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Spliced_seg.hpp>

#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Variation_inst.hpp>
#include <objects/seqfeat/Delta_item.hpp>

#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/Clone_seq_set.hpp>
#include <objects/seqfeat/Clone_seq.hpp>
#include <objects/seqfeat/VariantProperties.hpp>
#include <objects/seqfeat/Phenotype.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seq/Seq_literal.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_point.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqset/Bioseq_set.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>

#include <objects/valerr/ValidError.hpp>
#include <objects/valerr/ValidErrItem.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/DistanceMatrix.hpp>
#include <objects/genomecoll/GC_AssemblyDesc.hpp>

#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>

#include <objects/variation/Variation.hpp>
#include <objects/gbproj/GBProject_ver2.hpp>
#include <objects/gbproj/ProjectDescr.hpp>
#include <objects/gbproj/ProjectFolder.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/util/create_defline.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objmgr/align_ci.hpp>

#include <serial/iterator.hpp>
#include <algo/align/util/score_builder.hpp>

#include <objtools/snputil/snp_bitfield.hpp>
#include <objtools/snputil/snp_utils.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


void CLabel::SetLabelByData(objects::CProjectItem& item, objects::CScope* scope)
{
	string label = "No data";
    if (item.IsSetItem()) {
        const CSerialObject* obj = item.GetObject();
        if(obj) {
			label.clear();
            GetLabel(*obj, &label, CLabel::eDefault, scope);
        }
    }
	item.SetLabel(label);
}


DEFINE_STATIC_MUTEX(sm_Mutex);
CLabel::TLabelMap CLabel::sm_LabelMap;
CLabel::TAliasMap CLabel::sm_AliasMap;


void CLabel::GetLabel(
	const CObject& obj, string* label, ELabelType type
){
	const CSerialObject* so = dynamic_cast<const CSerialObject*>(&obj);
	if( so ){
		TTypeInfo type_info = so->GetThisTypeInfo();
		_ASSERT(type_info != CSeq_id::GetTypeInfo());
		_ASSERT(type_info != CSeq_loc::GetTypeInfo());
		_ASSERT(type_info != CSeq_entry::GetTypeInfo());
		_ASSERT(type_info != CBioseq::GetTypeInfo());
		_ASSERT(type_info != CBioseq_set::GetTypeInfo());
	}

	CLabel::GetLabel( obj, label, type, NULL );
}

void CLabel::GetLabel(
	const CObject& obj, string* label, ELabelType type, CScope* scope
){
    if( !label ){
        return;
    }

    try {
        const CSerialObject* so = dynamic_cast<const CSerialObject*>(&obj);
        if (so) {
            TLabelMap::const_iterator iter =
                sm_LabelMap.find(so->GetThisTypeInfo()->GetName());
            if (iter != sm_LabelMap.end()) {
                iter->second->GetLabel(obj, label, type, scope);
            } else {
                *label = "[" + so->GetThisTypeInfo()->GetName() + "]";
            }
        } else {
            TLabelMap::const_iterator iter =
                sm_LabelMap.find(typeid(obj).name());
            if (iter != sm_LabelMap.end()) {
                iter->second->GetLabel(obj, label, type, scope);
            } else {
                *label = "[" + string(typeid(obj).name()) + "]";
            }
        }
    } catch( CException& e ){
        LOG_POST(Error << "CLabel::GetLabel(): error: " << e.GetMsg());
    }
}

void CLabel::TruncateLabel(string*  label,
                           unsigned length_limit,
                           unsigned length_tolerance)
{
    if (!label) return;
    if (label->empty()) return;
    size_t len = label->length();
    if (len <= length_limit) return; // nothing to do

    // scan the label to find the nearest meaningful truncate point
    bool point_found = false;
    size_t i = length_limit;
    size_t max_len = min(len, size_t(length_limit + length_tolerance));
    for (; i < max_len; ++i) {
        char c = (*label)[i];
        if (c == ';' || c == '.') {
            point_found = true;
            break;
        }
    } // for

    if ( !point_found ) {
        i = length_limit;
        for (;i < max_len; ++i) {
            char c = (*label)[i];
            if (c == ' ' || c == ',') {
                point_found = true;
                break;
            }
        } // for
    }

    if ( !point_found ) {
        i = length_limit;
    }

    label->erase(i);
    label->append("...");
}



void CLabel::RegisterLabelHandler( const string& type, ILabelHandler& handler )
{
    CMutexGuard LOCK(sm_Mutex);
    sm_LabelMap[type] = CRef<ILabelHandler>(&handler);
}


void CLabel::RegisterLabelHandler( const CTypeInfo& type, ILabelHandler& handler )
{
    CMutexGuard LOCK(sm_Mutex);
    sm_LabelMap[type.GetName()] = CRef<ILabelHandler>(&handler);
}


bool CLabel::HasHandler( const CTypeInfo& type )
{
    return HasHandler( type.GetName() );
}


bool CLabel::HasHandler( const string& type )
{
    CMutexGuard LOCK(sm_Mutex);
    TLabelMap::const_iterator iter = sm_LabelMap.find(type);
    return (iter != sm_LabelMap.end());
}


void CLabel::RegisterTypeIcon(const string& user_type,
                              const string& user_subtype,
                              const string& icon_alias)
{
    string key = user_type + "&" + user_subtype;

    CMutexGuard LOCK(sm_Mutex);
    sm_AliasMap[key] = icon_alias;
}


string CLabel::GetTypeIconAlias(const string& user_type, const string& user_subtype)
{
    string key = user_type + "&" + user_subtype;

    CMutexGuard LOCK(sm_Mutex);

    TAliasMap::const_iterator it = sm_AliasMap.find(key);
    if(it != sm_AliasMap.end()) {
        return it->second;
    } else if( ! user_subtype.empty())  {
        // ignore subtype
        key = user_type;
        key += "&";
        it = sm_AliasMap.find(key);
        if(it != sm_AliasMap.end()) {
            return it->second;
        }
    }
    return "";
}


string CLabel::GetTypeIconAlias(const string& user_type)
{
    string key = user_type + "&";

    CMutexGuard LOCK(sm_Mutex);
    TAliasMap::const_iterator it = sm_AliasMap.find(key);
    return (it != sm_AliasMap.end()) ? it->second : "";
}


void CLabel::GetIconAlias(const CObject& obj, string* alias,
                            objects::CScope* scope)
{
    if(alias)   {
        string type, subtype;
        GetLabel(obj, &type, eUserType, scope);
        GetLabel(obj, &subtype, eUserSubtype, scope);
        *alias = GetTypeIconAlias(type, subtype);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Description cache (singleton)

class CLabelDescriptionCacheImpl
{
public:
    typedef map<TIntId, string>  TGi2DescriptionMap;
public:
    CLabelDescriptionCacheImpl() {}

    string GetLabel(TGi gi) const
    {
        CFastMutexGuard lock(m_Lock);
        TGi2DescriptionMap::const_iterator it = m_DescrCache.find(GI_TO(TIntId, gi));
        if (it == m_DescrCache.end()) {
            return kEmptyStr;
        }
        return it->second;
    }
    void GetLabel(TGi gi, string* label) const
    {
        CFastMutexGuard lock(m_Lock);
        TGi2DescriptionMap::const_iterator it = m_DescrCache.find(GI_TO(TIntId, gi));
        if (it == m_DescrCache.end()) {
            label->clear();
            return;
        }
        *label = it->second;
    }
    bool HasLabel(TGi gi) const
    {
        CFastMutexGuard lock(m_Lock);
        TGi2DescriptionMap::const_iterator it = m_DescrCache.find(GI_TO(TIntId, gi));
        if (it == m_DescrCache.end()) {
            return false;
        }
        return true;
    }

    void SetLabel(TGi gi, const string& label)
    {
        CFastMutexGuard lock(m_Lock);
        m_DescrCache[GI_TO(TIntId, gi)] = label;
    }

    void AddCaches(const map<TGi, string>& caches)
    {
        CFastMutexGuard lock(m_Lock);
        map<TGi, string>::const_iterator iter = caches.begin();
        for (; iter != caches.end(); ++iter) {
            m_DescrCache[GI_TO(TIntId, iter->first)] = iter->second;
        }
    }

private:
    mutable CFastMutex   m_Lock;
    TGi2DescriptionMap   m_DescrCache;
};

static std::auto_ptr<CLabelDescriptionCacheImpl> s_DescrCachePtr;

void CLabelDescriptionCache::InitCache()
{
    if (s_DescrCachePtr.get() != 0) {
        return; // double initialization
    }
    s_DescrCachePtr.reset(new CLabelDescriptionCacheImpl());
}

void CLabelDescriptionCache::StopCache()
{
    s_DescrCachePtr.reset(0);
}


static
TGi s_GetGi( const CSeq_id& id, CScope* scope )
{
	// check: scope is NEEDED [protected]

	TGi gi = ZERO_GI;
    try {
        if( id.IsLocal() ) return gi;

        if( id.IsGi() ){
            gi = id.GetGi();
        }
        if( (gi == ZERO_GI) && scope ){
            CSeq_id_Handle bsh = sequence::GetId( id, *scope );
            bsh = sequence::GetId( bsh, *scope, sequence::eGetId_ForceGi );
            if( bsh ){
                gi = bsh.GetGi();
            }
        }
    } catch( std::exception& ){
        gi = ZERO_GI;
    }

    return gi;
}

static CFastMutex s_DFLock;
static sequence::CDeflineGenerator s_DefGen; ///< This class seems to be non-thread safe even as a stack var

static
inline void s_GetDefline( const CSeq_id& id, CScope* scope, string* label )
{
	// check: scope is NEEDED [protected]

	if( scope ){
        CBioseq_Handle bsh = scope->GetBioseqHandle( id );
        if( bsh ){
            CFastMutexGuard lock(s_DFLock);
            *label = s_DefGen.GenerateDefline( bsh );
        }
    }
}

static
void s_GetCachedDescription( const CSeq_id& id, CScope* scope, string* label )
{
	// check: scope is NEEDED [indirectly protected]

	TGi gi = ZERO_GI;

    if( s_DescrCachePtr.get() != 0 ){ // check if cache is ON
        gi = s_GetGi( id, scope );
        if( gi != ZERO_GI ){  // gi labels get cached
            s_DescrCachePtr->GetLabel( gi, label );
            if( !label->empty() ){
                return;
            }
        }
    }

    // no cache, try sequence defline first
    s_GetDefline( id, scope, label );

    if( label->empty() ){
        // use regular label
        id.GetLabel( label, CSeq_id::eBoth, CSeq_id::fLabel_Default );
    }

    if( gi != ZERO_GI ){
        s_DescrCachePtr->SetLabel(gi, *label);
    }
}

void CLabelDescriptionCache::CacheSeqId(const CSeq_id& id, CScope* scope)
{
	// check: scope is NEEDED [indirectly protected]

    string fastaLabel;

    try {
	    if( s_DescrCachePtr.get() != 0 ){ // check if cache is ON
            fastaLabel = id.AsFastaString();
            TGi gi = s_GetGi( id, scope );
            if( (gi != ZERO_GI) && !s_DescrCachePtr->HasLabel(gi) ){
                string label;
                s_GetDefline( id, scope, &label );
                if( !label.empty() ){
                    s_DescrCachePtr->SetLabel( gi, label );
                }
            }
        }
    } NCBI_CATCH("Failed to load Seq-id: " + fastaLabel + ".");
}

void CLabelDescriptionCache::AddCaches(const map<TGi, string>& caches)
{
    if (s_DescrCachePtr.get() != 0) { // check if cache is ON
        s_DescrCachePtr->AddCaches(caches);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Label Handlers

// default format function for a seq-id
// this is used by seq-id and seq-loc formatting
static inline void s_GetSeqLabel(const CSeq_id& id, string* label,
                                 CScope* scope, CLabel::ELabelType type)
{
	// check: scope is NEEDED [protected]

    CConstRef<CSeq_id> id_ptr(&id);
    if( scope && id_ptr->IsGi() ){
        CSeq_id_Handle sih = sequence::GetId(id, *scope, sequence::eGetId_Best);
        if( sih ){
            id_ptr = sih.GetSeqId();
        }
    }

    switch( type ){
    case CLabel::eUserType:
        *label += CGUIUserType::sm_Tp_Sequence_ID;
        break;

    case CLabel::eUserSubtype:
        {{
            CSeq_id::EAccessionInfo info = id_ptr->IdentifyAccession();
            if( info & CSeq_id::fAcc_nuc ){
                *label += CGUIUserType::sm_SbTp_DNA;

			} else if( info & CSeq_id::fAcc_prot ){
                *label += CGUIUserType::sm_SbTp_Protein;
            }
        }}
        break;

    case CLabel::eType:
        *label += id_ptr->GetThisTypeInfo()->GetName();
        break;

    default:
    case CLabel::eContent:
        id_ptr->GetLabel(label, CSeq_id::eContent, CSeq_id::fLabel_Default);
        break;

    case CLabel::eUserTypeAndContent:
        id_ptr->GetLabel(label, CSeq_id::eBoth, CSeq_id::fLabel_Default);
        break;

    case CLabel::eDescriptionBrief:
        *label += id_ptr->GetSeqIdString();
        break;
    case CLabel::eDescription:
        s_GetCachedDescription(*id_ptr, scope, label);
        break;

	case CLabel::eGi:
		 TGi gi = s_GetGi(id, scope);
		 if (gi != ZERO_GI) {
			*label+="gi|";
			*label+=NStr::NumericToString(gi);
		 }

		 break;
    }
}


//
// process a seq-interval
//
static inline
void s_GetSeq_intervalLabel(const CSeq_interval& interval, string* label,
                            CScope* scope, CLabel::ELabelType type,
                            const CSeq_id* last_id = NULL)
{
	// check: scope is NEEDED [indirectly protected]

    switch (type) {
    case CLabel::eUserType:
    case CLabel::eUserSubtype:
        break;

    case CLabel::eType:
        *label += interval.GetThisTypeInfo()->GetName();
        break;

    default:
    case CLabel::eContent:
    case CLabel::eUserTypeAndContent:
        if ( !last_id  ||  !interval.GetId().Match(*last_id)) {
            if ( !label->empty() ) {
                *label += "; ";
            }
            s_GetSeqLabel(interval.GetId(), label, scope, type);
            *label += ": ";
        }

        *label += NStr::IntToString(interval.GetFrom() + 1, NStr::fWithCommas);
        *label += "-";
        *label += NStr::IntToString(interval.GetTo() + 1, NStr::fWithCommas);

        *label += " [";
        *label += NStr::IntToString(interval.GetLength(), NStr::fWithCommas);
        *label += "]";

        if (interval.IsSetStrand()) {
            switch (interval.GetStrand()) {
            case eNa_strand_plus:
                *label += " (+)";
                break;
            case eNa_strand_minus:
                *label += " (-)";
                break;
            default:
                *label += " (+)";
                break;
            }
        }
        break;
    }
}

static inline
void s_GetSeq_TotalRangeLabel(const CSeq_loc& loc, string* label,
                              CScope* scope, CLabel::ELabelType type)
{
	// check: scope is NEEDED [indirectly protected]

	switch (type) {
    case CLabel::eUserType:
        *label += CGUIUserType::sm_Tp_Location;
        break;

    case CLabel::eUserSubtype:
        break;

    case CLabel::eType:
        *label += loc.GetThisTypeInfo()->GetName();
        break;

    default:
    case CLabel::eContent:
    case CLabel::eUserTypeAndContent:
        {{
            const CSeq_id& id = sequence::GetId(loc, scope);
            TSeqRange range = loc.GetTotalRange();
            s_GetSeqLabel(id, label, scope, type);
            *label += ": ";
            *label += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
            *label += "-";
            *label += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);

            *label += " [";
            *label += NStr::IntToString(range.GetLength(), NStr::fWithCommas);
            *label += "]";

            string extra_info = kEmptyStr;
            ENa_strand strand = sequence::GetStrand(loc, scope);
            switch (strand) {
            case eNa_strand_minus:
                extra_info = "-";
                break;
            case eNa_strand_unknown:
                break;
            case eNa_strand_plus:
            default:
                extra_info = "+";
                break;
            }

            size_t intervals = 0;
            switch (loc.Which()) {
            case CSeq_loc::e_Int:
                intervals = 1;
                break;

            case CSeq_loc::e_Packed_int:
                intervals = loc.GetPacked_int().Get().size();
                break;

            case CSeq_loc::e_Packed_pnt:
                intervals = loc.GetPacked_pnt().GetPoints().size();
                break;

            case CSeq_loc::e_Mix:
                /// FIXME: this may not always be correct -
                /// a mix may be a mix of complex intervals
                intervals = loc.GetMix().Get().size();
                break;

            default:
                break;
            }

            if (intervals != 0) {
                if ( !extra_info.empty() )
                    *label += ", ";
                extra_info += NStr::SizetToString(intervals);
                extra_info += " interval";
                if (intervals != 1) {
                    extra_info += "s";
                }
            }
            if ( !extra_info.empty() ) {
                *label += " (";
                *label += extra_info;
                *label += ")";
            }
        }}
        break;
    }
}

static void s_BreakSeq_loc(const CSeq_loc& loc, vector<CRef<CSeq_loc> >& locs, CScope* scope)
{
	// check: scope is NEEDED [indirectly protected]

	for (CSeq_loc_CI it(loc, CSeq_loc_CI::eEmpty_Allow); it; ++it) {
        const CSeq_id& id = it.GetSeq_id();

        if (id.Which() == CSeq_id::e_not_set) {
            continue;
        }

        unsigned int i;
        for (i = 0; i < locs.size(); ++i) {
            if (sequence::IsSameBioseq(*locs[i]->GetId(), it.GetSeq_id(), scope, CScope::eGetBioseq_All))
                break;
        }

        if (i == locs.size()) {
            locs.push_back(CRef<CSeq_loc>(new CSeq_loc()));
        }

        locs[i]->Add(*it.GetRangeAsSeq_loc());
    }
}

//
// internal processing of seq-loc labels
// seq-locs frequently refer to GIs only - we need to use sequence accessions
// to make a more friendly label
//
static inline
const CSeq_id* s_GetLabel(const CSeq_loc& loc, string* label,
                          CScope* scope, CLabel::ELabelType type,
                          const CSeq_id* last_id = NULL)
{
	// check: scope is NEEDED [indirectly protected]

	switch (type) {
    case CLabel::eType:
        *label += loc.GetThisTypeInfo()->GetName();
        switch (loc.Which()) {
        case CSeq_loc::e_Int:
            *label += ".int";
            break;
        case CSeq_loc::e_Packed_int:
            *label += ".packed-int";
            break;
        case CSeq_loc::e_Mix:
            *label += ".mix";
            break;
        case CSeq_loc::e_Whole:
            *label += ".whole";
            break;
        case CSeq_loc::e_Null:
            *label += ".null";
            break;

        default:
            break;
        }
        return last_id;

    case CLabel::eUserType:
        *label += CGUIUserType::sm_Tp_Location;
        return last_id;

    case CLabel::eUserSubtype:
        /*switch (loc.Which()) {
        case CSeq_loc::e_Int:
            *label += "Interval";
            break;
        case CSeq_loc::e_Packed_int:
            *label += "Intervals";
            break;
        case CSeq_loc::e_Mix:
            *label += "Mixed Location";
            break;
        case CSeq_loc::e_Whole:
            *label += "Location";
            break;

        default:
            *label += "Location";
            break;
        }*/
        return last_id;

    default:
        break;
    }

    switch (loc.Which()) {
    case CSeq_loc::e_Whole:
        s_GetSeqLabel(loc.GetWhole(), label, scope, type);
        return &loc.GetWhole();

    case CSeq_loc::e_Int:
        s_GetSeq_intervalLabel(loc.GetInt(), label, scope, type, last_id);
        return &loc.GetInt().GetId();

    case CSeq_loc::e_Packed_int:
        switch (type) {
        case CLabel::eDescription:
        case CLabel::eDescriptionBrief:
            {{
                CConstRef<CSeq_id> id(last_id);
                string str;
                ITERATE (CSeq_loc::TPacked_int::Tdata, iter,
                        loc.GetPacked_int().Get()) {
                    const CSeq_interval& ival = **iter;
                    if ( !str.empty() ) {
                        str += ", ";
                    }
                    s_GetSeq_intervalLabel(ival, &str, scope, type, id.GetPointer());
                    id.Reset(&ival.GetId());
                }
                *label += str;
                return id.GetPointer();
            }}

        default:
            s_GetSeq_TotalRangeLabel(loc, label, scope, type);
            return last_id;
        }

    case CSeq_loc::e_Pnt:
        {{
            string str = NStr::IntToString(loc.GetPnt().GetPoint() + 1, NStr::fWithCommas);
            if ( !last_id  ||  !loc.GetPnt().GetId().Match(*last_id) ) {
                if ( !label->empty() ) {
                    *label += "; ";
                }
                s_GetSeqLabel(loc.GetPnt().GetId(), label, scope, type);
                *label += ": ";
            } else if ( !label->empty() ) {
                *label += ", ";
            }
            *label += str;
            return &loc.GetPnt().GetId();
        }}

    case CSeq_loc::e_Packed_pnt:
        switch (type) {
        case CLabel::eDescription:
        case CLabel::eDescriptionBrief:
            {{
                string str;
                ITERATE (CSeq_loc::TPacked_pnt::TPoints, iter,
                        loc.GetPacked_pnt().GetPoints()) {
                    if ( !str.empty() ) {
                        str += ", ";
                    }
                    str += NStr::IntToString(*iter, NStr::fWithCommas);
                }

                if ( !last_id  ||  !loc.GetPnt().GetId().Match(*last_id) ) {
                    if ( !label->empty() ) {
                        *label += "; ";
                    }
                    s_GetSeqLabel(loc.GetPnt().GetId(), label, scope, type);
                    *label += ": ";
                } else if ( !label->empty() ) {
                    *label += ", ";
                }
                *label += str;
                return &loc.GetPnt().GetId();
            }}

        default:
            s_GetSeq_TotalRangeLabel(loc, label, scope, type);
            return last_id;
        }

    case CSeq_loc::e_Mix:
        switch (type) {
        case CLabel::eDescription:
        case CLabel::eDescriptionBrief:
            {{
                string str;
                ITERATE (CSeq_loc::TMix::Tdata, iter, loc.GetMix().Get()) {
                    if ( !str.empty() ) {
                        str += ", ";
                    }
                    last_id = s_GetLabel(**iter, &str, scope, type, last_id);
                }
                *label += str;
                return last_id;
            }}

        default:
            if (sequence::IsOneBioseq(loc, scope)) {
                s_GetSeq_TotalRangeLabel(loc, label, scope, type);
            } else {
                vector<CRef<CSeq_loc> > locs;
                s_BreakSeq_loc(loc, locs, scope);

                for (unsigned int i = 0; i < locs.size(); ++i) {
                    if (i > 0)
                        *label += ";";
                    s_GetSeq_TotalRangeLabel(*locs[i], label, scope, type);
                }
            }
            return last_id;
        }

    default:
        loc.GetLabel(label);
        return NULL;
    }
}


//
//
// seq-id handler
//
//
class CSeq_idHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
        const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
        if (id) {
            s_GetSeqLabel(*id, label, scope, type);
        }
    }
};


//
//
// seq-loc handler
//
//
class CSeq_locHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
        if (loc) {
            string s;
            s_GetLabel(*loc, &s, scope, type);
            *label += s;
        }
    }
};



//
//
// bioseq handler
//
//
class CBioseqHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* /*scope*/) const
    {
        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(&obj);
        if (bioseq) {
            switch (type) {
            case CLabel::eUserType:
                *label += CGUIUserType::sm_Tp_Sequence;
                break;

            case CLabel::eUserSubtype:
                if(bioseq->IsNa())  {
                    *label += CGUIUserType::sm_SbTp_DNA;
                } else if(bioseq->IsAa()) {
                    *label += CGUIUserType::sm_SbTp_Protein;
                }
                break;

            case CLabel::eType:
                bioseq->GetLabel(label, CBioseq::eType);
                break;
            case CLabel::eContent:
                bioseq->GetLabel(label, CBioseq::eContent);
                break;

            default:
            case CLabel::eUserTypeAndContent:
            case CLabel::eDescription:
            case CLabel::eDescriptionBrief:
                bioseq->GetLabel(label, CBioseq::eBoth);
                break;
            }
        }
    }
};


//
// Bioseq_set handler
//
static void s_GetBioseq_setType(const CBioseq_set& bst, string* label)
{
    // NB: This table should be kept in synch with CBioseq_set::EClass values
    typedef SStaticPair<CBioseq_set::TClass, const char*>  TPair;
    static const TPair sc_Types[] = {
        {CBioseq_set::eClass_not_set,      "not determined"},
        {CBioseq_set::eClass_nuc_prot,     "nuc-prot"},
        {CBioseq_set::eClass_segset,       "segmented"},
        {CBioseq_set::eClass_conset,       "constructed"},
        {CBioseq_set::eClass_parts,        "parts"},
        {CBioseq_set::eClass_gibb,         "geninfo backbone"},
        {CBioseq_set::eClass_gi,           "geninfo"},
        {CBioseq_set::eClass_genbank,      "genbank"},
        {CBioseq_set::eClass_pir,          "pir"},
        {CBioseq_set::eClass_pub_set,      "publication"},
        {CBioseq_set::eClass_equiv,        "equivalent maps or seqs"},
        {CBioseq_set::eClass_swissprot,    "swissprot"},
        {CBioseq_set::eClass_pdb_entry,    "pdb"},
        {CBioseq_set::eClass_mut_set,      "mutations"},
        {CBioseq_set::eClass_pop_set,      "population study"},
        {CBioseq_set::eClass_phy_set,      "phylogenetic study"},
        {CBioseq_set::eClass_eco_set,      "ecological sample study"},
        {CBioseq_set::eClass_gen_prod_set, "genomic products"},
        {CBioseq_set::eClass_wgs_set,      "wgs project"},
        {CBioseq_set::eClass_other,        "other"},
    };
    typedef CStaticArrayMap<CBioseq_set::TClass, const char*> TBioseq_setTypes;
    DEFINE_STATIC_ARRAY_MAP(TBioseq_setTypes, sc_Bioseq_setTypes, sc_Types);

    TBioseq_setTypes::const_iterator i = sc_Bioseq_setTypes.find(bst.GetClass());
    *label += (i != sc_Bioseq_setTypes.end()) ?
        sc_Bioseq_setTypes.find(bst.GetClass())->second :
        CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(bst.GetClass(), true);
}


static void s_GetIdLabel(const CBioseq_set& bst, string* label, CScope* scope)
{
    if (bst.GetSeq_set().empty()) {
        return;
    }
    const CBioseq* lcl = NULL;
    vector<string> ids;

    ITERATE (CBioseq_set::TSeq_set, it, bst.GetSeq_set()) {
        const CBioseq* bsp = NULL;
        if ((*it)->IsSeq()) {
            bsp = &(*it)->GetSeq();
        } else if ((*it)->IsSet()) {
            const CBioseq_set& bss = (*it)->GetSet();
            try {
                switch (bss.GetClass()) {
                case CBioseq_set::eClass_nuc_prot:
                    bsp = &bss.GetNucFromNucProtSet();
                    break;
                case CBioseq_set::eClass_segset:
                    bsp = &bss.GetMasterFromSegSet();
                    break;
                default:
                    break;
                }
            } catch (CException&) {
                bsp = NULL;
            }
        }
        if (bsp != NULL) {
            CRef<CSeq_id> id = FindBestChoice(bsp->GetId(), CSeq_id::Score);
            const CTextseq_id* tsip = id->GetTextseq_Id();
            if (tsip != NULL  &&  tsip->IsSetAccession()) {
                ids.push_back(tsip->GetAccession());
            } else if (id->IsLocal()  &&  id->GetLocal().IsStr()) {
                if (lcl == NULL) {
                    lcl = bsp;
                }
            }
        }
    }

    sort(ids.begin(), ids.end());
    vector<string>::iterator new_end = unique(ids.begin(), ids.end());
    if (new_end != ids.end()) {
        ids.erase(new_end);
    }

    if (!ids.empty()) {
        *label += ids.front();
        if (ids.size() > 1) {
            *label += '-';
            *label += ids.back();
        }
    } else if (lcl != NULL) {
        CLabel::GetLabel(*lcl, label, CLabel::eContent, scope);
    }
}


static void s_GetNucProtContent(const CBioseq_set& bst, string* label, CScope* scope)
{
    _ASSERT(bst.GetClass() == CBioseq_set::eClass_nuc_prot);

    try {
        const CBioseq& seq = bst.GetNucFromNucProtSet();
        CLabel::GetLabel(seq, label, CLabel::eContent, scope);
    } catch (CException&) {}
}


static void s_GetSegsetContent(const CBioseq_set& bst, string* label, CScope* scope)
{
    _ASSERT(bst.GetClass() == CBioseq_set::eClass_segset);

    try {
        const CBioseq& seq = bst.GetMasterFromSegSet();
        CLabel::GetLabel(seq, label, CLabel::eContent, scope);
    } catch (CException&) {}
}

static void s_GetGenProdContent(const CBioseq_set& bst, string* label, CScope* scope)
{
    _ASSERT(bst.GetClass() == CBioseq_set::eClass_gen_prod_set);

    try {
        const CBioseq& seq = bst.GetGenomicFromGenProdSet();
        CLabel::GetLabel(seq, label, CLabel::eContent, scope);
    } catch (CException&) {}
}

static void s_GetBioseq_setContent(const CBioseq_set& bst, string* label, CScope* scope)
{
    switch(bst.GetClass()) {
    case CBioseq_set::eClass_nuc_prot:
        s_GetNucProtContent(bst, label, scope);
        break;

    case CBioseq_set::eClass_segset:
        s_GetSegsetContent(bst, label, scope);
        break;

    case CBioseq_set::eClass_mut_set:
    case CBioseq_set::eClass_pop_set:
    case CBioseq_set::eClass_phy_set:
    case CBioseq_set::eClass_eco_set:
        s_GetIdLabel(bst, label, scope);
        break;

    case CBioseq_set::eClass_gen_prod_set:
        s_GetGenProdContent(bst, label, scope);
        break;

    case CBioseq_set::eClass_genbank:
        // if just a wrapper return the label of the wrapped Seq-entry
        if (bst.GetSeq_set().size() == 1) {
            CLabel::GetLabel(*bst.GetSeq_set().front(), label, CLabel::eContent, scope);
        } else {
            bst.GetLabel(label, CBioseq_set::eContent);
        }
        break;

    default:
        bst.GetLabel(label, CBioseq_set::eContent);
        break;
    };
}

static bool s_HasAlignment(const CBioseq_set& bst, CScope* scope)
{
	// check: scope is NEEDED [protected]

	if( scope ){
        CBioseq_set_Handle bsh = scope->GetBioseq_setHandle(bst);
        if( bsh ){
            CSeq_entry_Handle tse = bsh.GetTopLevelEntry();

            SAnnotSelector sel;
            sel.SetMaxSize(1);
            sel.SetLimitTSE(tse);
            CAlign_CI align_it(tse, sel);
            if (align_it.GetSize() != 0) {
                return true;
            }
        }
    }
    return false;
}


class CBioseq_setHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
		// check: scope is NEEDED [indirectly protected]

		if( !label ) return;


        const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(&obj);
        if (bioseq_set) {
            switch (type) {
            case CLabel::eUserType:
                *label += CGUIUserType::sm_Tp_Sequence_Set;
                break;

            case CLabel::eUserSubtype:  {
                switch(bioseq_set->GetClass())  {
                case CBioseq_set::eClass_nuc_prot:
                    *label += CGUIUserType::sm_SbTp_NucProt;
                    break;
                case CBioseq_set::eClass_segset:
                    *label += CGUIUserType::sm_SbTp_SegSeq;
                    break;
                case CBioseq_set::eClass_gen_prod_set:
                    *label += CGUIUserType::sm_SbTp_GenProdSet;
                    break;
                case CBioseq_set::eClass_pop_set:   {
                    bool aln = s_HasAlignment(*bioseq_set, scope);
                    *label += aln   ? CGUIUserType::sm_SbTp_PopSet_Aln
                                    : CGUIUserType::sm_SbTp_PopSet;
                    break;
                }
                case CBioseq_set::eClass_phy_set: {
                    bool aln = s_HasAlignment(*bioseq_set, scope);
                    *label += aln   ? CGUIUserType::sm_SbTp_PhySet_Aln
                                    : CGUIUserType::sm_SbTp_PhySet;
                    break;
                }
                case CBioseq_set::eClass_eco_set:   {
                    bool aln = s_HasAlignment(*bioseq_set, scope);
                    *label += aln   ? CGUIUserType::sm_SbTp_EcoSet_Aln
                                    : CGUIUserType::sm_SbTp_EcoSet;
                    break;
                }
                case CBioseq_set::eClass_mut_set:   {
                    bool aln = s_HasAlignment(*bioseq_set, scope);
                    *label += aln   ? CGUIUserType::sm_SbTp_MutSet_Aln
                                    : CGUIUserType::sm_SbTp_MutSet;
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case CLabel::eType:
                s_GetBioseq_setType(*bioseq_set, label);
                break;
            case CLabel::eContent:
                s_GetBioseq_setContent(*bioseq_set, label, scope);
                break;

            default:
            case CLabel::eUserTypeAndContent:
            case CLabel::eDescriptionBrief:
            case CLabel::eDescription:
                GetLabel(*bioseq_set, label, CLabel::eType, scope);
                *label += ": ";
                GetLabel(*bioseq_set, label, CLabel::eContent, scope);
                break;
            }
        }
    }
};

/// handler specific for User-object features
class CUserFeatHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj_in, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
        if (label == NULL) {
            return;
        }

        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj_in);
        if (feat) {
            _ASSERT(feat->GetData().IsUser());
            const CUser_object& obj = feat->GetData().GetUser();
            const CObject_id& id = obj.GetType();

            switch (type) {
            case CLabel::eUserType:
                *label += CGUIUserType::sm_Tp_Feature;
                break;

            case CLabel::eUserSubtype:
                if (id.IsStr()) {
                    *label += id.GetStr();
                    break;
                } else if (obj.IsSetClass()) {
                    *label += obj.GetClass();
                    break;
                }
                break;

            case CLabel::eType:
                if (id.IsStr()) {
                    *label += id.GetStr();
                    break;
                } else if (obj.IsSetClass()) {
                    *label += obj.GetClass();
                    break;
                }
                break;

            case CLabel::eUserTypeAndContent:
            case CLabel::eDescriptionBrief:
            case CLabel::eDescription:
            case CLabel::eContent:
            default:
                /// for content, we use the product seq-id, if it
                /// is available; otherwise, we use the standard text
                if (id.IsStr()  &&  id.GetStr() == "clone placement") {
                    *label += '[';
                    *label += id.GetStr();
                    *label += ']';
                    ///
                    /// clone placement user objects
                    ///
                    CConstRef<CUser_field> field;

                    field = obj.GetFieldRef("clone placement.name");
                    if (field) {
                        if ( !label->empty() ) {
                            *label += ' ';
                        }
                        *label += "/name=";
                        *label += field->GetData().GetStr();
                    }

                    field = obj.GetFieldRef("clone placement.type");
                    if (field) {
                        if ( !label->empty() ) {
                            *label += ' ';
                        }
                        *label += "/type=";
                        *label += field->GetData().GetStr();
                    }

                    field = obj.GetFieldRef("clone placement.concordant");
                    if (field) {
                        if ( !label->empty() ) {
                            *label += ' ';
                        }
                        *label += "/concordant=";
                        *label += field->GetData().GetBool() ? "true" : "false";
                    }
                } else if (id.IsStr()  &&
                    NStr::EqualNocase(id.GetStr(), "Display Data")  &&
                    obj.GetFieldRef("name")) {
                    *label += obj.GetFieldRef("name")->GetData().GetStr();
                } else {
                    /// default handler - the above steps exit early
                    feature::GetLabel(*feat, label, feature::fFGL_Content, scope);
                }
                break;
            }
        }
    }
};


static int s_GetChildNum(const objects::CSeq_feat& feat)
{
    string num_str = feat.GetNamedQual("Child Count");
    if ( !num_str.empty() ) {
        try {
            return NStr::StringToInt(num_str);
        } catch (CException&) {
            // ingore it
        }
    }
    return -1;
}


static bool s_IsLOH(const CVariation_ref& var)
{
    if (var.CanGetConsequence()) {
        ITERATE (CVariation_ref::TConsequence, iter, var.GetConsequence()) {
            if ((*iter)->IsLoss_of_heterozygosity()) {
                return true;
            }
        }
    }
    return false;
}


static bool s_IsCopyNumberMethod(const CVariation_ref::TMethod& method)
{
    ITERATE (CVariation_ref::TMethod, iter, method) {
        switch (*iter) {
            case CVariation_ref::eMethod_E_unknown:
            case CVariation_ref::eMethod_E_sequencing:
            case CVariation_ref::eMethod_E_maph:
            case CVariation_ref::eMethod_E_mcd_analysis:
            case CVariation_ref::eMethod_E_optical_mapping:
                return true;
            default:
                break;
        }
    }

    return false;
}

// seq-feat handler
class CSeq_featHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
        if (label == NULL) {
            return;
        }

        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
        if ( !feat ) {
            return;
        }
        if (feat->GetData().IsUser()) {
            m_UserHandler.GetLabel(obj, label, type, scope);
            return;
        }

        switch (type) {
        case CLabel::eUserType:
            *label += CGUIUserType::sm_Tp_Feature;
            break;

        case CLabel::eUserSubtype:
            switch (feat->GetData().GetSubtype()) {
            case CSeqFeatData::eSubtype_variation:
                *label += "SNP";
                break;

            case CSeqFeatData::eSubtype_exon:
                *label += "exon";
                break;

            default:
                feature::GetLabel(*feat, label, feature::fFGL_Type, scope);
                break;
            }
            break;

        case CLabel::eType:
            /// for type, we just used the standard text
            switch (feat->GetData().GetSubtype()) {
            case CSeqFeatData::eSubtype_exon:
            case CSeqFeatData::eSubtype_imp:
                feature::GetLabel(*feat, label, feature::fFGL_Type, scope);
                break;

            default:
                *label += feat->GetThisTypeInfo()->GetName();
                break;
            }
            break;

        case CLabel::eContentAndProduct:
            if (feat->IsSetProduct()) {
                try {
                    string s;
                    const CSeq_id& id = sequence::GetId(feat->GetProduct(), scope);
                    s_GetSeqLabel(id, &s, scope, CLabel::eContent);
                    // For proteins, show both the accession and the protein product name.
                    if (CSeqFeatData::eSubtype_cdregion == feat->GetData().GetSubtype()) {
                        string product_str;
                        feature::GetLabel(*feat, &product_str, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                        string type_str;
                        feature::GetLabel(*feat, &type_str, feature::fFGL_Type, scope);
                        // Do not append the product if it contains type information only
                        if (product_str != type_str) {
                            s += ": ";
                            s += product_str;
                        }
                    }
                    *label += s;
                }
                catch (CException&) {
                    feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                }
            }
            break;

        case CLabel::eContent:
            if (x_UseCustomLabel(*feat)) {
                x_GetCustomLabel(*feat, *label);
            }
            /// for content, we use the product seq-id, if it
            /// is available; otherwise, we use the standard text
            else if (feat->IsSetProduct()) {
                try {
                    const CSeq_id& id = sequence::GetId(feat->GetProduct(), scope);
                    s_GetSeqLabel(id, label, scope, CLabel::eContent);
                }
                catch (CException&) {
                    feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                }
            } else {
                switch (feat->GetData().GetSubtype()) {
                case CSeqFeatData::eSubtype_imp:
                    {{ // logic borrowed from misc_feature
                        bool got_label = false;
                        if (feat->IsSetQual()) {
                            ITERATE(CSeq_feat::TQual, it, feat->GetQual()) {
                                if (NStr::EqualNocase((*it)->GetQual(),"Name")  ||
                                    NStr::EqualNocase((*it)->GetQual(),"ID")) {
                                    *label = (*it)->GetVal();
                                    got_label = true;
                                    if (NStr::EqualNocase((*it)->GetQual(),"Name"))
                                        break;
                                }
                            }
                        }
                        if (!got_label) {
                            feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoComments | feature::fFGL_NoQualifiers, scope);
                            *label += " ";
                            if (feat->GetData().GetImp().IsSetDescr()) {
                                *label += feat->GetData().GetImp().GetDescr();
                            }
                        }
                        if (label->size() > 30) *label = label->substr(0, 30) + "...";
                    }}
                    break;

                case CSeqFeatData::eSubtype_site:
                    if (feat->GetData().GetSite() == CSeqFeatData::eSite_other  &&
                        feat->IsSetComment()) {
                        *label += feat->GetComment();
                    } else {
                        feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                    }
                    break;

                case CSeqFeatData::eSubtype_exon:
                    {{
                        *label += "exon";
                        const string& q = feat->GetNamedQual("number");
                        if ( !q.empty() ) {
                            *label += " ";
                            *label += q;
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_variation:
                    {{
                        /// RSID first
                        NSnp::TRsid Rsid(NSnp::GetRsid(*feat));
                        if(Rsid) {
                            *label += "rs";
                            *label += NStr::NumericToString(Rsid);
                        }
                        else {
                            if (label->empty() && feat->IsSetComment()) {
                                *label += feat->GetComment();
                            }
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_variation_ref:
                    {{
                        const CVariation_ref& var = feat->GetData().GetVariation();
                        if (var.CanGetId()) {
                            if (var.GetId().GetTag().IsId()) {
                                *label += var.GetId().GetDb() + "|";
                                *label += NStr::IntToString(var.GetId().GetTag().GetId());
                            } else {
                                *label += var.GetId().GetTag().GetStr();
                            }
                        } else {
                            if (feat->CanGetLocation()) {
                                const CSeq_loc& loc = feat->GetLocation();
                                s_GetSeq_TotalRangeLabel(loc, label, scope, CLabel::eContent);
                            }

                            //feature::GetLabel(*feat, label, feature::fFGL_Both, scope);
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_misc_feature:
                    {{
                        bool got_label = false;
                        if (feat->IsSetQual()) {
                            ITERATE(CSeq_feat::TQual, it, feat->GetQual()) {
                                if (NStr::EqualNocase((*it)->GetQual(),"Name")  ||
                                    NStr::EqualNocase((*it)->GetQual(),"ID")) {
                                    *label += (*it)->GetVal();
                                    got_label = true;
                                    break;
                                }
                            }
                        }
                        if ( !got_label ) {
                            feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_clone:
                    {{
                        const CSeqFeatData::TClone& clone = feat->GetData().GetClone();
                        if (clone.IsSetName()) {
                            *label += clone.GetName();
                        } else {
                            feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_seq:
                    {{
                        if (feat->CanGetTitle()) {
                            *label += feat->GetTitle();
                        } else {
                            s_GetSeq_TotalRangeLabel(feat->GetData().GetSeq(), label, scope, type);
                        }
                    }}
                    break;

                default:
                    feature::GetLabel(*feat, label, feature::fFGL_Content, scope);
                    break;
                }
            }
            break;

        case CLabel::eDescriptionBrief:
        case CLabel::eDescription:
            /// the brief description contains:
            /// - the label containing both type and content
            /// - a line showing feature lengths
            switch (feat->GetData().GetSubtype()) {
            case CSeqFeatData::eSubtype_variation_ref:
                {{
                    const CVariation_ref& var = feat->GetData().GetVariation();
                    string validation = "Not Tested";
                    if (var.IsSetValidated()) {
                        if (var.GetValidated()) {
                            validation = "Yes";
                        } else {
                            validation = "Fail";
                        }
                    } else if (var.IsSetVariant_prop()  &&  var.GetVariant_prop().IsSetOther_validation()) {
                        if (var.GetVariant_prop().GetOther_validation()) {
                            validation = "Yes";
                        } else {
                            validation = "Fail";
                        }
                    }

                    // phenotype and clinical-significance
                    string phenotype = "";
                    string clinical_sig = "";
                    if (var.CanGetPhenotype()) {
                        ITERATE (CVariation_ref::TPhenotype, pnt_iter, var.GetPhenotype()) {
                            if (clinical_sig.empty()  &&  (*pnt_iter)->CanGetClinical_significance()) {
                                clinical_sig = NSnp::ClinSigAsString((*pnt_iter)->GetClinical_significance());
                            }

                            if (phenotype.empty()  &&  (*pnt_iter)->CanGetTerm()) {
                                phenotype = (*pnt_iter)->GetTerm();
                            }
                        }
                    }

                    string sample_sex = "";
                    if (var.CanGetSomatic_origin()) {
                        ITERATE (CVariation_ref::TSomatic_origin, so_iter, var.GetSomatic_origin()) {
                            if ((*so_iter)->CanGetSource()  &&
                                (*so_iter)->GetSource().GetSubtype() == CSubSource::eSubtype_sex) {
                                sample_sex = (*so_iter)->GetSource().GetName();
                                break;
                            }
                        }
                    }

                    bool is_sv = (var.IsSetId()  &&  var.GetId().GetTag().IsStr()  &&
                        var.GetId().GetTag().GetStr().find("sv") == 1);

                    // allele type
                    string variant_type;
                    if (s_IsLOH(var)) {
                        variant_type = "LOH";
                    } else if (var.IsComplex()) {
                        variant_type = "Complex";
                    } else if (var.IsUniparentalDisomy()) {
                        variant_type = "UPD";
                    } else if (var.IsInsertion()) {
                        variant_type = "Insertion";
                    } else if (var.IsGain()) {
                        if (var.CanGetMethod()  &&  s_IsCopyNumberMethod(var.GetMethod())) {
                            variant_type = "Copy number gain";
                        } else {
                            variant_type = "Duplication";
                        }
                    } else if (var.IsLoss()) {
                        if (var.CanGetMethod()  &&  s_IsCopyNumberMethod(var.GetMethod())) {
                            variant_type = "Copy number loss";
                        } else {
                            variant_type = "Deletion";
                        }
                    } else if (var.IsDeletion()) {
                        if (var.CanGetMethod()  &&  s_IsCopyNumberMethod(var.GetMethod())) {
                            variant_type = "Copy number loss";
                        } else {
                            variant_type = "Deletion";
                        }
                    } else if (var.IsInversion()) {
                        variant_type = "Inversion";
                    } else if (var.IsEversion()) {
                        variant_type = "Tandem duplication";
                    } else if (var.IsTranslocation()) {
                        variant_type = "Translocation";
                    } else if (var.IsDeletionInsertion()) {
                        variant_type = "Indel";
                    } else if (var.IsUnknown()) {
                        variant_type = "Unknown";
                    } else if (var.IsCNV()) {
                        variant_type = "Copy number variation";
                    } else {
                        variant_type = "Other";
                    }

                    if (is_sv) {
                        int child_num = s_GetChildNum(*feat);
                        if (child_num == 0) {
                            // sv, but treated as ssv
                            is_sv = false;
                        } else {
                            if (child_num > 0) {
                                *label += "Number of Supporting Variant Calls: ";
                                *label += NStr::IntToString(child_num);
                                *label += "\n";
                            }
                            *label += "Variant Region Type: ";
                            *label += variant_type;
                            if ( !phenotype.empty() ) {
                                *label += "\nPhenotype: " + phenotype;
                            }
                            if ( !clinical_sig.empty() ) {
                                *label += "\nClinical Interpretation: ";
                                *label += clinical_sig;
                            }
                            if ( !sample_sex.empty() ) {
                                *label += "\nGender: ";
                                *label += sample_sex;
                            }
                            *label += "\nValidation Status: " + validation;
                            *label += "\nTotal Length: ";
                            *label += NStr::IntToString(
                                feat->GetLocation().GetTotalRange().GetLength(),
                                NStr::fWithCommas);
                        }
                    }

                    if ( !is_sv ) {
                        if (var.CanGetParent_id()) {
                            *label += "Parent Variant Region: ";
                            if (var.GetParent_id().GetTag().IsId()) {
                                *label += NStr::IntToString(var.GetParent_id().GetTag().GetId());
                            } else {
                                *label += var.GetParent_id().GetTag().GetStr();
                            }
                            *label += "\n";
                        }


                        *label += "Variant Call Type: ";
                        *label += variant_type;

                        if ( !phenotype.empty() ) {
                            *label += "\nPhenotype: " + phenotype;
                        }
                        if ( !clinical_sig.empty() ) {
                            *label += "\nClinical Interpretation: ";
                            *label += clinical_sig;
                        }
                        if ( !sample_sex.empty() ) {
                            *label += "\nGender: ";
                            *label += sample_sex;
                        }

                        // inheritance
                        CVariation_ref::TAllele_origin org = -1;
                        if (var.IsSetAllele_origin()) {
                            org = var.GetAllele_origin();
                        } else if (var.IsSetVariant_prop()  &&  var.GetVariant_prop().IsSetAllele_origin()) {
                            org = var.GetVariant_prop().GetAllele_origin();
                        }

                        if (org > 0) {
                            string inh;
                            if (org & CVariation_ref::eAllele_origin_somatic) {
                                inh += "somatic ";
                            }
                            if (org & CVariation_ref::eAllele_origin_inherited) {
                                inh += "inherited ";
                            }
                            if (org & CVariation_ref::eAllele_origin_paternal) {
                                inh += "paternal ";
                            }
                            if (org & CVariation_ref::eAllele_origin_maternal) {
                                inh += "maternal ";
                            }
                            if (org & CVariation_ref::eAllele_origin_de_novo) {
                                inh += "de_novo ";
                            }
                            if (org & CVariation_ref::eAllele_origin_biparental) {
                                inh += "biparental ";
                            }
                            if (org & CVariation_ref::eAllele_origin_uniparental) {
                                inh += "uniparental ";
                            }
                            if (org & CVariation_ref::eAllele_origin_not_tested) {
                                inh += "not-tested ";
                            }
                            if (org & CVariation_ref::eAllele_origin_tested_inconclusive) {
                                inh += "tested-inconclusive ";
                            }
                            if (org & CVariation_ref::eAllele_origin_other) {
                                inh += "other";
                            }

                            if (org == CVariation_ref::eAllele_origin_unknown) {
                                inh = "unknown";
                            }

                            *label += "\nInheritance: " + inh;
                        }

                        // validation status
                        *label += "\nValidation Status: " + validation;

                        if (feat->CanGetComment()) {
                            *label += "\n" + feat->GetComment();
                        }
                    }

                    int allele_len = -1;
                    if (var.GetData().IsInstance()  &&  var.GetData().GetInstance().IsSetDelta()) {
                        const CVariation_inst::TDelta& delta = var.GetData().GetInstance().GetDelta();
                        ITERATE (CVariation_inst::TDelta, iter, delta) {
                            if ((*iter)->IsSetSeq()  &&  (*iter)->GetSeq().IsLiteral()) {
                                allele_len = (*iter)->GetSeq().GetLiteral().GetLength();
                                break;
                            }
                        }
                    }
                    if (allele_len > -1) {
                        *label += "\nAllele length: " + NStr::NumericToString(allele_len, NStr::fWithCommas);
                    }

                }}
                break;

            case CSeqFeatData::eSubtype_clone:
                {{
                    const CSeqFeatData::TClone& clone = feat->GetData().GetClone();
                    *label += "Length: ";
                    *label += NStr::IntToString(feat->GetLocation().GetTotalRange().GetLength(),
                        NStr::fWithCommas);
                    *label += "\nStart: ";
                    *label += NStr::IntToString(feat->GetLocation().GetStart(
                        eExtreme_Positional) + 1, NStr::fWithCommas);
                    *label += ", Stop: ";
                    *label += NStr::IntToString(feat->GetLocation().GetStop(
                        eExtreme_Positional) + 1, NStr::fWithCommas);
                    if (clone.IsSetPlacement_method()) {
                        const CEnumeratedTypeValues* type_val =
                            CClone_ref::GetTypeInfo_enum_EPlacement_method();
                        if (type_val) {
                            string method = type_val->FindName(clone.GetPlacement_method(), true);
                            if ( !method.empty() ) {
                                *label += "\nPlacement method: " + method;
                            }
                        }
                    }
                    if (clone.IsSetConcordant()) {
                        *label += "\nClone placement: ";
                        *label += clone.GetConcordant() ? "concordant" : "discordant";
                    }
                    if (clone.IsSetUnique()) {
                        *label += "\nClone placement: ";
                        *label += clone.GetUnique() ? "unique" : "multiple";
                    }
                    if (clone.IsSetClone_seq()) {
                        typedef map<int, vector<string> > TEndMap;
                        TEndMap end_map;
                        const CClone_ref::TClone_seq::Tdata& ends = clone.GetClone_seq().Get();
                        ITERATE (CClone_ref::TClone_seq::Tdata, e_iter, ends) {
                            const CClone_seq& seq = **e_iter;
                            if (seq.CanGetSeq()  &&  seq.CanGetSupport()) {
                                const CSeq_id* id = seq.GetSeq().GetId();
                                if( id ){
									CSeq_id_Handle idh;
                                    string id_str;
									if( scope ){
										idh = sequence::GetId( *id, *scope );
										if( idh ){
											CSeq_id_Handle idhb = sequence::GetId(idh, *scope, sequence::eGetId_Best);
											if( idhb ){
												idh = idhb;
											}
										}
									}
                                    if( idh ){
                                        CConstRef<CSeq_id> seq_id_ref = idh.GetSeqId();
                                        seq_id_ref->GetLabel( &id_str, CSeq_id::eContent );
                                    } else {
                                        id->GetLabel(&id_str);
                                    }
                                    end_map[seq.GetSupport()].push_back(id_str);
                                }
                            }
                        }

                        if (end_map.count(CClone_seq::eSupport_prototype) > 0) {
                            *label += "\nPrototype: " + CStringUtil::ConcatenateStrings(end_map[CClone_seq::eSupport_prototype]);
                        }

                        if (end_map.count(CClone_seq::eSupport_supporting) > 0) {
                            *label += "\nSupporting (" +
                                NStr::NumericToString(end_map[CClone_seq::eSupport_supporting].size()) +
                                "): ";
                            *label += CStringUtil::FormatStringsOccurrences(end_map[CClone_seq::eSupport_supporting]);
                        }

                        if (end_map.count(CClone_seq::eSupport_non_supporting) > 0) {
                            *label += "\nNon-supporting (" +
                                NStr::NumericToString(end_map[CClone_seq::eSupport_non_supporting].size()) +
                                "): ";
                            *label += CStringUtil::FormatStringsOccurrences(end_map[CClone_seq::eSupport_non_supporting]);
                        }

                        if (end_map.count(CClone_seq::eSupport_supports_other) > 0) {
                            *label += "\nSupports other (" +
                                NStr::NumericToString(end_map[CClone_seq::eSupport_supports_other].size()) +
                                "): ";
                            *label += CStringUtil::FormatStringsOccurrences(end_map[CClone_seq::eSupport_supports_other]);
                        }
                    }
                }}
                break;

            case CSeqFeatData::eSubtype_variation:
                GetLabel(obj, label, CLabel::eUserTypeAndContent, scope);
                break;

            case CSeqFeatData::eSubtype_imp:
                feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                *label += " ";
                if (feat->GetData().GetImp().IsSetDescr()) {
                    *label += feat->GetData().GetImp().GetDescr();
                }
                break;

            case CSeqFeatData::eSubtype_exon:
                GetLabel(obj, label, CLabel::eContent, scope);
                break;

            case CSeqFeatData::eSubtype_region:
                {{
                    feature::GetLabel(*feat, label, feature::fFGL_Type, scope);
                    *label += " ";
                    *label += CGUIUserType::sm_Tp_Feature + " : ";
                    GetLabel(obj, label, CLabel::eUserTypeAndContent, scope);
                    if (feat->IsSetQual()) {
                        ITERATE(CSeq_feat::TQual, it, feat->GetQual()) {
                            *label += "\n" + (*it)->GetQual() + ": " + (*it)->GetVal();
                        }
                    }
                }}
                break;

            case CSeqFeatData::eSubtype_seq:
                {{
                    *label += "Refers to: ";
                    if (feat->CanGetTitle()) {
                        *label += feat->GetTitle();
                    } else {
                        s_GetSeq_TotalRangeLabel(feat->GetData().GetSeq(), label, scope, type);
                    }
                    if (feat->IsSetQual()) {
                        ITERATE(CSeq_feat::TQual, it, feat->GetQual()) {
                            *label += "\n" + (*it)->GetQual() + ": " + (*it)->GetVal();
                        }
                    }
                }}
                break;

            case CSeqFeatData::eSubtype_site:
                if (feat->GetData().GetSite() == CSeqFeatData::eSite_other  &&
                    feat->IsSetComment()) {
                    *label += "[Site] ";
                    *label += feat->GetComment();
                } else {
                    feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                }
                break;

            default:
                feature::GetLabel(*feat, label, feature::fFGL_Type, scope);
                *label += " ";
                *label += CGUIUserType::sm_Tp_Feature + " : ";
                GetLabel(obj, label, CLabel::eUserTypeAndContent, scope);
                break;
            }
            break;

        case CLabel::eUserTypeAndContent:
        default:
            if (x_UseCustomLabel(*feat)) {
                x_GetCustomLabel(*feat, *label);
            }
            /// for both, we use a modified construct of the base text
            else if (feat->IsSetProduct()) {
                try {
                    string s;
                    const CSeq_id& id = sequence::GetId(feat->GetProduct(), scope);
                    s_GetSeqLabel(id, &s, scope, CLabel::eContent);
                    if (!feat->GetData().IsRna()) {
                        string product_str;
                        string type_str;
                        feature::GetLabel(*feat, &product_str, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                        feature::GetLabel(*feat, &type_str, feature::fFGL_Type, scope);
                        // Do not append the product if it contains type information only
                        if (product_str != type_str) {
                            s += ": ";
                            s += product_str;
                        }

                    }
                    *label += s;
                }
                catch (CException&) {
                    feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                }
            } else {
                switch (feat->GetData().GetSubtype()) {
                case CSeqFeatData::eSubtype_gene:
                    feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                    break;

                case CSeqFeatData::eSubtype_imp:
                    feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                    *label += " ";
                    if (feat->GetData().GetImp().IsSetDescr()) {
                        *label += feat->GetData().GetImp().GetDescr();
                    }
                    break;

                case CSeqFeatData::eSubtype_exon:
                    GetLabel(obj, label, CLabel::eContent, scope);
                    break;

                case CSeqFeatData::eSubtype_region:
                    feature::GetLabel(*feat, label,
                                      feature::fFGL_Type |
                                      feature::fFGL_Content |
                                      feature::fFGL_NoQualifiers, scope);
                    if (feat->IsSetQual()) {
                        ITERATE(CSeq_feat::TQual, it, feat->GetQual()) {
                            *label += "\n" + (*it)->GetQual() + ": " + (*it)->GetVal();
                        }
                    }
                    break;

                case CSeqFeatData::eSubtype_site:
                    if (feat->GetData().GetSite() == CSeqFeatData::eSite_other  &&
                        feat->IsSetComment()) {
                        *label += "[Site] ";
                        *label += feat->GetComment();
                    } else {
                        feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                    }

                    break;

                case CSeqFeatData::eSubtype_variation:
                    {{
                        CSnpBitfield bf_interp(*feat);
                        if (bf_interp.isGood()) {
                            string tag(bf_interp.GetVariationClassString());
                            *label += tag;
                            *label += " rs";
                            *label += NStr::NumericToString(NSnp::GetRsid(*feat));
                        } else {
                            feature::GetLabel(*feat, label,
                                              feature::fFGL_Type |
                                              feature::fFGL_Content |
                                              feature::fFGL_NoQualifiers, scope);
                        }

                        if (feat->IsSetQual()) {
                            ///
                            /// alleles come next
                            NSnp::TAlleles Alleles;
                            NSnp::GetAlleles(*feat, Alleles);
                            if(!Alleles.empty()) {
                                if ( !label->empty() ) {
                                    *label += "; ";
                                }
                                *label += "Alleles: ";
                                *label += NStr::Join(Alleles, "/");
                                /// Add length of allele for insertion/deletion types
                                ///
                                if (bf_interp.isGood() && (bf_interp.IsTrue(CSnpBitfield::eDips) || bf_interp.IsTrue(CSnpBitfield::eDeletion))) {
                                    int length = NSnp::GetLength(*feat);
                                    *label += "; deletion length=";
                                    *label += NStr::IntToString(length);

                                }
                            }
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_variation_ref:
                    {{
                        const CVariation_ref& var = feat->GetData().GetVariation();
                        if (var.CanGetId()) {
                            *label += "[" + var.GetId().GetDb() + "] ";
                            if (var.GetId().GetTag().IsId()) {
                                *label += NStr::IntToString(var.GetId().GetTag().GetId());
                            } else {
                                *label += var.GetId().GetTag().GetStr();
                            }
                        }
                        ITERATE (CSeq_feat::TQual, iter, feat->GetQual()) {
                            const CGb_qual& qual = **iter;
                            *label += " " + qual.GetQual();
                            if ( !qual.GetVal().empty() ) {
                                *label += "=" + qual.GetVal();
                            }
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_clone:
                    {{
                        const CSeqFeatData::TClone& clone = feat->GetData().GetClone();
                        if (clone.IsSetName()) {
                            *label += clone.GetName();
                        } else {
                            feature::GetLabel(*feat, label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                        }
                    }}
                    break;

                case CSeqFeatData::eSubtype_seq:
                    {{
                        if (feat->CanGetTitle()) {
                            *label += feat->GetTitle();
                        } else {
                            s_GetSeq_TotalRangeLabel(feat->GetData().GetSeq(), label, scope, type);
                        }
                    }}
                    break;

                default:
                    feature::GetLabel(*feat, label,
                                      feature::fFGL_Type |
                                      feature::fFGL_Content |
                                      feature::fFGL_NoQualifiers, scope);
                    break;
                }
            }
            break;
        }
    }
protected:
    bool x_UseCustomLabel(const objects::CSeq_feat &feat) const;
    void x_GetCustomLabel(const objects::CSeq_feat &feat, string& label) const;
    void x_LabelFromQualifiers(const objects::CSeq_feat &feat, string& label) const;
    bool x_IncludeFeatureTypeInLabel(const objects::CSeq_feat &feat) const;
private:
    CUserFeatHandler m_UserHandler;
};


bool CSeq_featHandler::x_UseCustomLabel(const CSeq_feat &feat) const
{
    static set<CSeqFeatData::ESubtype> subtypes{ CSeqFeatData::eSubtype_misc_feature, CSeqFeatData::eSubtype_misc_recomb, CSeqFeatData::eSubtype_misc_structure, CSeqFeatData::eSubtype_mobile_element,
        CSeqFeatData::eSubtype_oriT, CSeqFeatData::eSubtype_protein_bind, CSeqFeatData::eSubtype_region, CSeqFeatData::eSubtype_regulatory, CSeqFeatData::eSubtype_rep_origin,
        CSeqFeatData::eSubtype_repeat_region, CSeqFeatData::eSubtype_stem_loop };

    return subtypes.end() != subtypes.find(feat.GetData().GetSubtype());
}

void CSeq_featHandler::x_GetCustomLabel(const CSeq_feat &feat, string& label) const
{
    CSeqFeatData::ESubtype subtype = feat.GetData().GetSubtype();
    if (x_IncludeFeatureTypeInLabel(feat) && ((subtype != CSeqFeatData::eSubtype_misc_feature) && (subtype != CSeqFeatData::eSubtype_region)))  // Add the subtype only for features that do not have specific qualifiers
        label = CSeqFeatData::SubtypeValueToName(subtype);
    string second_part;
    if (feat.GetData().IsRegion()) {
        second_part = feat.GetData().GetRegion();
    }
    else {
        x_LabelFromQualifiers(feat, second_part);
    }
    if (second_part.empty() && feat.IsSetComment()) {
        second_part = feat.GetComment();
        size_t pos = second_part.find(';');
        if (pos != string::npos)
            second_part = second_part.substr(0, pos);
    }
    if (!second_part.empty()) {
        if (!label.empty())
            label += ": ";
        label += second_part;
    }
    if (label.empty())
        label = CSeqFeatData::SubtypeValueToName(subtype);
}

void CSeq_featHandler::x_LabelFromQualifiers(const CSeq_feat &feat, string& label) const
{
    static vector<string> quals{ "regulatory_class", "recombination_class", "feat_class", "bound_moiety", "mobile_element_type", "mobile_element", "rpt_type",
        "satellite", "rpt_family", "mod_base", "operon", "standard_name", "allele" };

    for (const auto &qual_name : quals) {
        const string &value = feat.GetNamedQual(qual_name);
        if (value.empty())
            continue;

        label = value;
        break;
    }
}

bool CSeq_featHandler::x_IncludeFeatureTypeInLabel(const objects::CSeq_feat &feat) const
{
    static vector<string> class_quals{ "feat_class", "regulatory_class", "recombination_class", "mobile_element_type", "mobile_element", "rpt_type", "satellite", "rpt_family" };
    for (const auto &qual_name : class_quals) {
        const string &value = feat.GetNamedQual(qual_name);
        if (!value.empty())
            return false;
    }
    return true;
}

//
//
// seq-entry handler
//
//
class CSeq_entryHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
        const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(&obj);
        if (entry) {
            if (entry->IsSet()) {
                CLabel::GetLabel(entry->GetSet(), label, type, scope);
            } else {
                CLabel::GetLabel(entry->GetSeq(), label, type, scope);
            }
        }
    }
};


static void s_GetAlignmentType(const CSeq_align& align, string* label)
{
    switch (align.GetSegs().Which()) {
    default:
    case CSeq_align::TSegs::e_not_set:
        *label += "[Unknown]";
        break;
    case CSeq_align::TSegs::e_Denseg:
        *label += "[Dense-seg]";
        break;
    case CSeq_align::TSegs::e_Dendiag:
        *label += "[Dense-diag]";
        break;
    case CSeq_align::TSegs::e_Std:
        *label += "[Standard-seg]";
        break;
    case CSeq_align::TSegs::e_Packed:
        *label += "[Packed-seg]";
        break;
    case CSeq_align::TSegs::e_Disc:
        *label += "[Discontinuous]";
        break;
    }
}


static void s_GetAlignmentContent(const CSeq_align& align, string* label,
                                  CScope* scope)
{
    const CSeq_align::TSegs& asegs = align.GetSegs();
    CSeq_align::TSegs::E_Choice asegs_type = asegs.Which();
    switch (asegs_type) {
    default:
    case CSeq_align::TSegs::e_not_set:
        *label += "unknown contents";
        break;
    case CSeq_align::TSegs::e_Spliced:
        {{
        const CSeq_align::TSegs::TSpliced& spliced = asegs.GetSpliced();
        if (spliced.CanGetProduct_id()) {
            const CSeq_id& product_id = spliced.GetProduct_id();
            string str;
            s_GetSeqLabel(product_id, &str, scope, CLabel::eContent);
            *label += str;
            *label += " x ";
        }
        if (spliced.CanGetGenomic_id()) {
            const CSeq_id& genomic_id = spliced.GetGenomic_id();
            string str;
            s_GetSeqLabel(genomic_id, &str, scope, CLabel::eContent);
            *label += str;
        }
        }}
        break;
    case CSeq_align::TSegs::e_Denseg:
        {{
            string str;
            size_t seqs = align.GetSegs().GetDenseg().GetIds().size();
            if (seqs < 4) {
                set<CSeq_id_Handle> handle_set;
                ITERATE (CDense_seg::TIds, id_iter, align.GetSegs().GetDenseg().GetIds()) {
                    CSeq_id_Handle sih = CSeq_id_Handle::GetHandle(**id_iter);
                    if ( !handle_set.insert(sih).second ) {
                        continue;
                    }

                    if ( !str.empty() ) {
                        str += " x ";
                    }
                    s_GetSeqLabel(*sih.GetSeqId(), &str, scope, CLabel::eContent);
                }

                if( handle_set.size() == 1 ){
                    str += " x " + str;
                }

            } else {
                str = NStr::SizetToString(seqs, NStr::fWithCommas) + " sequences.";
            }

            *label += str;
        }}
        break;
    case CSeq_align::TSegs::e_Dendiag:
        *label += "[Dense-diag]";
        break;
    case CSeq_align::TSegs::e_Std:
        *label += "[Standard-seg]";
        break;
    case CSeq_align::TSegs::e_Packed:
        *label += "[Packed-seg]";
        break;
    case CSeq_align::TSegs::e_Sparse:
        *label += "[Sparse-seg]";
        break;
    case CSeq_align::TSegs::e_Disc:
        {{
            string str;
            set<CSeq_id_Handle> handle_set;
            CTypeConstIterator<CSeq_id> id_iter(align.GetSegs());
            for ( ;  id_iter;  ++id_iter) {
                CSeq_id_Handle sih = CSeq_id_Handle::GetHandle(*id_iter);
                if ( !handle_set.insert(sih).second ) {
                    continue;
                }

                if ( !str.empty() ) {
                    str += " x ";
                }
                s_GetSeqLabel(*sih.GetSeqId(), &str, scope, CLabel::eContent);
            }

            *label += str;
            if (handle_set.size() == 1) { // paralogous alignment
                *label += " x ";
                *label += str;
            }
        }}
        break;
    }
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



static void s_GetAlignmentTooltip(const CSeq_align& align, string* label,
                                  CScope* scope)
{
    const CSeq_align::TDim num_rows_limit = 10;

    ///
    /// first, format our seq-ids
    ///
    vector<bool> strands;
    CSeq_align::TDim num_rows = align.CheckNumRows();
    {{
         string ids;
         CSeq_align::TDim i = 0;
         for(;  i < num_rows;  ++i ){
             CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( align.GetSeq_id(i) );
             if( scope ){
                CSeq_id_Handle idh_best = sequence::GetId( idh, *scope, sequence::eGetId_Best);
                if( idh_best ){
                    idh = idh_best;
                }
             }

             if ( !ids.empty() ) {
                 ids += " x ";
             }
             CConstRef<CSeq_id> seq_id(idh.GetSeqIdOrNull());
             if (!seq_id) {
                ids = "UNK";
             }
             else {
                idh.GetSeqId()->GetLabel(&ids, CSeq_id::eContent);
             }

             TSeqRange range = align.GetSeqRange(i);
             ids += " (";
             ids += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
             ids += "..";
             ids += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);
             ids += ")";

             try {
                strands.push_back(align.GetSeqStrand(i) == eNa_strand_plus);
             } catch (CException&) {
                 // ignore the exception
             }
             // break out of the loop to avoid long (slow) tooltips
             if (i > num_rows_limit)
                 break;
         }

         *label += ids;

         if (i < num_rows) { // early escape?
            *label += " x ... [ total ";
            *label += NStr::IntToString(num_rows, NStr::fWithCommas);
            *label += "]";
            return;
         }
     }}

    /// only report strand for pair-wise alignments
    if (num_rows == 2  &&  strands.size() == 2) {
        *label += "\nStrand: ";
        *label += strands[0] == strands[1] ? "forward" : "reverse";
    }

    ///
    /// next, add a remark about the total aligned range
    ///
    CScoreBuilder builder;
    // new method
    TSeqPos align_length = builder.GetAlignLength(align);
    *label += "\nAligned bases: ";
    *label += NStr::IntToString(align_length, NStr::fWithCommas);

    size_t segs = s_CountSegments(align);
    *label += "\nAligned segments: ";
    *label += NStr::SizetToString(segs, NStr::fWithCommas);

    char buf[255];
    double coverage = 0.0;
    *label += "\nCoverage: ";
    if ( !align.GetNamedScore(CSeq_align::eScore_PercentCoverage, coverage) && scope ){
        coverage = builder.GetPercentCoverage( *scope, align );
    }
    if (coverage < 100.0  &&  coverage > 99.9) {
        *label += "99.9+";
    } else {
        sprintf(buf, "%2.1f", coverage);
        *label += buf;
    }
    *label += "%";

    if (align_length < 1000000) {
        //int identities = builder.GetIdentityCount(*scope, align);
        //int mismatches = builder.GetMismatchCount(*scope, align);
        int identities = 0;
        int mismatches = 0;
        if (
			!( align.GetNamedScore(CSeq_align::eScore_IdentityCount, identities)
				&& align.GetNamedScore(CSeq_align::eScore_MismatchCount, mismatches)
			)
			&& scope
		){
           builder.GetMismatchCount(*scope, align, identities, mismatches);
        }

        double identity = 0.0;
        *label += "\nIdentity: ";
        if ( !align.GetNamedScore(CSeq_align::eScore_PercentIdentity, identity) ) {
            identity = identities * 100.0 / align_length;
        }
        if (identity < 100.0  &&  identity > 99.9) {
            *label += "99.9+";
        } else {
            sprintf(buf, "%2.1f", identity);
            *label += buf;
        }
        *label += "%";


        *label += "\nMismatches: ";
        *label += NStr::IntToString(mismatches, NStr::fWithCommas);
    }
    int gap_count  = builder.GetGapCount(align);
    *label += "\nGaps: ";
    *label += NStr::IntToString(gap_count, NStr::fWithCommas);

#if 0
    switch (align.GetSegs().Which()) {
    default:
    case CSeq_align::TSegs::e_not_set:
    case CSeq_align::TSegs::e_Dendiag:
    case CSeq_align::TSegs::e_Std:
    case CSeq_align::TSegs::e_Packed:
        s_GetAlignmentContent(align, label, scope);
        break;

    case CSeq_align::TSegs::e_Disc:
        {{
            string s;
            size_t count = 0;
            ITERATE (CSeq_align::TSegs::TDisc::Tdata, iter, align.GetSegs().GetDisc().Get()) {
                if ( !s.empty() ) {
                    s += '\n';
                }
                s += NStr::IntToString(++count) + ") ";
                s_GetAlignmentTooltip(**iter, &s, scope);
            }
            *label += s;
        }}
        break;

    case CSeq_align::TSegs::e_Denseg:
        {{
            size_t seqs = align.GetSegs().GetDenseg().GetIds().size();
            if (seqs != 2) {
                s_GetAlignmentContent(align, label, scope);
                break;
            }

            //
            // pairwise alignment
            //
            const CSeq_id& id1 = *align.GetSegs().GetDenseg().GetIds().front();
            const CSeq_id& id2 = *align.GetSegs().GetDenseg().GetIds().back();

            string str;
            int mate_ti = 0;
            if (align.GetNamedScore("matepair ti", mate_ti)  ||
                align.GetNamedScore("bad matepair ti", mate_ti)) {
                //
                // "good" mate pair
                //
                const CSeq_id* id_trace = &id1;
                if (id2.IsGeneral()  &&
                    (id2.GetGeneral().GetDb() == "ti"  ||
                     id2.GetGeneral().GetDb() == "TRACE")) {
                    id_trace = &id2;
                }

                str = "Trace: ";
                s_GetSeqLabel(*id_trace, &str, scope, CLabel::eContent);
                str += "\nMate:  ti|";
                str += NStr::SizetToString((unsigned int)mate_ti);

                int code = 0;
                if (align.GetNamedScore("bad matepair code", code)) {
                    switch (code) {
                    case 1:
                        str += "\nError: Mate has wrong orientation";
                        break;
                    case 2:
                        str += "\nError: Trace has non-unique placement";
                        break;
                    case 3:
                        str += "\nError: Mate lies outside expected distance from trace";
                        break;
                    default:
                        str += "\nError Code: ";
                        str += NStr::IntToString(code);
                        break;
                    }
                }
            } else {
                size_t count = 0;
                ITERATE (CDense_seg::TIds, iter, align.GetSegs().GetDenseg().GetIds()) {
                    if ( !str.empty() ) {
                        str += " x ";
                    }
                    s_GetSeqLabel(**iter, &str, scope, CLabel::eContent);

                    TSeqRange range = align.GetSegs().GetDenseg().GetSeqRange(count++);
                    str += " [";
                    str += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
                    str += "-";
                    str += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);
                    str += "]";
                }

                /// compose scores if they exist
                string score_str;
                if (align.IsSetScore()) {
                    ITERATE (CSeq_align::TScore, iter, align.GetScore()) {
                        score_str += ' ';
                        const CScore& score = **iter;
                        if (score.IsSetId()) {
                            if (score.GetId().IsId()) {
                                score_str += NStr::IntToString(score.GetId().GetId());
                            } else {
                                score_str += score.GetId().GetStr();
                            }
                            score_str += '=';
                        }
                        if (score.GetValue().IsReal()) {
                            score_str += NStr::DoubleToString(score.GetValue().GetReal());
                        } else {
                            score_str += NStr::DoubleToString(score.GetValue().GetInt());
                        }
                    }
                } else {
                    score_str = " (no scores)";
                }
                str += score_str;
            }

            *label += str;
        }}
        break;
    }
#endif
}


class CSeq_alignHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
		// check: scope is NEEDED [indirectly protected]

		const CSeq_align* align  = dynamic_cast<const CSeq_align*> (&obj);
        if( !align ) return;

        switch (type) {
        case CLabel::eUserType:
            *label += CGUIUserType::sm_Tp_Alignment;
            break;

        case CLabel::eUserSubtype:
            *label += "";
            break;

        case CLabel::eType:
            s_GetAlignmentType(*align, label);
            return;

        default:
        case CLabel::eContent:
        case CLabel::eDescriptionBrief:
            s_GetAlignmentContent(*align, label, scope);
            return;

        case CLabel::eUserTypeAndContent:
            s_GetAlignmentType(*align, label);
            *label += ' ';
            s_GetAlignmentContent(*align, label, scope);
            break;

        case CLabel::eDescription:
            s_GetAlignmentTooltip(*align, label, scope);
            break;
        }
    }
};


class CSeq_align_setHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* /*scope*/) const
    {
        const CSeq_align_set* align  = dynamic_cast<const CSeq_align_set*> (&obj);
        if ( !align ) {
            return;
        }

        switch (type) {
        case CLabel::eUserType:
            *label += CGUIUserType::sm_Tp_Alignment_Set;
            break;

        case CLabel::eUserSubtype:
            *label += "";
            break;

        case CLabel::eType:
            *label += "Seq-align-set";
            return;

        default:
        case CLabel::eContent:
        case CLabel::eDescriptionBrief:
            *label += "Seq-align-set"; //TODO
            return;

        case CLabel::eUserTypeAndContent:
            *label += "Seq-align-set"; //TODO
            break;

        case CLabel::eDescription:
            *label += "Seq-align-set"; //TODO
            break;
        }
    }
};

//
// seq-annots are complicated
//

static void s_GetAnnotType(const CSeq_annot& annot, string* label)
{
    const CSeq_annot::TData& annot_data = annot.GetData();
    switch (annot_data.Which()) {
    case CSeq_annot::TData::e_Ftable:
        *label += "Feature Table";
        break;

    case CSeq_annot::TData::e_Align:
        *label += "Alignment";
        break;

    case CSeq_annot::TData::e_Graph:
        *label += "Graph";
        break;

    case CSeq_annot::TData::e_Ids:
        *label += "IDs";
        break;
    case CSeq_annot::TData::e_Locs:
        *label += "Locations";
        break;

    default:
        *label = "Annotation";
        return;
    }
}


static void s_GetAnnotContent(const CSeq_annot& annot, string* label, CScope* scope)
{
	// check: scope is NEEDED [protected]

	//
    // check for formatted comments in descriptors first
    //
    if (annot.IsSetDesc()) {
        const string* title = NULL;
        const string* name = NULL;
        ITERATE (CSeq_annot::TDesc::Tdata, iter, annot.GetDesc().Get()) {
            switch ((*iter)->Which()) {
            case CAnnotdesc::e_Title:
                title = &(*iter)->GetTitle();
                break;

            case CAnnotdesc::e_Name:
                name = &(*iter)->GetName();
                break;

            default:
                break;
            }
        }

        if (title) {
            *label = *title;
            return;
        } else if (name) {
            *label = *name;
            return;
        }
    }

    //
    // default handling
    //
    string str, extra;
    size_t items = 0;
    const CSeq_annot::TData& annot_data = annot.GetData();

    switch (annot_data.Which()) {
    case CSeq_annot::TData::e_Ftable:
        str = "Feature Table, ";
        items = annot_data.GetFtable().size();
        if( items > 0 ){
            const CSeq_id* sid = annot_data.GetFtable().front()->GetLocation().GetId();
            if( sid ){
                CSeq_id_Handle idh;
				if( scope ){
					idh = sequence::GetId(*sid, *scope, sequence::eGetId_Canonical);
				}
                if( idh ){
                    CSeq_id_Handle idh_best =
                        sequence::GetId(*sid, *scope, sequence::eGetId_Best)
					;
                    if( idh_best ){
                        idh = idh_best;
                    }
                }

                if( idh ){
                    CConstRef<CSeq_id> seq_id_ref = idh.GetSeqId();
                    extra = "on " + seq_id_ref->GetSeqIdString( true );
                } else {
                    extra = "on " + sid->GetSeqIdString( true );
                }
            }
        }

        break;

    case CSeq_annot::TData::e_Align:
        {{
            ITERATE (CSeq_annot::TData::TAlign, iter, annot_data.GetAlign()) {
                const CSeq_align& align = **iter;
                string s;
                s_GetAlignmentContent(align, &s, scope);
                if (str.empty()) {
                    str = s;
                } else if (str != s) {
                    str.erase();
                    break;
                }
            }
            if (str.empty()) {
                str = "Mixed sequence alignments";
            }
        }}
        if (str.empty()) {
            str = "Alignment, ";
        } else {
            str += ", ";
        }
        items = annot_data.GetAlign().size();
        break;

    case CSeq_annot::TData::e_Graph:
        str = "Graph, ";
        items = annot_data.GetGraph().size();
        break;

    case CSeq_annot::TData::e_Ids:
        str = "IDs, ";
        items = annot_data.GetIds().size();
        break;
    case CSeq_annot::TData::e_Locs:
        str = "Locations, ";
        items = annot_data.GetLocs().size();
        break;

    default:
        *label = "Annotation";
        return;
    }

    str += NStr::SizetToString( items );
    if( items != 1 ){
        str += " entries";
    } else {
        str += " entry";
    }

    if( !extra.empty() ){
        str += " (" + extra + ")";
    }

    *label += str;

}


class CSeq_annotHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* scope) const
    {
		// check: scope is NEEDED [indirectly protected]

		const CSeq_annot* annot  = dynamic_cast<const CSeq_annot*> (&obj);
        if ( !annot ) {
            return;
        }

        switch (type) {
        case CLabel::eUserType:
            *label += CGUIUserType::sm_Tp_Annotation;
            break;

        case CLabel::eUserSubtype:   {
            switch(annot->GetData().Which()) {
            case CSeq_annot::TData::e_Align:
                *label += CGUIUserType::sm_SbTp_Alignments;
                break;

            case CSeq_annot::TData::e_Ftable:
                *label += CGUIUserType::sm_SbTp_Features;
                break;

            case CSeq_annot::TData::e_Graph:
                *label += CGUIUserType::sm_SbTp_Graphs;
                break;

            case CSeq_annot::TData::e_Locs:
                *label += CGUIUserType::sm_SbTp_Locations;
                break;

            case CSeq_annot::TData::e_Ids:
                *label += CGUIUserType::sm_SbTp_Sequence_IDs;
                break;

            case CSeq_annot::TData::e_Seq_table:
                *label += CGUIUserType::sm_SbTp_Table;
                break;

            default:
                _ASSERT(false);
                break;
            }
            return;
        }

        case CLabel::eType:
            s_GetAnnotType(*annot, label);
            return;

        case CLabel::eContent:
        case CLabel::eDescriptionBrief:
            s_GetAnnotContent(*annot, label, scope);
            return;

        case CLabel::eUserTypeAndContent:
        case CLabel::eDescription:
            s_GetAnnotType(*annot, label);
            *label += ": ";
            s_GetAnnotContent(*annot, label, scope);
            break;

        default:
            break;
        }
    }
};

//
//
// ValidError handler
//
//
class CValidErrorHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* /*scope*/) const
    {
		// check: no scope info is actually used

		const CValidError* valerr = dynamic_cast<const CValidError*>(&obj);
        if ( ! valerr) {
            return;
        }

        switch (type) {
        case CLabel::eUserType:
            *label += CGUIUserType::sm_Tp_ValidationResult;
            break;

        case CLabel::eUserSubtype:
            *label += CGUIUserType::sm_SbTp_Undefined;
            break;

        case CLabel::eType:
            *label += valerr->GetThisTypeInfo()->GetName();
            return;

        case CLabel::eContent:
        case CLabel::eUserTypeAndContent:
        case CLabel::eDescriptionBrief:
        {
            CValidError_CI valerr_it(*valerr);
            string acc;
            if (valerr->TotalSize() > 0) {
                if (valerr_it->CanGetAccession()) {
                    acc = valerr_it->GetAccession();
                }
            }
            *label += "Validation results for " + acc
            + ": " + NStr::SizetToString(valerr->TotalSize());
            if (valerr->TotalSize() == 1) {
                *label += " error";
            } else {
                *label += " errors";
            }
            return;
        }

        case CLabel::eDescription:

            *label = "Validation errors total: " + NStr::SizetToString(valerr->TotalSize());
            if (valerr->InfoSize() > 0) {
                *label += " Info: " + NStr::SizetToString(valerr->InfoSize());
            }
            if (valerr->WarningSize() > 0) {
                *label += " Warning: " + NStr::SizetToString(valerr->WarningSize());
            }
            if (valerr->ErrorSize() > 0) {
                *label += " Error: " + NStr::SizetToString(valerr->ErrorSize());
            }
            if (valerr->CriticalSize() > 0) {
                *label += " Reject: " + NStr::SizetToString(valerr->CriticalSize());
            }
            if (valerr->FatalSize() > 0) {
                *label += " Fatal: " + NStr::SizetToString(valerr->FatalSize());
            }
            return;

        default:
            break;
        }
    }
};

//
//
// CTaxIdSelSet handler
//
//
class CTaxIDSetLabelHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, CScope* /*scope*/) const
    {
		// check: no scope info is actually used

		const CTaxIdSelSet* tax_ids = dynamic_cast<const CTaxIdSelSet*>(&obj);
        if ( ! tax_ids) {
            return;
        }

        switch (type) {
        case CLabel::eUserType:
            *label += CGUIUserType::sm_Tp_TaxId_Set;
            break;

        case CLabel::eUserSubtype:
            *label += CGUIUserType::sm_SbTp_Undefined;
            break;

        case CLabel::eType:
            *label += "Set of Tax IDs";
            return;

        case CLabel::eContent:
        case CLabel::eUserTypeAndContent:
        case CLabel::eDescriptionBrief:
        {
            CTaxIdSelSet::TTaxId count = tax_ids->GetSelectedTaxIds().count();

            *label += "Num TaxIDs: " + NStr::UIntToString((unsigned int)count);
            return;
        }

        case CLabel::eDescription:
        {
            *label += "Ids: ";

            int char_count = 0;
            CTaxIdSelSet::TBitVec::enumerator e = tax_ids->GetSelectedTaxIds().first();

            // could be any number of ids but they will be displayed on one line in a list,
            // so pick a max string length which char count can't exceed
            for (; e != tax_ids->GetSelectedTaxIds().end() && char_count < 60; ++e) {
                CTaxIdSelSet::TTaxId id = *e;

                if (char_count > 0)
                    *label += ", ";

                string idstr = NStr::UIntToString((unsigned int)id);
                char_count += idstr.length();

                *label += idstr;
            }

            /// Could not fit all the selected ids on one line:
            if (e != tax_ids->GetSelectedTaxIds().end())
                *label += " ...";

            return;
        }

        default:
            break;
        }
    }
};


//
//
// CGBProject_ver2 handler
// Moved from CGBProject_ver2
//
//
class CGBProject_ver2LabelHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* str,
                  CLabel::ELabelType type,
                  objects::CScope* scope) const
    {
        const CGBProject_ver2* proj = dynamic_cast<const CGBProject_ver2*>(&obj);
        if (proj) {
            switch (type) {
            case CLabel::eUserType:
                *str += CGUIUserType::sm_Tp_Project;
                break;

            case CLabel::eUserSubtype:
                *str += CGUIUserType::sm_SbTp_ProjectVer2;
                break;

            case CLabel::eType:
                *str += "Project (ver.2)";
                break;

            case CLabel::eContent:
                {{
                    string title;
                    if (proj->IsSetDescr()  &&  proj->GetDescr().IsSetTitle()) {
                        title = proj->GetDescr().GetTitle();
                    }

                    if ( !title.empty() ) {
                        *str += title;
                    } else if (proj->IsSetData()) {
                        CTypeConstIterator<CProjectItem> iter(proj->GetData());

                        string s;
                        for ( ;  iter;  ++iter) {
                            const CSerialObject* cso = iter->GetObject();
                            if (cso) {
                                if ( !s.empty() ) {
                                    s += ", ";
                                }
                                CLabel::GetLabel(*cso, &s, type, scope);
                            }
                        }
                        *str += s;
                    }
                }}
                break;

            case CLabel::eUserTypeAndContent:
                {{
                    string type_str;
                    GetLabel(obj, &type_str, CLabel::eType, scope);
                    string content_str;
                    GetLabel(obj, &content_str, CLabel::eType, scope);
                    *str += type_str;
                    if ( !type_str.empty()  &&  !content_str.empty() ) {
                        *str += ": ";
                    }
                    if ( !content_str.empty() ) {
                        *str += content_str;
                    }
                }}
                break;

            default:
                break;
            }
        }
    }
};



////////////////////////////////////////////////////////////
// label handlers for genome workbench project types
// these will be regsitered at application initialization
// Moved from CProjectItem
class CProjectItemLabelHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* str,
                  CLabel::ELabelType type,
                  objects::CScope* scope) const
    {
        if(str == NULL) {
            return;
        }
        const CProjectItem* item = dynamic_cast<const CProjectItem*>(&obj);
        LOG_POST(Error << MSerial_AsnText << *item);
        if (item) {
            switch(type)    {
            case CLabel::eContent:
                *str = item->IsSetLabel() ? item->GetLabel() : "";
                break;
            case CLabel::eUserType:
                *str += CGUIUserType::sm_Tp_ProjectItem;
                break;

            case CLabel::eUserSubtype:
                break;

            case CLabel::eType:
                *str = "Project Item";
                break;

            case CLabel::eUserTypeAndContent:
            case CLabel::eDescriptionBrief:
            case CLabel::eDescription:
                *str += CGUIUserType::sm_Tp_ProjectItem;
                *str = " : " + item->GetLabel();
                break;

            default:
                break;
            }
        }
        /* the old way of getting the Label, now it shall be set explicitly
        const CObject* obj = item->GetObject();
        if (obj) {
            CLabel::GetLabel(*obj, str, type, scope);
        }*/
    }
};


/// *******************************************************************
/// Moved from plugins/algo/init
/// *******************************************************************

///
/// CBioTreeContainer label handler
///
class CBioTreeContainerHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
		// check: no scope info is actually used

		const CBioTreeContainer* tree =
            dynamic_cast<const CBioTreeContainer*>(&obj);
        if (tree) {
            switch (type) {
            case CLabel::eUserType:
                *label += tree->CanGetTreetype() ? tree->GetTreetype() : "Bio Tree";
                break;

            case CLabel::eUserSubtype:
                break;

            case CLabel::eType:
                *label += "Biological Tree";
                break;

            case CLabel::eContent:
                {{
                    string str;

                    if (tree->IsSetLabel()) {
                        str.append("'");
                        str += tree->GetLabel();
                        str.append("'");
                        str += " ";
                    }

                    str += NStr::SizetToString(tree->GetNodeCount());
                    str += " nodes, ";
                    str += NStr::SizetToString(tree->GetLeafCount());
                    str += " leaves";

                    if (tree->IsSetTreetype()) {
                        *label += tree->GetTreetype();
                        *label += " (" + str + ")";
                    } else {
                        *label += "Tree, " + str;
                    }
                }}
                break;

            default:
            case CLabel::eUserTypeAndContent:
                GetLabel(obj, label, CLabel::eType, scope);
                *label += ": ";
                GetLabel(obj, label, CLabel::eContent, scope);
                break;
            }
        }
    }
};


///
/// CGCAsssemblyHandler label handler
///
class CGCAsssemblyHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
		// check: no scope info is actually used

		const CGC_Assembly* assm = dynamic_cast<const CGC_Assembly*>(&obj);
        if (assm) {
            switch (type) {
            case CLabel::eUserType:
            case CLabel::eType:
                *label += "Assembly";
                break;

            case CLabel::eUserSubtype:
                break;


            case CLabel::eContent:
                *label += assm->GetName();
                break;

            case CLabel::eDescriptionBrief:
            case CLabel::eDescription:
                {{
                    const CGC_AssemblyDesc& desc = assm->GetDesc();
                    if (desc.IsSetLong_name()) {
                        *label += desc.GetLong_name();
                    } else if (desc.IsSetName()) {
                        *label += desc.GetName();
                    }
                }}
                break;

            default:
            case CLabel::eUserTypeAndContent:
                GetLabel(obj, label, CLabel::eType, scope);
                *label += ": ";
                GetLabel(obj, label, CLabel::eContent, scope);
                break;
            }
        }
    }
};


///
/// CDistanceMatrix label handler
///
class CDistanceMatrixHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType /*type*/, objects::CScope* /*scope*/) const
    {
		// check: no scope info is used

        const CDistanceMatrix * mat =
            dynamic_cast<const CDistanceMatrix*>(&obj);

        if (mat) {
            *label += "Distance Matrix";
        }
    }
};


///
/// CEntrezgene label handler
///
class CEntrezgeneHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
		// check: no scope info is actually used

        const CEntrezgene* egene =
            dynamic_cast<const CEntrezgene*>(&obj);
        if (egene) {
            switch (type) {
            case CLabel::eUserType:
                *label += CGUIUserType::sm_Tp_EntrezGeneRecord;
                break;

            case CLabel::eUserSubtype:
                break;

            case CLabel::eType:
                *label += "Entrez Gene Record";
                break;

            case CLabel::eContent:
                egene->GetGene().GetLabel(label);
                *label += " [";
                egene->GetSource().GetOrg().GetLabel(label);
                *label += "]";
                break;

            case CLabel::eDescription:
                egene->GetGene().GetLabel(label);
                if (egene->GetGene().IsSetDesc()) {
                    *label += " : " + egene->GetGene().GetDesc();
                }
                *label += " [";
                egene->GetSource().GetOrg().GetLabel(label);
                *label += "]";
                break;

            default:
            case CLabel::eUserTypeAndContent:
                GetLabel(obj, label, CLabel::eType, scope);
                *label += ": ";
                GetLabel(obj, label, CLabel::eContent, scope);
                break;
            }
        }
    }
};


///
/// CEntrezgene_Set label handler
///
class CEntrezgene_SetHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
		// check: no scope info is actually used

        const CEntrezgene_Set* egene_set =
            dynamic_cast<const CEntrezgene_Set*>(&obj);
        if (egene_set) {
            switch (type) {
            case CLabel::eUserType:
            case CLabel::eType:
                *label += "Entrez Gene Set";
                break;

            case CLabel::eContent:
                {{
                    string s;
                    ITERATE (CEntrezgene_Set::Tdata, iter, egene_set->Get()) {
                        if ( !s.empty() ) {
                            s += "; ";
                        }
                        (*iter)->GetGene().GetLabel(&s);
                        s += " (";
                        (*iter)->GetSource().GetOrg().GetLabel(&s);
                        s += ")";
                    }
                    *label += s;
                }}
                break;

            default:
            case CLabel::eUserTypeAndContent:
                GetLabel(obj, label, CLabel::eType, scope);
                *label += ": ";
                GetLabel(obj, label, CLabel::eContent, scope);
                break;
            }
        }
    }
};

///
/// CVariation label handler
///
class CVariationHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* label,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
        const CVariation* pVariation(dynamic_cast<const CVariation*>(&obj));
        if(pVariation) {
            switch (type) {
            case CLabel::eUserType:
				*label += CGUIUserType::sm_Tp_Variation;
                break;

            case CLabel::eUserSubtype:
                break;

            case CLabel::eType:
                *label += "Variation";
                break;

            case CLabel::eContent:
				if(pVariation->CanGetId()) {
					pVariation->GetId().GetLabel(label);
				}
				if(pVariation->CanGetOther_ids() && !pVariation->GetOther_ids().empty()) {
					*label += "[";
					const CVariation::TOther_ids& OtherIds(pVariation->GetOther_ids());
					ITERATE(CVariation::TOther_ids, iOtherIds, OtherIds) {
						if(iOtherIds != OtherIds.begin())
							*label += ", ";
						(*iOtherIds)->GetLabel(label);
					*label += "]";
					}
				}
                break;

            case CLabel::eDescription:
				if(pVariation->CanGetDescription())
					*label += pVariation->GetDescription();
				break;

            default:
                GetLabel(obj, label, CLabel::eType, scope);
                *label += ": ";
                GetLabel(obj, label, CLabel::eContent, scope);
                break;
            }
        }
    }
};




//
// force registration at load
//

struct SForceRegister
{
    SForceRegister()
    {
        CLabel::RegisterLabelHandler(*CSeq_id::GetTypeInfo(),     *new CSeq_idHandler());
        CLabel::RegisterLabelHandler(*CSeq_loc::GetTypeInfo(),    *new CSeq_locHandler());
        CLabel::RegisterLabelHandler(*CBioseq::GetTypeInfo(),     *new CBioseqHandler());
        CLabel::RegisterLabelHandler(*CBioseq_set::GetTypeInfo(), *new CBioseq_setHandler());
        CLabel::RegisterLabelHandler(*CSeq_feat::GetTypeInfo(),   *new CSeq_featHandler());
        CLabel::RegisterLabelHandler(*CSeq_entry::GetTypeInfo(),  *new CSeq_entryHandler());
        CLabel::RegisterLabelHandler(*CSeq_annot::GetTypeInfo(),  *new CSeq_annotHandler());
        CLabel::RegisterLabelHandler(*CSeq_align::GetTypeInfo(),  *new CSeq_alignHandler());
        CLabel::RegisterLabelHandler(*CSeq_align_set::GetTypeInfo(),    *new CSeq_align_setHandler());
        CLabel::RegisterLabelHandler(*CValidError::GetTypeInfo(),       *new CValidErrorHandler());
        // TBD: Need support for GetTypeInfo to see taxid collection in selection inspector
        //CLabel::RegisterLabelHandler(*CTaxIdSelSet::GetTypeInfo(),       *new CTaxIDSetLabelHandler());

        // Originially located in plugins/algo/init
        CLabel::RegisterLabelHandler(*CBioTreeContainer::GetTypeInfo(), *new CBioTreeContainerHandler);
        CLabel::RegisterLabelHandler(*CGC_Assembly::GetTypeInfo(),      *new CGCAsssemblyHandler);
        CLabel::RegisterLabelHandler(*CDistanceMatrix::GetTypeInfo(),   *new CDistanceMatrixHandler);
        CLabel::RegisterLabelHandler(*CEntrezgene::GetTypeInfo(),       *new CEntrezgeneHandler);
		CLabel::RegisterLabelHandler(*CVariation::GetTypeInfo(),  *new CVariationHandler);
        CLabel::RegisterLabelHandler(*CGBProject_ver2::GetTypeInfo(), *new CGBProject_ver2LabelHandler);
        CLabel::RegisterLabelHandler(*CProjectItem::GetTypeInfo(), *new CProjectItemLabelHandler);
    }
};


static SForceRegister sm_ForceRegistration;

END_NCBI_SCOPE
