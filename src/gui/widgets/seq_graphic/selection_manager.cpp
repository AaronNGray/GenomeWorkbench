/*  $Id: selection_manager.cpp 39118 2017-08-01 20:05:22Z evgeniev $
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
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/selection_manager.hpp>
#include <gui/widgets/seq_graphic/cds_glyph.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CSelectionVisitor::SetScope(objects::CScope* scope)
{ 
    m_Scope.Reset(scope); 
    Clear();
}

objects::CScope* CSelectionVisitor::GetScope()
{
    return m_Scope.GetPointer();
}

bool CSelectionVisitor::Visit(CSeqGlyph* glyph)
{
    switch (m_ActionMode) {
            case eAct_Select:
                return x_DoSelection(glyph);
            case eAct_Deselect:
                glyph->SetSelected(false);
                return true;
            case eAct_CollectSel:
                if (glyph->IsSelected()) {
                    m_SelGlyphs.push_back( CConstRef<CSeqGlyph>(glyph) );
                }
                return true;
            default:
                _ASSERT(false);
                return false;
    }
}


void CSelectionVisitor::SetSelectedObjectSig(const string& obj_sig)
{
    m_SelectedSignatures.clear();
    NStr::Split(obj_sig, ";,:", m_SelectedSignatures, NStr::fSplit_Tokenize);
    NON_CONST_ITERATE (list<string>, iter, m_SelectedSignatures) {
        NStr::TruncateSpacesInPlace(*iter);
    }
    // remove the empty ones
    m_SelectedSignatures.remove("");
}

void CSelectionVisitor::Clear()
{
    m_SelectedIndexes.Clear(m_Scope);
    m_SelectedObjects.clear();
    m_UnknownSelectedIndexes.Clear(m_Scope);

    m_SelFeatures.clear();
    m_SelCDSFeatures.clear();
    m_PostUpdateAction = ePUA_None;
}


bool CSelectionVisitor::IsObjectSelected(const CObject* obj)
{
    if (obj) {
        if (m_SelectedObjects.count(CConstRef<CObject>(obj)) > 0) {
            return true;
        }

        // The original object changes for these two cases:
        //  1. For component segments (the original objec tis a seq_id)
        //  2. For snp featuers
        // For this two cases, we need to fall back to CObjectIndex
        // to do the matching.
        bool use_index = false;
        if (typeid(*obj) == typeid(CSeq_id)) {
            use_index = true;
        } else if (typeid(*obj) == typeid(CSeq_feat)) {
            const CSeq_feat* feat_obj = dynamic_cast<const CSeq_feat*>(obj);
            if (feat_obj->GetData().GetSubtype() == CSeqFeatData::eSubtype_variation) {
                use_index = true;
            }
        }
        if (use_index  &&  m_SelectedIndexes.HasMatches(*obj, *m_Scope)) {
            return true;
        }
    }

    return false;
}


const CSeqGlyph::TConstObjects&
CSelectionVisitor::GetSelectedLayoutObjects(CSeqGlyph* top_glyph)
{
    m_SelGlyphs.clear();
    SetActionMode(eAct_CollectSel);
    top_glyph->Accept(this);

    return m_SelGlyphs;
}


void CSelectionVisitor::GetObjectSelection(TConstObjects& objs) const
{
    objs.reserve(m_SelectedObjects.size());
    ITERATE(TSelectedObjects, it, m_SelectedObjects) {
        objs.push_back(CConstRef<CObject>(*it));
    }
}


void CSelectionVisitor::ResetObjectSelection(CSeqGlyph* glyph)
{
    m_SelFeatures.clear();
    m_SelCDSFeatures.clear();
    if (x_NeedUpdate()) {
        m_SelectedIndexes.Clear(m_Scope);
        m_SelectedObjects.clear();
        m_UnknownSelectedIndexes.Clear(m_Scope);
        SetActionMode(eAct_Deselect);
        glyph->Accept(this);
    }
}


void CSelectionVisitor::DeSelectObject(const CObject* obj)
{
    if(obj  &&  IsObjectSelected(obj)) {
        if ( !m_SelectedIndexes.Empty() ) {
            m_SelectedIndexes.Remove(*obj);
        }
        TSelectedObjects::iterator iter = m_SelectedObjects.end();
        iter = m_SelectedObjects.find(CConstRef<CObject>(obj));
        if (iter == m_SelectedObjects.end()) {
            const CSeq_id* id = dynamic_cast<const CSeq_id*>(obj);
            if (id) {
                CSeq_id_Handle sih = sequence::GetId(*id, *m_Scope, sequence::eGetId_Canonical);
                NON_CONST_ITERATE (TSelectedObjects, o_iter, m_SelectedObjects) {
                    const CSeq_id* t_id = dynamic_cast<const CSeq_id*>(o_iter->GetPointer());
                    if (t_id  &&  sih == *t_id) {
                        iter = o_iter;
                        break;
                    }
                }
            }
        }
        if (iter != m_SelectedObjects.end()) {
            m_SelectedObjects.erase(iter);
        }

        m_DeselectedObjs = 1;
        if (m_SelectedIndexes.Empty()  ||  m_SelectedObjects.empty()) {
            m_SelectedObjects.clear();
            m_SelectedIndexes.Clear(m_Scope);
        }
    }
}


void CSelectionVisitor::SelectObject(const CObject* obj, bool verified)
{
    CConstRef<CObject> object(obj);
    if (m_SelectedObjects.count(object) != 0) return;

    if (verified) {
        m_SelectedObjects.insert(object);
        m_SelectedIndexes.Add(NULL, *const_cast<CObject*>(obj));
    } else if ( !m_UnknownSelectedIndexes.HasMatches(*obj, *m_Scope) ) {
        // the object may be broadcasted from other view, and it is
        // not verified by any means if it is a seq-id
        m_UnknownSelectedIndexes.Add(NULL, *const_cast<CObject*>(obj));
    }
}


void CSelectionVisitor::SelectSelection(const CSeqGlyph::TConstObjects& glyphs)
{
    ITERATE (CSeqGlyph::TConstObjects, iter, glyphs) {
        const IObjectBasedGlyph* obj_glyph =
            dynamic_cast<const IObjectBasedGlyph*>(iter->GetPointer());
        if (obj_glyph) {
            vector< CConstRef<CObject> > objs;
            obj_glyph->GetObjects(objs);
            NON_CONST_ITERATE (vector< CConstRef<CObject> >, obj_iter, objs) {
                if ( m_SelectedObjects.count(*obj_iter) == 0) {
                    m_SelectedIndexes.Add(NULL, const_cast<CObject&>(**obj_iter));
                    m_SelectedObjects.insert(*obj_iter);
                }
            }
        }
    }
}


void CSelectionVisitor::UpdateSelection(CSeqGlyph* glyph)
{
    SetActionMode(eAct_Select);
    m_TraversedObjs = 0;
    m_TraversedSigs = 0;
    m_SelCDSFeatures.clear();
    m_SelFeatures.clear();
    m_PostUpdateAction = ePUA_None;

    if (x_NeedUpdate()) {
        glyph->Accept(this);
        ITERATE(TSelectedObjects, it, m_SelectedObjects) {
            if (m_UnknownSelectedIndexes.Empty()) {
                break;
            }
            m_UnknownSelectedIndexes.Remove(**it);
        }
        if(m_PostUpdateAction != ePUA_None) {
            NON_CONST_ITERATE(TSelectedGlyphs, iSelFeatures, m_SelFeatures) {
                CRef<CSeqGlyph> pGlyph(iSelFeatures->Lock());

                if(pGlyph.NotNull()) {
                    CFeatGlyph* pFeatGlyph(dynamic_cast<CFeatGlyph*>(pGlyph.GetPointerOrNull()));
                    if(pFeatGlyph){
                        if(m_PostUpdateAction == ePUA_CallLayoutChanged) {
                            pFeatGlyph->LayoutChanged();
                        }
                    }
                }
            }
        }
    }
}


bool CSelectionVisitor::x_DoSelection(CSeqGlyph* glyph)
{
    bool cont = true;

    IObjectBasedGlyph* obj_glyph = dynamic_cast<IObjectBasedGlyph*>(glyph);
    if ( !obj_glyph ) {
        return cont;
    }

    // the optimization may not make sense since there might be
    // two or more objects share the same signature
    if (m_SelectedSignatures.size() == m_TraversedSigs) {
        cont = false;
    }

    // signatures for features
    if (cont  &&  !m_SelectedSignatures.empty()) {
        string f_sig = obj_glyph->GetSignature();
        ITERATE(list<string>, it, m_SelectedSignatures) {
            string sig = *it;
            // Considering that some signatures may not contain data source
            // information, we tolerate partial matching here.
            if (sig.find(f_sig) != string::npos) {
                if(glyph->SetSelected(true)) {
                    m_PostUpdateAction = ePUA_CallLayoutChanged;
                }
                m_SelFeatures.push_back(CWeakRef<CSeqGlyph>(glyph));
                CCdsGlyph* cds = dynamic_cast<CCdsGlyph*>(glyph);
                if (cds) {
                    // keep CDS features for speciality grid rendering
                    m_SelCDSFeatures.push_back(CWeakRef<CSeqGlyph>(glyph));
                }
                ++m_TraversedSigs;
            }
        }
        // we assume selection by m_SelectedSignatures is only used in cgi mode,
        // and in cgi mode, m_SelectedIndexes will never be used.
        return cont;
    }

    // the optimization may not make sense since there might be
    // two or more objects share the same original object
    //if (m_SelectedObjects.size() == m_TraversedObjs  &&
    //    m_DeselectedObjs <= 0) {// do nothing in no selected objects
    //    return cont;
    //}

    bool selected = false;
    vector< CConstRef<CObject> > objs;
    obj_glyph->GetObjects(objs);

    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CObject& object = **it;
        if ( !m_UnknownSelectedIndexes.Empty()  &&
            m_UnknownSelectedIndexes.HasMatches(object, *m_Scope)) {
                m_SelectedObjects.insert(CConstRef<CObject>(&object));
                m_SelectedIndexes.Add(NULL, const_cast<CObject&>(object));
                CObjectIndex::TResults results;
                selected = true;
                ++m_TraversedObjs;
                break;
        }

        if (IsObjectSelected(&object)) {
            selected = true;
            ++m_TraversedObjs;
        }

        // special case for SeqGraphs might be needed
        /*CConstRef<CObject> obj1 = *it;
        ITERATE (TSelectedObjects, it2, *m_SelectedObjects) {
        CConstRef<CObject> obj2 = *it2;
        if (obj1.GetPointer() == obj2.GetPointer()) {
        selected = true;
        break;
        }
        }*/
    }

    if (selected != glyph->IsSelected()) {
        if (glyph->SetSelected(selected)) {
            m_PostUpdateAction = ePUA_CallLayoutChanged;
        }
        if (!selected  &&  m_DeselectedObjs > 0) {
            --m_DeselectedObjs;
        }
    }

    // Save intervals for selected features and pairwise alignments
    // we do matching later, to render hair-lines for intervals
    if (selected  &&  obj_glyph) {
        m_SelFeatures.push_back(CWeakRef<CSeqGlyph>(glyph));

        CCdsGlyph* cds = dynamic_cast<CCdsGlyph*>(obj_glyph);
        if (cds) {
            // keep CDS features for speciality grid rendering
            m_SelCDSFeatures.push_back(CWeakRef<CSeqGlyph>(glyph));
        }
    }
    return true;
}


END_NCBI_SCOPE
