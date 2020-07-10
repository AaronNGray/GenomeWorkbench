/*  $Id: attached_handles.cpp 44371 2019-12-05 22:13:10Z joukovv $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/attached_handles.hpp>

#include <objects/gbproj/ProjectItem.hpp>
#include <gui/objutils/obj_convert.hpp>

#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/entrezgene/Entrezgene_Set.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/biotree/BioTreeContainer.hpp>

#include <objmgr/util/sequence.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static CSeq_entry_Handle s_AttachEntryToScope( CScope& scope, CSeq_entry& entry )
{
    CSeq_entry_Handle seh;
    try {
        seh = scope.AddTopLevelSeqEntry( entry, CScope::kPriority_Default, CScope::eExist_Get );

    } catch( CException& ex ){
//         try {
//             seh = scope.GetSeq_entryHandle(entry);
//             LOG_POST(Warning << "attempted to attach already existing seq-entry");
//         }
//        catch( CException& ){
            LOG_POST( Error << "Failed to attach seq-entry: " << ex.GetMsg() );
//        }
    }

    return seh;
}


static CSeq_annot_Handle s_AttachAnnotToScope( CScope& scope, CSeq_annot& annot )
{
    switch( annot.GetData().Which() ){
    case CSeq_annot::TData::e_Seq_table:
    case CSeq_annot::TData::e_Ftable:
    case CSeq_annot::TData::e_Align:
    case CSeq_annot::TData::e_Graph:
        try{ 
            return scope.AddSeq_annot( annot, CScope::kPriority_Default, CScope::eExist_Get );

        } catch( CException& ex ){
            LOG_POST( Error << "Failed to attach seq-annot: " << ex.GetMsg() );
        }

    default:
        /// bags of IDs or locs ignored
        break;
    }

    return CSeq_annot_Handle();
}


static CRef<CSeq_annot> s_AlignSet2Annot(const CObject& obj)
{
    CRef<CSeq_annot> annot;
    const CSeq_align_set* align_set = dynamic_cast<const CSeq_align_set*>(&obj);
    if (align_set) {
        annot.Reset(new CSeq_annot());
        ITERATE (CSeq_align_set::Tdata, iter, align_set->Get()) {
            CRef<CSeq_align> ref(const_cast<CSeq_align*>(iter->GetPointer()));
            annot->SetData().SetAlign().push_back(ref);
        }
    }
    return annot;
}


static CRef<CSeq_annot> s_Align2Annot(const CObject& obj)
{
    CRef<CSeq_annot> annot;
    const CSeq_align* align = dynamic_cast<const CSeq_align*>(&obj);
    if (align) {
        annot.Reset(new CSeq_annot());
        annot->SetData().SetAlign()
            .push_back(CRef<CSeq_align>(const_cast<CSeq_align*>(align)));
    }
    return annot;
}

#define USE_ANNOT_FOR_WELL_KNOWN_SEQUENCES

void CAttachedHandles::Attach(CScope& scope, CProjectItem& item,
        bool annot_only)
{
    switch (item.GetItem().Which()) {
    case CProjectItem::TItem::e_Entry:
        {{
            int n_bioseq = 0;
            int n_filtered = 0;
            CSeq_entry &se = item.SetItem().SetEntry();
            if (annot_only) {
                for (CTypeIterator<CBioseq> bioseq(Begin(se)); bioseq; ++bioseq) {
                    ++n_bioseq;
                    CBioseq& bs = *bioseq;
                    // Check that there are no local (or general) ids in the
                    // bioseqs id set
                    bool local = false;
                    ITERATE(list<CRef<CSeq_id> >, it, bs.GetId()) {
                        const CSeq_id &id = **it;
                        if (id.IsLocal() || id.IsGeneral()) {
                            local = true;
                            break;
                        }
                    }
                    if (!local) {
                        // Attach annot only
                        if (bs.CanGetAnnot()) {
                            NON_CONST_ITERATE (CBioseq::TAnnot, iter, bs.SetAnnot()) {
                                CSeq_annot_Handle h =
                                    s_AttachAnnotToScope(scope, **iter);
                                if (h) {
                                    m_Seq_annot_Handles.push_back(h);
                                }
                            }
                        }
                    } else {
                        // Whole bioseq, wrap to seq-entry first.
                        CRef<CSeq_entry> se_wrap(new CSeq_entry);
                        se_wrap->SetSeq(bs);
                        CSeq_entry_Handle seh =
                            s_AttachEntryToScope(scope, *se_wrap);
                        if (seh) {
                            m_Seq_entry_Handles.push_back(seh);
                        }
                    }
                }
                // Top-level annot
                if (se.IsSet() && se.GetSet().CanGetAnnot()) {
                    NON_CONST_ITERATE (CBioseq::TAnnot, iter, se.SetSet().SetAnnot()) {
                        CSeq_annot_Handle h =
                            s_AttachAnnotToScope(scope, **iter);
                        if (h) {
                            m_Seq_annot_Handles.push_back(h);
                        }
                    }
                }
            } else {
                /// CSeq_entry already - just add to scope
                CSeq_entry_Handle seh =
                    s_AttachEntryToScope(scope, item.SetItem().SetEntry());
                if (seh) {
                    m_Seq_entry_Handles.push_back(seh);
                }
            }
        }}
        break;

    case CProjectItem::TItem::e_Annot:
        {{
            /// Annotation - call CScope::AddSeq_annot()
            CSeq_annot_Handle h =
                s_AttachAnnotToScope(scope, item.SetItem().SetAnnot());
            if (h) {
                m_Seq_annot_Handles.push_back(h);
            }
        }}
        break;

    case CProjectItem::TItem::e_Submit:
        /// Seq-submit may be either entries or annotations
        /// these are handled as above
        switch (item.GetItem().GetSubmit().GetData().Which()) {
        case CSeq_submit::TData::e_Entrys:
            NON_CONST_ITERATE (CSeq_submit::TData::TEntrys, iter, item.SetItem().SetSubmit().SetData().SetEntrys()) {
                CSeq_entry_Handle h =
                    s_AttachEntryToScope(scope, **iter);
                if (h) {
                    m_Seq_entry_Handles.push_back(h);
                }
            }
            break;

        case CSeq_submit::TData::e_Annots:
            NON_CONST_ITERATE (CSeq_submit::TData::TAnnots, iter, item.SetItem().SetSubmit().SetData().SetAnnots()) {
                CSeq_annot_Handle h =
                    s_AttachAnnotToScope(scope, **iter);
                if (h) {
                    m_Seq_annot_Handles.push_back(h);
                }
            }
            break;

        default:
            break;
        }
        break;

    case CProjectItem::TItem::e_Seq_align:
        /// alignments have a simple annotation representation
        /// and can be converted inexpensively
        {{
            CRef<CSeq_annot> annot = s_Align2Annot(*item.GetObject());
            CSeq_annot_Handle h = s_AttachAnnotToScope(scope, *annot);
            if (h) {
                m_Seq_annot_Handles.push_back(h);
            }
        }}
        break;

    case CProjectItem::TItem::e_Seq_align_set:
        /// alignments have a simple annotation representation
        /// and can be converted inexpensively
        {{
            CRef<CSeq_annot> annot = s_AlignSet2Annot(*item.GetObject());            
            CSeq_annot_Handle h = s_AttachAnnotToScope(scope, *annot);
            if (h) {
                m_Seq_annot_Handles.push_back(h);
            }
        }}
        break;

    case CProjectItem::TItem::e_Other:
        /// other items should be converted to top-level
        /// seq-entries and added to the scope
        {{
            const CObject* obj = item.GetObject();
            {{
                 /// exclude certain types of object
                 const CSerialObject* cso = dynamic_cast<const CSerialObject*>(obj);
                 if (cso) {
                     const CTypeInfo* ti = cso->GetThisTypeInfo();
                     if (ti == CSeq_loc::GetTypeInfo()) {
                         /// nothing to do here - seq-locs are independent
                         /// entities
                         break;
                     }
                     if (ti == CEntrezgene::GetTypeInfo()) {
                         /// nothing to do here - Entrezgene records
                         /// are independent entities
                         break;
                     }
                     if (ti == CEntrezgene_Set::GetTypeInfo()) {
                         /// nothing to do here - Entrezgene records
                         /// are independent entities
                         break;
                     }
                     if (ti == CSeq_feat::GetTypeInfo()) {
                         CSeq_feat& feat = const_cast<CSeq_feat&>
                             (dynamic_cast<const CSeq_feat&>(*obj));

                         CRef<CSeq_annot> annot(new CSeq_annot());
                         annot->SetData().SetFtable().push_back(CRef<CSeq_feat>(&feat));
                         CSeq_annot_Handle h = s_AttachAnnotToScope(scope, annot.GetObject());
                         if (h) {
                             m_Seq_annot_Handles.push_back(h);
                         }
                         break;
                     }
                     if (ti == CGC_Assembly::GetTypeInfo()) {
                         // Attach all alignment annotations to scope
                         CGC_Assembly* assm = const_cast<CGC_Assembly*>(
                             dynamic_cast<const CGC_Assembly*>(cso));
                         CTypeIterator<CSeq_annot> annot_iter(*assm);
                         for (; annot_iter; ++annot_iter) {
                             if (annot_iter->IsAlign()) {
                                 // We only collect seq-aligns with type not set to 'global', and
                                 // alignments will be sorted by assembly unit, JIRA GB-2344
                                 typedef map<string, CSeq_annot::TData::TAlign> TSortedAligns;
                                 TSortedAligns sorted_aligns;
                                 NON_CONST_ITERATE(CSeq_annot::TData::TAlign, aln_iter, annot_iter->SetData().SetAlign()) {
                                     if ((*aln_iter)->GetType() != CSeq_align::eType_global) {
                                         CSeq_id_Handle alt_locus_id =
                                             sequence::GetId((*aln_iter)->GetSeq_id(0), scope, sequence::eGetId_Canonical);
                                         CConstRef< CGC_Sequence > gc_seq = assm->Find (alt_locus_id) ;
                                         if (gc_seq) {
                                             string assm_unit = gc_seq->GetAssemblyUnit()->GetName();
                                             sorted_aligns[assm_unit].push_back(*aln_iter);
                                         }
                                     }
                                 }

                                 if ( !sorted_aligns.empty() ) {
                                     string annot_name = "";
                                     if (annot_iter->IsSetDesc()) {
                                         ITERATE (objects::CAnnot_descr::Tdata, descrIter, annot_iter->GetDesc().Get()) {
                                             if ((*descrIter)->IsTitle()) {
                                                 annot_name = (*descrIter)->GetTitle();
                                                 break;
                                             } else if ((*descrIter)->IsName()) {
                                                 annot_name = (*descrIter)->GetName();
                                             }
                                         }
                                     }
                                     if ( !annot_name.empty() ) {
                                         annot_name += " - ";
                                     }

                                     NON_CONST_ITERATE(TSortedAligns, sorted_annot_iter, sorted_aligns) {
                                         CRef<CSeq_annot> annot(new CSeq_annot());
                                         annot->SetNameDesc(annot_name + sorted_annot_iter->first);
                                         annot->SetData().SetAlign() = sorted_annot_iter->second;
                                         CSeq_annot_Handle h =
                                             s_AttachAnnotToScope(scope, *annot);
                                         if (h) {
                                             m_Seq_annot_Handles.push_back(h);
                                         }
                                     }
                                 }
                             }
                         }
                         break;
                     }

                     if (ti == CBioTreeContainer::GetTypeInfo()) {
                         // nothing to do here
                         break;
                     }

                     if (ti == CProjectItem::GetTypeInfo()) {
                        CProjectItem& sub_item = const_cast<CProjectItem&>(dynamic_cast<const CProjectItem&>(*obj));
                        Attach(scope, sub_item);
                        break;
                     }
                 }
             }}

            if (obj) {
                CObjectConverter::TObjList objs;
                CObjectConverter::Convert(scope, *obj,
                    CSeq_entry::GetTypeInfo(), objs);
                ITERATE (CObjectConverter::TObjList, obj_iter, objs) {
                    CSeq_entry& entry =
                        const_cast<CSeq_entry&>
                        (dynamic_cast<const CSeq_entry&>(obj_iter->GetObject()));
                    CSeq_entry_Handle h =
                        s_AttachEntryToScope(scope, entry);
                    if (h) {
                        m_Seq_entry_Handles.push_back(h);
                    }
                }
            }
        }}
        break;

    case CProjectItem::TItem::e_Id:
        /// ID -> do nothing; IDs should be referrable directly
    case CProjectItem::TItem::e_Obsolete_plugin:
        /// don't do anything special to a plugin message
    default:
        /// default trap: pubmed ID, taxonomy ID
        break;
    }
}

static void s_DetachEntryFromScope(CScope& scope, CSeq_entry_Handle& handle)
{
    if( !handle ){
        return;
    }

    try {
        scope.RemoveTopLevelSeqEntry( const_cast<CTSE_Handle&>(handle.GetTSE_Handle()) );

    } catch( CException& ex ){
        LOG_POST(
            Error << "s_DetachEntryFromScope(): error removing seq-entry: " << ex.GetMsg()
        );
    }
}

static void s_DetachAnnotFromScope(CScope& scope, CSeq_annot_Handle& annot)
{
    if( annot ){
        try {
            annot.GetEditHandle().Remove();

        } catch( CException& ex ){
            LOG_POST(Error 
                << "s_DetachAnnotFromScope(): error removing seq-annot: "
                << ex.GetMsg()
            );
        }
    }
}

void CAttachedHandles::Detach(CScope& scope)
{
    NON_CONST_ITERATE (vector<CSeq_entry_Handle>, iter, m_Seq_entry_Handles) {
        s_DetachEntryFromScope(scope, *iter);
    }
    m_Seq_entry_Handles.clear();

    NON_CONST_ITERATE (vector<CSeq_annot_Handle>, iter, m_Seq_annot_Handles) {
        s_DetachAnnotFromScope(scope, *iter);
    }
    m_Seq_annot_Handles.clear();
}

END_NCBI_SCOPE
