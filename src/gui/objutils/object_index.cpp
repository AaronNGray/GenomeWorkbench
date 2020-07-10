/*  $Id: object_index.cpp 38779 2017-06-16 16:27:59Z katargir $
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

#include <gui/objutils/object_index.hpp>
#include <gui/objutils/obj_event.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


// if "id_handle" represent an accession - return pointer to a orresponding text id
inline const CConstRef<CSeq_id>   GetAccession(const CSeq_id_Handle& id_handle)
{
    if(id_handle.HaveMatchingHandles()) {
        CConstRef<CSeq_id> seq_id = id_handle.GetSeqId();
        const CTextseq_id* text_id = seq_id->GetTextseq_Id();
        return (text_id  &&  text_id->IsSetAccession()) ? seq_id : CRef<CSeq_id>();
    }
    return CRef<CSeq_id>();
}


inline bool IsAccessionNoVersion(const CSeq_id& seq_id)
{
    const CTextseq_id* text_id = seq_id.GetTextseq_Id();
    return text_id->IsSetVersion()  &&  ! text_id->IsSetName() &&
           ! text_id->IsSetRelease();
}

///////////////////////////////////////////////////////////////////////////////
/// CSeq_id_Descr

CSeq_id_Descr::CSeq_id_Descr()
:   m_OriginalAccNoVer(false)
{
}


CSeq_id_Descr::CSeq_id_Descr(ISelObjectHandle* obj_handle, const CSeq_id& id,
                             CScope& scope, bool product)
:   m_OriginalAccNoVer(false)
{
    Assign(obj_handle, id, scope, product);
}


CSeq_id_Descr::~CSeq_id_Descr()
{
}


void CSeq_id_Descr::Assign(ISelObjectHandle* obj_handle, const CSeq_id& id,
                           CScope& scope, bool product)
{
    m_ObjHandle = obj_handle;
    m_Id.Reset(&id);
    m_IdHandles = scope.GetIds(id); // extract all synonymous Ids
    if (m_IdHandles.size() == 0) {
        m_IdHandles.push_back(CSeq_id_Handle::GetHandle(id));
    }
    sort(m_IdHandles.begin(), m_IdHandles.end());
    m_Product = product;

    // find or create a versionless accession
    CConstRef<CSeq_id> acc_id;

    for( size_t i = 0;  i < m_IdHandles.size();  i++ )  { // iterate by handles
        const CSeq_id_Handle& handle = m_IdHandles[i];
        acc_id = GetAccession(handle);
        if(acc_id)  {
            if(IsAccessionNoVersion(*acc_id))   {  // already versionless
                m_AccNoVer = handle;
                m_OriginalAccNoVer = true;
                return;
            }
            break; // end loop with this acc_id
        }
    }

    if(acc_id)  {   // has accession with version - create a versionless one
        m_OriginalAccNoVer = false;

        CSeq_id id;
        id.Select(acc_id->Which());
        const CTextseq_id* text_id = acc_id->GetTextseq_Id();
        const_cast<CTextseq_id*>(id.GetTextseq_Id())->SetAccession(text_id->GetAccession());
        m_AccNoVer = CSeq_id_Handle::GetHandle(id);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CSeq_feat_Descr

CSeq_feat_Descr::CSeq_feat_Descr()
    : m_ProductDescr(NULL)
{
}


CSeq_feat_Descr::~CSeq_feat_Descr()
{
    delete m_ProductDescr;
}


void CSeq_feat_Descr::Assign(ISelObjectHandle* obj_handle, const CSeq_feat& feat,
                             CSeq_id_Descr* product_descr, CScope& scope)
{
    m_ObjHandle.Reset(obj_handle);
    m_Feat.Reset(&feat);
    m_ProductDescr = product_descr;
}


///////////////////////////////////////////////////////////////////////////////
/// CObjectIndex

CObjectIndex::SRec::SRec(CSeq_id_Descr* descr, bool original, bool product)
:   m_Original(original),
    m_Product(product),
    m_Descr(descr)
{
}


inline const CSeq_id_Descr* CObjectIndex::SRec::GetIdDescr() const
{
    //_ASSERT( ! m_HasLoc);
    return m_Descr;
}


CObjectIndex::CObjectIndex()
{
}


CObjectIndex::~CObjectIndex()
{
    Clear(NULL);
}


void CObjectIndex::Clear(CScope* scope)
{
    // clean-up Features
    NON_CONST_ITERATE(TFeatDescrs, it_f, m_FeatDescrs)   { //TODO ?
        const CSeq_id_Descr* id = it_f->second->GetProductDescr();
        TIdDescrs::iterator it_id =
            std::find(m_IdDescrs.begin(), m_IdDescrs.end(), id);
        if (it_id != m_IdDescrs.end()) {
            m_IdDescrs.erase(it_id);
        }

        delete it_f->second;
    }
    m_FeatDescrs.clear();
    m_FeatMap.clear();

    // clean-up Ids
    NON_CONST_ITERATE(TIdDescrs, it_i, m_IdDescrs)   { //TODO ?
        delete *it_i;
    }
    m_IdDescrs.clear();
    m_IdMap.clear();

    // clean-up Aligns
    NON_CONST_ITERATE(TAlignMap, it_a, m_AlignMap)  {
        delete it_a->second;
    }
    m_AlignMap.clear();

    m_Scope.Reset(scope);
}


void CObjectIndex::Add(ISelObjectHandle* obj_handle, const CSeq_id& id, bool product)
{
    x_Add(obj_handle, id, product);
}


CSeq_id_Descr* CObjectIndex::x_Add(ISelObjectHandle* obj_handle,
                                 const CSeq_id& id, bool product)
{
    CSeq_id_Descr* descr = new CSeq_id_Descr();
    descr->Assign(obj_handle, id, *m_Scope, product);
    m_IdDescrs.push_back(descr);

    if( ! descr->HasOriginalAccNoVer())    {
        CSeq_id_Handle idh = descr->GetAccNoVer();
        if (idh) {
            // add created versionless accession to the index
            SRec rec(descr, false, product);
            m_IdMap.insert(TIdMap::value_type(idh, rec));
        }
    }
    // otherwise versionless accession will be indexed together with all other
    // handles and will have equal rights in matching

    // add to index all regular handles
    const vector<CSeq_id_Handle>& handles = descr->GetIdHandles();
    for( size_t i = 0;  i < handles.size();  i++ )  {
        const CSeq_id_Handle& h = handles[i];
        SRec rec(descr, true, product);
        m_IdMap.insert(TIdMap::value_type(h, rec));
    }

    return descr;
}


// TODO - optimize  by bit reversing
inline CObjectIndex::TFeatCode CObjectIndex::x_GetFeatureCode(const CSeq_feat& feature)
{
    const CSeq_loc& loc = feature.GetLocation();
    unsigned int start = 0;
    unsigned int stop = 0;
    try {
        start = loc.GetStart(eExtreme_Positional);
        stop = loc.GetStop(eExtreme_Positional);
    } catch (CException&) {
        try {
            TSeqRange total_r = loc.GetTotalRange();
            start = total_r.GetFrom();
            stop = total_r.GetTo();
        } catch (CException& e) {
            ERR_POST("Error on CObjectIndex::x_GetFeatureCode(): " << e.GetMsg());
        }
    }
    unsigned int res = start ^ ((start - stop) << 16);

    const CSeqFeatData& data = feature.GetData();
    unsigned int type = (unsigned int) data.GetSubtype();
    return res ^ type;
}


void CObjectIndex::Add(ISelObjectHandle* obj_handle, const CSeq_feat& feature)
{
    TFeatDescrs::const_iterator it = m_FeatDescrs.find(&feature);
    if(it != m_FeatDescrs.end())    {
        ERR_POST("CObjectIndex::Add() - feature already in index");
        return; // already in the index
    }

    CSeq_id_Descr* product_descr = NULL;

    // index Product ID
    if(feature.IsSetProduct()) {
        const CSeq_loc& prod_loc = feature.GetProduct();
        const CSeq_id* id = prod_loc.GetId();
        if(id) {
            product_descr = x_Add(obj_handle, *id, true);
        }
    }

    // create descriptior and put in the storage map
    CSeq_feat_Descr* descr = new CSeq_feat_Descr();
    descr->Assign(obj_handle, feature, product_descr, *m_Scope);
    m_FeatDescrs.insert(make_pair(&feature, descr)); // index by pointer

    TFeatCode code = x_GetFeatureCode(feature);
    m_FeatMap.insert(TCodeToFeatMap::value_type(code, descr)); // index by code
}


void CObjectIndex::Add(ISelObjectHandle* obj_handle, const CSeq_align& align)
{
    m_AlignMap.insert( make_pair(CConstRef<CSeq_align>(&align), obj_handle) );
}


void CObjectIndex::Add(ISelObjectHandle* obj_handle, CObject& obj)
{
    const type_info& info = typeid(obj);
    if(info == typeid(CSeq_feat))   {
        const CSeq_feat* feature = dynamic_cast<const CSeq_feat*>(&obj);
        Add(obj_handle, *feature);
    } else if(info == typeid(CSeq_id))   {
        const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
        Add(obj_handle, *id);
    } else if(info == typeid(CIdLoc))   {
        const CIdLoc* loc = dynamic_cast<const CIdLoc*>(&obj);
        Add(obj_handle, *loc->m_Id);
    } else if(info == typeid(CSeq_align))   {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
        Add(obj_handle, *align);
    } else {
        ERR_POST("ObjectIndex::Add() - type not supported " << info.name());
    }
}


bool CObjectIndex::Remove(const CObject& obj)
{
    bool removed = false;
    const type_info& info = typeid(obj);
    if(info == typeid(CSeq_id))   {
        const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
        removed = Remove(*id);
    } else if(info == typeid(CSeq_feat))   {
        const CSeq_feat* feature = dynamic_cast<const CSeq_feat*>(&obj);
        removed = Remove(*feature);
    } else if(info == typeid(CSeq_align))   {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
        removed = Remove(*align);
    } else {
        ERR_POST("ObjectIndex::Remove() - type not supported " << info.name());
    }
    return removed;
}


bool CObjectIndex::Remove(const CSeq_id& id)
{
    NON_CONST_ITERATE(TIdDescrs, it, m_IdDescrs)    {
        CSeq_id_Descr* descr = *it;
        if(descr->GetId()->Match(id))   {
            m_IdDescrs.erase(it);
            x_RemoveFromIdMap(descr);
            delete descr;
            return true;
        }
    }
    return false;
}


void CObjectIndex::x_Remove(const CSeq_id_Descr* descr)
{
    if(descr)   {
        // remove it from m_IdDescrs vector
        TIdDescrs::iterator it_v = std::find(m_IdDescrs.begin(), m_IdDescrs.end(), descr);
        if(it_v != m_IdDescrs.end())    {
            m_IdDescrs.erase(it_v);
            x_RemoveFromIdMap(descr);
            //delete descr;
        }
    }
}


void CObjectIndex::x_RemoveFromIdMap(const CSeq_id_Descr* descr)
{
    // remove all associated IDs from ID map
    vector<CSeq_id_Handle> handles = descr->GetIdHandles();
    CSeq_id_Handle idh = descr->GetAccNoVer();
    if (idh) {
        handles.push_back(idh);
    }

    for( size_t i = 0;  i < handles.size();  i++ )  {
        const CSeq_id_Handle& h = handles[i];

        // for every handle in "descr" - find the entry and erase it
        pair<TIdMap::iterator , TIdMap::iterator> p = m_IdMap.equal_range(h);
        for( TIdMap::iterator it = p.first;  it != p.second;  ++it ) {
            const SRec& rec = it->second;
            if(rec.m_Descr == descr)    {
                m_IdMap.erase(it);
                break; // done with this handle
            }
        }
    }
}


bool CObjectIndex::Remove(const CSeq_feat& feature)
{
    TFeatDescrs::iterator it = m_FeatDescrs.find(&feature);

    TFeatCode code = x_GetFeatureCode(feature);
    pair<TCodeToFeatMap::iterator , TCodeToFeatMap::iterator> p =
        m_FeatMap.equal_range(code);
    // iterate by features that have the same code
    for( TCodeToFeatMap::iterator it_d = p.first;  it_d != p.second;  ++it_d ) {
        const CSeq_feat_Descr* descr = it_d->second;
        const CSeq_feat& feat = descr->GetFeat();
        bool found_match = false;
        if(it != m_FeatDescrs.end()) {
            if (&feat == &feature) {
                found_match = true;
            }
        } else {
            it = m_FeatDescrs.begin();
            while (it != m_FeatDescrs.end()  &&  it->second != descr) {
                ++it;
            }
            if(it != m_FeatDescrs.end()) {
                found_match = true;
            }
        }

        if (found_match) {
            m_FeatMap.erase(it_d);
            x_Remove(descr->GetProductDescr());
            delete descr;
            m_FeatDescrs.erase(it);
            return true;
        }
    }

    return false;
}


bool CObjectIndex::Remove(const CSeq_align& align)
{
    m_AlignMap.erase(CConstRef<CSeq_align>(&align));
    return true;
}


bool CObjectIndex::HasMatches(const CObject& object, CScope& scope) const
{
    // dispatch by type
    const type_info& info = typeid(object);
    if(info == typeid(CSeq_feat))   {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&object);
        return HasMatches(*feat, scope);
    } else if(info == typeid(CSeq_id))   {
        const CSeq_id* id = dynamic_cast<const CSeq_id*>(&object);
        CSeq_id_Descr descr;
        descr.Assign(NULL, *id, scope);
        return HasMatches(descr);
    } else if(info == typeid(CSeq_align))   {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(&object);
        return HasMatches(*align);
    }
    return false;
}


void CObjectIndex::GetMatches(const CObject& object, CScope& scope, TResults& results) const
{
    // dispatch by type
    const type_info& info = typeid(object);
    if(info == typeid(CSeq_feat))   {
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&object);
        GetMatches(*feat, scope, results);
    } else if(info == typeid(CSeq_id))   {
        const CSeq_id* id = dynamic_cast<const CSeq_id*>(&object);
        CSeq_id_Descr descr;
        descr.Assign(NULL, *id, scope);
        GetMatches(descr, results);
    } else if(info == typeid(CSeq_align))   {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(&object);
        GetMatches(*align, results);
    }
}


bool CObjectIndex::HasMatches(const CSeq_id_Descr& descr)  const
{
    const vector<CSeq_id_Handle>& handles = descr.GetIdHandles();
    bool at_least_one = (CSelectionEvent::sm_ObjMatchPolicy == CSelectionEvent::eAtLeastOne);
    bool no_ver = (CSelectionEvent::sm_IdMatchPolicy == CSelectionEvent::eAccOnly);

    // for every element in "handles" perform a lookup
    for( size_t i = 0;  i < handles.size();  i++ )  {
        const CSeq_id_Handle& h = handles[i];
        if(x_HasMatches(descr, h, at_least_one, no_ver))    {
            return true;
        }
    }

    if(no_ver  &&  ! descr.HasOriginalAccNoVer())  {
        // AccNoVer is not original so it is not in the "handles", need a separate lookup
        const CSeq_id_Handle& h = descr.GetAccNoVer();
        return x_HasMatches(descr, h, at_least_one, no_ver);
    }
    return false;
}


// for a given id returns the set of matching ids
void CObjectIndex::GetMatches(const CSeq_id_Descr& descr, TResults& results) const
{
    results.clear();

    const vector<CSeq_id_Handle>& handles = descr.GetIdHandles();
    bool at_least_one = (CSelectionEvent::sm_ObjMatchPolicy == CSelectionEvent::eAtLeastOne);
    bool no_ver = (CSelectionEvent::sm_IdMatchPolicy == CSelectionEvent::eAccOnly);

    // for every element in "handles" perform a lookup and add all results
    // to "results"
    for( size_t i = 0;  i < handles.size();  i++ )  {
        const CSeq_id_Handle& h = handles[i];
        x_GetMatches(descr, h, at_least_one, no_ver, results);
    }

    if(no_ver  &&  ! descr.HasOriginalAccNoVer())  {
        // AccNoVer is not original so it is not in the "handles", need a separate lookup
        const CSeq_id_Handle& h = descr.GetAccNoVer();
        x_GetMatches(descr, h, at_least_one, no_ver, results);
    }
}


bool CObjectIndex::x_HasMatches(const CSeq_id_Descr& descr, const CSeq_id_Handle& h,
                                bool at_least_one, bool no_ver) const
{
    pair<TIdMap::const_iterator , TIdMap::const_iterator> p = m_IdMap.equal_range(h);
    for( TIdMap::const_iterator it = p.first;  it != p.second;  ++it ) {
        const SRec& rec = it->second;
        if( ! rec.m_Product  ||  CSelectionEvent::sm_MatchByProduct)    {
            const CSeq_id_Descr* value = rec.GetIdDescr(); // TODO
            if(at_least_one  &&  (rec.m_Original  ||  no_ver))    {
                return true;
            } else if(value->GetIdHandles() == descr.GetIdHandles()) {  // exact match for all ids
                return true;
            }
        }
    }
    return false;
}

// retrieves matches for a single handle
void CObjectIndex::x_GetMatches(const CSeq_id_Descr& descr, const CSeq_id_Handle& h,
                                bool at_least_one, bool no_ver, TResults& results) const
{
    pair<TIdMap::const_iterator , TIdMap::const_iterator> p = m_IdMap.equal_range(h);
    for( TIdMap::const_iterator it = p.first;  it != p.second;  ++it ) {
        const SRec& rec = it->second;
        if( ! rec.m_Product  ||  CSelectionEvent::sm_MatchByProduct)    {
            const CSeq_id_Descr* value = rec.GetIdDescr(); // TODO
            CRef<ISelObjectHandle> ref = value->GetObjectHandle();
            const ISelObjectHandle* obj_handle = ref.GetPointer();

            if(at_least_one  &&  (rec.m_Original  ||  no_ver))    {
                results.insert(obj_handle);
            } else if(value->GetIdHandles() == descr.GetIdHandles()) {  // exact match for all ids
                results.insert(obj_handle);
            }
        }
    }
}


bool CObjectIndex::HasMatches(const CSeq_feat& feature, CScope& scope) const
{
    //// Matching against Features in the index
    // first - fast lookup by pointer
    if(m_FeatDescrs.find(&feature) != m_FeatDescrs.end())
        return true;

    // now try matching by value (not by pointer)
    TFeatCode code = x_GetFeatureCode(feature);
    pair<TCodeToFeatMap::const_iterator , TCodeToFeatMap::const_iterator> p =
        m_FeatMap.equal_range(code);

    // iterate by features that have the same code and check if they are indeed equal
    for( TCodeToFeatMap::const_iterator it = p.first;  it != p.second;  ++it ) {
        const CSeq_feat_Descr& descr = *it->second;
        const CSeq_feat& feat = descr.GetFeat();
        if(feat.Equals(feature))    {
            return true;
        }
    }

    //// Matching agains IDs
    if(CSelectionEvent::sm_MatchByProduct  &&  feature.IsSetProduct())   {
        const CSeq_loc& prod_loc = feature.GetProduct();
        const CSeq_id* id = prod_loc.GetId();
        if(id) {
            CSeq_id_Descr descr;
            descr.Assign(NULL, *id, scope);
            return HasMatches(descr);
        }
    }
    return false;
}

void CObjectIndex::GetMatches(const CSeq_feat& feature, CScope& scope,
                              TResults& results) const
{
    // skipping mathing by pointer - we will find the same object by code

    TFeatCode code = x_GetFeatureCode(feature);
    pair<TCodeToFeatMap::const_iterator , TCodeToFeatMap::const_iterator> p =
        m_FeatMap.equal_range(code);

    // iterate by features that have the same code
    for( TCodeToFeatMap::const_iterator it = p.first;  it != p.second;  ++it ) {
        const CSeq_feat_Descr& descr = *it->second;
        const CSeq_feat& feat = descr.GetFeat();
        if(&feat == &feature  ||  feat.Equals(feature)) {
            results.insert(descr.GetObjectHandle());
        }
    }

    //// Matching against IDs
    if(CSelectionEvent::sm_MatchByProduct  &&  feature.IsSetProduct())   {
        const CSeq_loc& prod_loc = feature.GetProduct();
        const CSeq_id* id = prod_loc.GetId();
        if(id) {
            CSeq_id_Descr descr;
            descr.Assign(NULL, *id, scope);
            GetMatches(descr, results);
        }
    }
}


bool CObjectIndex::HasMatches(const CSeq_align& align) const
{
    CConstRef<CSeq_align> ref(&align);
    return m_AlignMap.find(ref) != m_AlignMap.end();
}


void CObjectIndex::GetMatches(const CSeq_align& align, TResults& results) const
{
    CConstRef<CSeq_align> ref(&align);
    TAlignMap::const_iterator it = m_AlignMap.find(ref);
    if(it != m_AlignMap.end())  {
        results.insert(it->second);
    }
}


bool CObjectIndex::Empty() const
{
    return m_IdDescrs.empty()  &&  m_FeatDescrs.empty()  &&  m_AlignMap.empty();
}


void CObjectIndex::GetObjects(TConstObjects& objects) const
{
    typedef CConstRef<CObject> TR;

    ITERATE(TIdDescrs, it_i, m_IdDescrs)    {
        const CSeq_id_Descr* descr = *it_i;
        if( ! descr->IsProduct() || CSelectionEvent::sm_MatchByProduct) {
            const CSeq_id* id = descr->GetId();
            objects.push_back(TR(id));
        }
    }

    ITERATE(TFeatDescrs, it_f, m_FeatDescrs) {
        const CSeq_feat* feat = it_f->first;
        objects.push_back(TR(feat));
    }

    ITERATE(TAlignMap, it_a, m_AlignMap)    {
        const CSeq_align* align = &*it_a->first;
        objects.push_back(TR(align));
    }
}


void CObjectIndex::GetOriginalIds(vector<const CSeq_id*>& ids) const
{
    ITERATE(TIdDescrs, it_i, m_IdDescrs)    {
        const CSeq_id_Descr* descr = *it_i;
        if( ! descr->IsProduct()) {
            ids.push_back(descr->GetId());
        }
    }
}


void CObjectIndex::GetFeatures(vector<const CSeq_feat*>& features) const
{
    ITERATE(TFeatDescrs, it_f, m_FeatDescrs) {
        const CSeq_feat* feat = it_f->first;
        features.push_back(feat);
    }
}


void CObjectIndex::GetAligns(vector<const CSeq_align*>& aligns) const
{
    ITERATE(TAlignMap, it_a, m_AlignMap)    {
        const CSeq_align* align = &*it_a->first;
        aligns.push_back(align);
    }
}


END_NCBI_SCOPE
