/*  $Id: project_converters.cpp 39528 2017-10-05 15:27:37Z katargir $
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
#include <corelib/ncbiobj.hpp>

#include <objects/gbproj/ProjectItem.hpp>
#include <objects/gbproj/ProjectFolder.hpp>
#include <gui/objects/GBProject.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <gui/objutils/obj_convert.hpp>

#include <serial/iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static CRef<CSeq_annot> s_Align2Annot(const CSeq_align& align)
{
    CRef<CSeq_annot> annot(new CSeq_annot());
    annot->SetData().SetAlign()
        .push_back(CRef<CSeq_align>(const_cast<CSeq_align*>(&align)));
    return annot;
}

static CRef<CSeq_annot> s_AlignSet2Annot(const CSeq_align_set& align_set)
{
    CRef<CSeq_annot> annot(new CSeq_annot());
    ITERATE (CSeq_align_set::Tdata, iter, align_set.Get()) {
        CRef<CSeq_align> ref
            (const_cast<CSeq_align*>(iter->GetPointer()));
        annot->SetData().SetAlign().push_back(ref);
    }
    return annot;
}

static void Convert_Seq_annot_ProjectItem(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CRelation::TFlags flags,
                                          ICanceled*);

/// Seq-align --> ProjectItem
static void Convert_Seq_align_ProjectItem(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CBasicRelation::TFlags flags,
                                          ICanceled*)
{
    const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
    if (align) {
        Convert_Seq_annot_ProjectItem(scope, *s_Align2Annot(*align), objs, flags, NULL);
    }
}

/// Seq-align-set --> ProjectItem
static void Convert_Seq_align_set_ProjectItem(
                                              CScope& scope,
                                              const CObject& obj,
                                              CBasicRelation::TObjects& related,
                                              CBasicRelation::TFlags flags,
                                              ICanceled*)
{
    const CSeq_align_set* align =
        dynamic_cast<const CSeq_align_set*>(&obj);
    if (align) {
        Convert_Seq_annot_ProjectItem
            (scope, *s_AlignSet2Annot(*align), related, flags, NULL);
    }
}

/// Seq-annot --> ProjectItem
static void Convert_Seq_annot_ProjectItem(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CBasicRelation::TFlags flags,
                                          ICanceled*)
{
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&obj);
    if (annot) {
        CRef<CProjectItem> item(new CProjectItem());
        item->SetItem().SetAnnot(const_cast<CSeq_annot&>(*annot));
        objs.push_back(CRelation::SObject(*item));
    }
}

/// Seq-entry --> ProjectItem
static void Convert_Seq_entry_ProjectItem(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CBasicRelation::TFlags flags,
                                          ICanceled*)
{
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*> (&obj);
    if (entry) {
        CRef<CProjectItem> item(new CProjectItem());
        item->SetItem().SetEntry(const_cast<CSeq_entry&>(*entry));

        objs.push_back(CRelation::SObject(*item));
    }
}

/// Seq-id --> ProjectItem
static void Convert_Seq_id_ProjectItem(
                                       CScope& scope,
                                       const CObject& obj,
                                       CBasicRelation::TObjects& objs,
                                       CBasicRelation::TFlags flags,
                                       ICanceled*)
{
    const CSeq_id* id = dynamic_cast<const CSeq_id*>(&obj);
    if (id) {
        CRef<CProjectItem> item(new CProjectItem());
        item->SetItem().SetId(const_cast<CSeq_id&>(*id));
        objs.push_back(CRelation::SObject(*item));
    }
}

/// Seq-submit --> ProjectItem
static void Convert_SeqSubmit_ProjectItem(
                                          CScope& scope,
                                          const CObject& obj,
                                          CBasicRelation::TObjects& objs,
                                          CBasicRelation::TFlags flags,
                                          ICanceled*)
{
    const CSeq_submit* submit = dynamic_cast<const CSeq_submit*>(&obj);
    if (submit) {
        CRef<CProjectItem> item(new CProjectItem());
        item->SetItem().SetSubmit(const_cast<CSeq_submit&>(*submit));
        objs.push_back(CRelation::SObject(*item));
    }
}

/// Container objects:
/// CDocument
/// CGBProjectHandle
/// CProjectItem

static void Convert_GBProject_Object(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags flags,
                                     ICanceled*)
{
    const CGBProjectHandle* proj = dynamic_cast<const CGBProjectHandle*> (&obj);
    if (proj) {
        const CGBProjectHandle::TData& data = proj->GetData();
        CTypeConstIterator<CProjectItem> iter(data);
        for ( ;  iter;  ++iter) {
            objs.push_back(CRelation::SObject(*iter));
        }
    }
}

static void Convert_ProjectItem_Object(
                                       CScope& scope,
                                       const CObject& obj,
                                       CBasicRelation::TObjects& objs,
                                       CBasicRelation::TFlags flags,
                                       ICanceled*)
{
    const CProjectItem* item = dynamic_cast<const CProjectItem*> (&obj);
    if (item  &&  item->GetObject()) {
        objs.push_back(CRelation::SObject(*item->GetObject()));
    }
}

static void Convert_Container_Object(
                                     CScope& scope,
                                     const CObject& obj,
                                     CBasicRelation::TObjects& objs,
                                     CBasicRelation::TFlags flags,
                                     ICanceled*)
{
    const CProjectItem* item = dynamic_cast<const CProjectItem*> (&obj);
    if (item) {
        Convert_ProjectItem_Object(scope, obj, objs, flags, NULL);
        return;
    }
    const CGBProjectHandle* proj = dynamic_cast<const CGBProjectHandle*> (&obj);
    if (proj) {
        const CGBProjectHandle::TData& data = proj->GetData();
        CTypeConstIterator<CProjectItem> iter(data);
        for ( ;  iter;  ++iter)
            Convert_ProjectItem_Object(scope, *iter, objs, flags, NULL);
        return;
    }
}




struct SConverterEntry
{
    const char*      m_Name;
    CBasicRelation::TFNConvert m_pFnCvt;
    const CTypeInfo* m_FromType;
    const CTypeInfo* m_ToType;
    const CBasicRelation::SPropPair* m_Properties;
};

static SConverterEntry m_ConverterEntries[] = {
    // Seq-align
    { "Seq-align --> ProjectItem", Convert_Seq_align_ProjectItem,
        CSeq_align::GetTypeInfo(), CProjectItem::GetTypeInfo() },
    // Seq-align-set
    { "Seq-align-set --> ProjectItem", Convert_Seq_align_set_ProjectItem,
        CSeq_align_set::GetTypeInfo(), CProjectItem::GetTypeInfo() },
    // Seq-annot
    { "Seq-annot --> ProjectItem", Convert_Seq_annot_ProjectItem,
        CSeq_annot::GetTypeInfo(), CProjectItem::GetTypeInfo() },
    // Seq-entry
    { "Seq-entry --> ProjectItem", Convert_Seq_entry_ProjectItem,
        CSeq_entry::GetTypeInfo(), CProjectItem::GetTypeInfo() },
    // Seq-id
    { "Seq-id --> ProjectItem", Convert_Seq_id_ProjectItem,
        CSeq_id::GetTypeInfo(), CProjectItem::GetTypeInfo() },
    // Seq-submit
    { "Seq-submit --> ProjectItem", Convert_SeqSubmit_ProjectItem,
        CSeq_submit::GetTypeInfo(), CProjectItem::GetTypeInfo() }
};

void init_project_converters()
{
    for (size_t i = 0;
         i < sizeof(m_ConverterEntries)/sizeof(m_ConverterEntries[0]); ++i) {
        CObjectConverter::Register
            (new CBasicRelation(m_ConverterEntries[i].m_Name,
                                m_ConverterEntries[i].m_FromType->GetName(),
                                m_ConverterEntries[i].m_ToType->GetName(),
                                m_ConverterEntries[i].m_pFnCvt,
                                m_ConverterEntries[i].m_Properties));
    }

    CObjectConverter::Register
        ( new CBasicRelation("CGBProjectHandle --> Object",
                             CGBProjectHandle::GetTypeInfo()->GetName(),
                             "Object", Convert_GBProject_Object));

    CObjectConverter::Register
        (new CBasicRelation("CProjectItem --> Object",
                            CProjectItem::GetTypeInfo()->GetName(),
                            "Object",
                            Convert_ProjectItem_Object));

    CObjectConverter::Register
        (new CBasicRelation("Any Container --> Object",
                            "Any Container", "Object",
                            Convert_Container_Object));
}

END_NCBI_SCOPE
