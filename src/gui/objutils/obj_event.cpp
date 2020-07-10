/*  $Id: obj_event.cpp 38850 2017-06-26 17:44:57Z katargir $
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

#include <gui/objutils/obj_event.hpp>

#include <corelib/ncbitime.hpp>
#include <serial/iterator.hpp>

#include <objects/seq/seq_id_mapper.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/objutils/utils.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CSelectionEvent::TRawNameToType    CSelectionEvent::sm_RawNameToType;

CSelectionEvent::EObjMatchPolicy
    CSelectionEvent::sm_ObjMatchPolicy = CSelectionEvent::eAtLeastOne;
CSelectionEvent::EIdMatchPolicy
    CSelectionEvent::sm_IdMatchPolicy = CSelectionEvent::eAccOnly;
bool    CSelectionEvent::sm_MatchAlnLocs = true;
bool    CSelectionEvent::sm_MatchByProduct = true;
bool    CSelectionEvent::sm_AutoBroadcast = true;
bool    CSelectionEvent::sm_InterDocBroadcast = true;
vector<string> CSelectionEvent::sm_TreeBroadcastProperties;
bool    CSelectionEvent::sm_TreeBroadcastOneToOne = true;



CSelectionEvent::CSelectionEvent( objects::CScope& scope )
    : m_Scope( &scope )
    , m_HasRangeSelection( false )
    , m_HasObjectSelection( false )
{
    x_Init();
}

void CSelectionEvent::x_Init()
{
    if( sm_RawNameToType.size() == 0 ){ // fill the map only once
        string  feat_key = typeid(CSeq_feat).name();
        string  align_key = typeid(CSeq_align).name();
        string  loc_key = typeid(CSeq_loc).name();
        string  id_key = typeid(CSeq_id).name();

        sm_RawNameToType[feat_key] = eSeq_feat;
        sm_RawNameToType[align_key] = eSeq_align;
        sm_RawNameToType[loc_key] = eSeq_loc;
        sm_RawNameToType[id_key] = eSeq_id;
    }
}




bool    CSelectionEvent::HasRangeSelection() const
{
    return m_HasRangeSelection;
}


const CHandleRangeMap&  CSelectionEvent::GetRangeSelection() const
{
    return m_RangeMap;
}


void CSelectionEvent::AddRangeSelection(const CSeq_id& id, const TRangeColl& segs)
{
    m_HasRangeSelection = true;

    CSeq_id_Handle h_id = CSeq_id_Handle::GetHandle(id);
    if (segs.size()) {
        ITERATE(TRangeColl, it, segs)   {
            m_RangeMap.AddRange(h_id, *it, eNa_strand_both);
        }
    } else {
        CRef<CSeq_loc> loc(new CSeq_loc());
        loc->SetEmpty().Assign(id);
        AddRangeSelection(*loc);
    }
}

void CSelectionEvent::AddRangeSelection(const objects::CSeq_loc& loc)
{
    m_HasRangeSelection = true;

    m_RangeMap.AddLocation(loc);
}


bool    CSelectionEvent::GetRangeSelection(const CSeq_id& id, CScope& scope,
                                           TRangeColl& segs)
{
    bool ok = false;
    const CHandleRangeMap::TLocMap& loc_map = m_RangeMap.GetMap();
    ITERATE(CHandleRangeMap::TLocMap, it, loc_map)  { // for every CSeq_id
        CConstRef<CSeq_id> sel_id = it->first.GetSeqId();

        if(Match(*sel_id, *m_Scope, id, scope))   {
            bool res = CSeqUtils::GetRangeCollection(*sel_id, m_RangeMap, segs);
            ok |= res;
        }
    }
    return ok;
}


void CSelectionEvent::GetRangeSelectionAsLocs(TConstObjects& objs) const
{
    const CHandleRangeMap::TLocMap& loc_map = m_RangeMap.GetMap();
    CSeq_loc::TRanges ranges;

    ITERATE(CHandleRangeMap::TLocMap, it, loc_map)  { // for every CSeq_id
        CRef<CSeq_id> sel_id(new CSeq_id());
        sel_id->Assign(*it->first.GetSeqId());

        const CHandleRange& h_range = it->second;

        // create a CSeq_loc
        ranges.clear();
        ITERATE(CHandleRange, it_r, h_range) {
            if (it_r->first.NotEmpty()) {
                ranges.push_back(it_r->first);
            }
        }
        if(! ranges.empty())    {
            CSeq_loc* loc = new CSeq_loc(*sel_id, ranges);
            objs.push_back(CConstRef<CObject>(loc));
        }
    }
}


bool    CSelectionEvent::HasObjectSelection()
{
    return m_HasObjectSelection;
}


bool CSelectionEvent::AddObjectSelection(const CObject& obj)
{
    const type_info& info = typeid(obj);
    if(info == typeid(CSeq_feat))   {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
        AddObjectSelection(*feat);
        return true;
    } else if(info == typeid(CSeq_align)) {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
        AddObjectSelection(*align);
        return true;
    } else if(info == typeid(CSeq_loc)) {
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
        AddObjectSelection(*loc);
        return true;
    } else if(info == typeid(CSeq_id)) {
        const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
        AddObjectSelection(*id);
        return true;
    }
    return false;
}


void CSelectionEvent::AddObjectSelection(const TConstObjects& objs)
{
    ITERATE(TConstObjects, it, objs)  {
        const CObject& obj = **it;
        const type_info& info = typeid(obj);

        if(info == typeid(CSeq_feat))   {
           const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
           AddObjectSelection(*feat);
        } else if(info == typeid(CSeq_align)) {
            const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
            AddObjectSelection(*align);
        } else if(info == typeid(CSeq_loc)) {
            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(&obj);
            AddObjectSelection(*loc);
        } else if(info == typeid(CSeq_id)) {
            const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
            AddObjectSelection(*id);
        } else {
			m_Other.push_back(CConstRef<CObject>(&obj));
		}
    }
}

void CSelectionEvent::AddIndexed(const string& area_name, const CObject& obj)
{
    m_Indexed[area_name].push_back(CConstRef<CObject>(&obj));
}

void CSelectionEvent::GetIndexed(const string& area_name, TConstObjects& objs) const
{
    TIndexed::const_iterator it = m_Indexed.find(area_name);
    if (it != m_Indexed.end()) {
        copy(it->second.begin(), it->second.end(), back_inserter(objs));
    }
}

void CSelectionEvent::AddObjectSelection(const CSeq_id& id)
{
    m_HasObjectSelection = true;

    m_Ids.push_back(CConstRef<CSeq_id>(&id));
}


void CSelectionEvent::AddObjectSelection(const CSeq_feat& feat)
{
    m_HasObjectSelection = true;

    m_Feats.push_back(CConstRef<CSeq_feat>(&feat));

    if(sm_MatchByProduct  &&  feat.IsSetProduct())   {
        const CSeq_loc& prod_loc = feat.GetProduct();
        const CSeq_id* id = prod_loc.GetId();
        if(id) {
            m_Ids.push_back(CConstRef<CSeq_id>(id));
        }
    }
}


void CSelectionEvent::AddObjectSelection(const CSeq_align& align)
{
    m_HasObjectSelection = true;

    m_Aligns.push_back(CConstRef<CSeq_align>(&align));
}



void CSelectionEvent::AddObjectSelection(const CSeq_loc& loc)
{
    m_HasObjectSelection = true;

    m_SeqLocs.push_back(CConstRef<CSeq_loc>(&loc));
}


void CSelectionEvent::AddObjectSelection(const objects::CSeq_id& id, const CRange<TSeqPos>& range)
{
    m_HasObjectSelection = true;

    CIdLoc* loc = new CIdLoc();
    loc->m_Id.Reset(&id);
    loc->m_Range = range;
    m_IdLocs.push_back(CConstRef<CIdLoc>(loc));
}

void CSelectionEvent::AddTaxIDSelection(TTaxId tid) 
{
    m_HasObjectSelection = true;

    m_TaxIds.SelectTaxId(tid);
}


void CSelectionEvent::GetAllObjects(TConstObjects& objs) const
{
    typedef CConstRef<CObject> TR;

    ITERATE(TFeats, it_feat, m_Feats)    {
        objs.push_back(TR(&**it_feat));
    }
    ITERATE(TAligns, it_align, m_Aligns)    {
        objs.push_back(TR(&**it_align));
    }
    ITERATE(TSeqLocs, it_loc, m_SeqLocs)    {
        objs.push_back(TR(&**it_loc));
    }
    ITERATE(TIds, it_id, m_Ids)    {
        objs.push_back(TR(&**it_id));
    }
    ITERATE(TIdLocs, it_loc, m_IdLocs)    {
        objs.push_back(TR(&**it_loc));
    }
}

void CSelectionEvent::GetOther(TConstObjects& objs) const
{
    ITERATE(TOther, it, m_Other)    {
        objs.push_back(*it);
    }
}


bool CSelectionEvent::Match(const CObject& obj1, CScope& scope1,
                            const CObject& obj2, CScope& scope2)
{
    if(&obj1 == &obj2)  { // trivial matching
        return true;
    }

    const CSeq_feat *feat1 = NULL, *feat2 = NULL;
    const CSeq_align *align1 = NULL, *align2 = NULL;
    const CSeq_loc *loc1 = NULL, *loc2 = NULL;
    const CSeq_id *id1 = NULL, *id2 = NULL;

    const type_info& info2 = typeid(obj2);
    string name2 = info2.name();
    TRawNameToType::const_iterator it = sm_RawNameToType.find(name2);
    if(it == sm_RawNameToType.end())    {
        return false; // unsupported type
    }
    EObjType type2 = it->second;

    switch(type2)   {
    case eSeq_feat: feat2 = dynamic_cast<const CSeq_feat*>(&obj2); break;
    case eSeq_align:    align2 = dynamic_cast<const CSeq_align*>(&obj2); break;
    case eSeq_loc:      loc2 = dynamic_cast<const CSeq_loc*>(&obj2); break;
    case eSeq_id:       id2 = dynamic_cast<const CSeq_id*>(&obj2); break;
    }

    const type_info& info1 = typeid(obj1);
    string name1 = info1.name();
    it = sm_RawNameToType.find(name1);
    if(it == sm_RawNameToType.end())    {
        return false; // unsupported type
    }
    EObjType type1 = it->second;

    switch(type1)   {
    case eSeq_feat:
        feat1 = dynamic_cast<const CSeq_feat*>(&obj1);
        switch(type2)   {
        case eSeq_feat: return MatchFeatWithFeat(*feat1, scope1, *feat2, scope2);
        case eSeq_align: return false;
        case eSeq_loc:  return MatchFeatWithLoc(*feat1, scope1, *loc2, scope2);
        case eSeq_id:   return MatchFeatWithId(*feat1, scope1, *id2, scope2);
        }
    case eSeq_align:
        align1 = dynamic_cast<const CSeq_align*>(&obj1);
        if(type2 == eSeq_align) {
            return MatchAlignWithAlign(*align1, scope1, *align2, scope2);
        } else {
            return false;
        }
    case eSeq_loc:
        loc1 = dynamic_cast<const CSeq_loc*>(&obj1);
        switch(type2)   {
        case eSeq_feat: return MatchFeatWithLoc(*feat2, scope1, *loc1, scope2);
        case eSeq_align: return false;
        case eSeq_loc:  return MatchLocWithLoc(*loc1, scope1, *loc2, scope2);
        case eSeq_id:   return MatchLocWithId(*loc1, scope1, *id2, scope2);
        }
    case eSeq_id:
        id1 = dynamic_cast<const CSeq_id*>(&obj1);
        switch(type2)   {
        case eSeq_feat: return MatchFeatWithId(*feat2, scope1, *id1, scope2);
        case eSeq_align: return false;
        case eSeq_loc:  return MatchLocWithId(*loc2, scope1, *id1, scope2);
        case eSeq_id:   return MatchIdWithId(*id1, scope1, *id2, scope2);
        }
    }

    _ASSERT(false); // must be unreachable
    return false;
}


bool CSelectionEvent::MatchFeatWithFeat(const CSeq_feat& feat1, CScope& scope1,
                                        const CSeq_feat& feat2, CScope& scope2)
{
    // TO DO extend to support comparision by values (not by pointer)
    return &feat1 == &feat2  ||  feat1.Equals(feat2);
}


bool CSelectionEvent::MatchFeatWithLoc(const CSeq_feat& feat1, CScope& scope1,
                                       const CSeq_loc& loc2, CScope& scope2)
{
    if(feat1.CanGetLocation())  {
        const CSeq_loc& feat_loc = feat1.GetLocation();
        if(MatchLocWithLoc(feat_loc, scope1, loc2, scope2)) {
            return true;
        }
    }
    if(sm_MatchByProduct  &&  feat1.CanGetProduct())   {
        const CSeq_loc& prod_loc = feat1.GetProduct();
        if(MatchLocWithLoc(prod_loc, scope1, loc2, scope2)) {
            return true;
        }
    }
    return false;
}


bool CSelectionEvent::MatchFeatWithId(const CSeq_feat& feat1, CScope& scope1,
                                      const CSeq_id& id2, CScope& scope2)
{
    if(sm_MatchByProduct  &&  feat1.IsSetProduct())   {
        const CSeq_loc& prod_loc = feat1.GetProduct();
        if(MatchLocWithId(prod_loc, scope1, id2, scope2)) {
            return true;
        }
    }
    return false;
}


bool CSelectionEvent::MatchAlignWithAlign(const CSeq_align& align1, CScope& scope1,
                                          const CSeq_align& align2, CScope& scope2)
{
    // TO DO extend to support comparision by values (not by pointer)
    return &align1 == &align2  ||  align1.Equals(align2);
}


bool CSelectionEvent::MatchLocWithLoc(const CSeq_loc& loc1, CScope& scope1,
                                      const CSeq_loc& loc2, CScope& scope2)
{
    // TO DO extend to support flexiable ID matching
    return &loc1 == &loc2  ||  loc1.Equals(loc2);
}


bool CSelectionEvent::MatchLocWithId(const CSeq_loc& loc1, CScope& scope1,
                                     const CSeq_id& id2, CScope& scope2)
{
    // iterate by all CSeq_id-s in the loc1 and try to match them with id2
    set<CSeq_id_Handle> id_set;
    CTypeConstIterator<CSeq_id> id_it(loc1);
    for ( ;  id_it;  ++id_it) {
        id_set.insert(CSeq_id_Handle::GetHandle(*id_it));
    }

    switch(sm_ObjMatchPolicy)    {
    case eAtLeastOne:
        {{
            CSeq_id_Handle id2h = CSeq_id_Handle::GetHandle(id2);
            if (id_set.find(id2h) != id_set.end()) {
                return true;
            }
        }}

    default:
        ITERATE (set<CSeq_id_Handle>, iter, id_set) {
            if (MatchIdWithId(*iter->GetSeqId(), scope1, id2, scope2))   {
                return true;
            }
        }
        break;
    }

    return false;
}

struct SW
{
    CStopWatch  m_W;
    SW()    { m_W.Start();  }
    ~SW()   {   LOG_POST(Info << "Time " << m_W.Elapsed()); }
};


bool    CSelectionEvent::MatchIdWithId(const CSeq_id& sel_id, CScope& scope1,
                                       const CSeq_id& id, CScope& scope2)
{
    //SW w;

    switch(sm_ObjMatchPolicy)    {
    case eAllIds:   {
        vector<CSeq_id_Handle> ids1 = scope1.GetIds(sel_id);
        vector<CSeq_id_Handle> ids2 = scope2.GetIds(id);
        if(ids1.size() == ids2.size())  {
            sort(ids1.begin(), ids1.end());
            sort(ids2.begin(), ids2.end());
            for( size_t i = 0; i < ids1.size(); i++ )   {
                if(! x_SimpleMatch(ids1[i], ids2[i]))
                    return false;
            }
            return true;
        } else return false;
    }
    case eAtLeastOne: {
        vector<CSeq_id_Handle> ids1 = scope1.GetIds(sel_id);
        vector<CSeq_id_Handle> ids2 = scope2.GetIds(id);
        for( size_t i = 0; i < ids1.size(); i++ )   {
            for( size_t j = 0; j < ids2.size(); j++)    {
                if(x_SimpleMatch(ids1[i], ids2[j]))
                    return true;
            }
        }
        return false;
    }
    }
    _ASSERT(false); // must be unreachable
    return false;
}


bool    CSelectionEvent::x_SimpleMatch(const objects::CSeq_id_Handle& h_sel_id,
                                     const objects::CSeq_id_Handle& h_id)
{
    //cout << "\nx_SimpleMatch()  " << h_sel_id.AsString() << "  " << h_id.AsString();
    switch(sm_IdMatchPolicy) {
    case eAccOnly:
        return h_sel_id.MatchesTo(h_id) || h_id.MatchesTo(h_sel_id);
    case eExact:
        return h_sel_id == h_id;
    };
    _ASSERT(false); // must be unreachable
    return false; 
}


END_NCBI_SCOPE
