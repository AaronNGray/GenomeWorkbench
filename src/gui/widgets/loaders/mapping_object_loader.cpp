/*  $Id: mapping_object_loader.cpp 37951 2017-03-07 22:08:17Z rudnev $
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
 */

#include <ncbi_pch.hpp>

#include <serial/iterator.hpp>
#include <gui/widgets/loaders/mapping_object_loader.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <gui/objutils/gencoll_svc.hpp>

#include <sstream>

#include "resolve_id_dlg.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CMappingObjectLoader::x_CreateMapper(const string& assembly)
{
    if (assembly.empty())
        return;

    try {
        // since this is remapping, we clearly need a full assembly here
        CRef<CGC_Assembly> assm = CGencollSvc::GetInstance()->GetGCAssembly(assembly, true, "Gbench");
        if (assm) {
            CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
            CRef<CScope> scope(new CScope(*obj_mgr));
            scope->AddDefaults();
            CIdMapperGCAssembly::EAliasMapping alias =
                assm->IsRefSeq() ? CIdMapperGCAssembly::eRefSeqAcc : CIdMapperGCAssembly::eGenBankAcc;
            m_IdMapper.reset(new CIdMapperGCAssembly(*scope, *assm, alias));
        }
    } NCBI_CATCH("CMappingObjectLoader - Creating Id mapper.");
}

void CMappingObjectLoader::x_UpdateMap(CSeq_annot& annot)
{
    if (m_IdMapper.get() == 0)
        return;

    CTypeIterator< CSeq_id > idit(annot);
    for ( ;  idit;  ++idit ) {
        try {
            CSeq_id_Handle src_idh = CSeq_id_Handle::GetHandle(*idit);
            if (src_idh  &&  m_IdMap.count(src_idh) == 0) {
                CSeq_id_Handle target_idh = m_IdMapper->Map(src_idh);
                if (target_idh) {
                    m_IdMap[src_idh] = target_idh;
                }
            }
        } catch (CException&) {
            /// ingored
        }
    }
}


bool CMappingObjectLoader::x_ShowMappingDlg(IObjectLoader::TObjects& objects)
{
    set<CSeq_id_Handle> genomic_id_set;
    set<CSeq_id_Handle> transcript_id_set;
    set<CSeq_id_Handle> protein_id_set;

    NON_CONST_ITERATE (IObjectLoader::TObjects, it, objects) {
        CSeq_annot* annot = dynamic_cast<CSeq_annot*>(const_cast<CObject*>(it->GetObjectPtr()));
        if (annot == 0)
            continue;
        ///
        /// collect IDs from this
        /// possible locations include:
        ///  - features in feature tables
        ///  - alignments
        ///  - sequences
        CTypeIterator< CSeq_id > idit(*annot);
        for ( ;  idit;  ++idit ) {
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*idit);
            if (idh) {
                genomic_id_set.insert(idh);
            }
        }


        CTypeConstIterator<CSeq_feat> feat_iter(*annot);
        for ( ;  feat_iter;  ++feat_iter) {
            try {
                /// if the product is set, we can grab the appropriate ID
                if (feat_iter->IsSetProduct()) {
                    CSeq_id_Handle idh =
                        CSeq_id_Handle::GetHandle
                        (sequence::GetId(feat_iter->GetProduct(), NULL));
                    switch (feat_iter->GetData().Which()) {
                        case CSeqFeatData::e_Rna:
                            transcript_id_set.insert(idh);
                            break;
                        case CSeqFeatData::e_Cdregion:
                            protein_id_set.insert(idh);
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (CException&) {
            }
        }
    }

    ///
    /// Sun's Workshop compiler doesn't support templated insert() or copy()
    /// so we have to do this manually...
    ///
    vector<CSeq_id_Handle> genomic_ids;
    vector<CSeq_id_Handle> transcript_ids;
    vector<CSeq_id_Handle> protein_ids;
    {{
        ITERATE (set<CSeq_id_Handle>, it, genomic_id_set) {
            if (transcript_id_set.count(*it) == 0  &&
                protein_id_set.count(*it) == 0) {
                genomic_ids.push_back(*it);
            }
        }

        ITERATE (set<CSeq_id_Handle>, it, transcript_id_set) {
            transcript_ids.push_back(*it);
        }

        ITERATE (set<CSeq_id_Handle>, it, protein_id_set) {
            protein_ids.push_back(*it);
        }
    }}

    CResolveIdDlg::TData data;
    if (!genomic_ids.empty())
        data.push_back(CResolveIdDlg::TData::value_type
                       ("Genomic", genomic_ids));
    if (!transcript_ids.empty())
        data.push_back(CResolveIdDlg::TData::value_type
                       ("Transcript", transcript_ids));
    if (!protein_ids.empty())
        data.push_back(CResolveIdDlg::TData::value_type
                       ("Protein", protein_ids));

    if (data.empty())
        return true;

    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    for (TIdMap::iterator it = m_IdMap.begin(); it != m_IdMap.end(); ++it) {
        CSeq_id_Handle idh;
        try {
            CSeq_id_Handle idh = sequence::GetId(it->second, *scope);
            if (idh)
                it->second = idh;
        } catch (const std::exception&) {
        }
    }

    CResolveIdDlg dlg(NULL);
    dlg.SetRegistryPath("Dialogs.CResolveIdDlg");
    dlg.SetData(data, m_IdMap, *scope);

    if (dlg.ShowModal() != wxID_OK)
        return false;

    if (m_IdMap.size()) {
        /// now, for each item in the reply...
        NON_CONST_ITERATE (IObjectLoader::TObjects, it, objects) {
            CSeq_annot* annot = dynamic_cast<CSeq_annot*>(const_cast<CObject*>(it->GetObjectPtr()));

            /// find all seq-ids...
            /// this is done less fine-grained than above on purpose.
            CTypeIterator<CSeq_id> id_iter(*annot);
            for ( ;  id_iter;  ++id_iter) {
                CResolveIdDlg::TIdMap::const_iterator new_id =
                    m_IdMap.find(CSeq_id_Handle::GetHandle(*id_iter));
                if (new_id != m_IdMap.end()) {
                    /// reassign to the new one if the new one exists
                    id_iter->Assign(*new_id->second.GetSeqId());
                }
            }
        }
    }
    return true;
}

END_NCBI_SCOPE
