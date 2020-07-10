/*  $Id: table_data_gcassembly.cpp 41352 2018-07-12 18:45:00Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data_base.hpp>
#include <gui/objutils/label.hpp>

#include <objects/genomecoll/GC_Replicon.hpp>

#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


class CTableDataGCAssembly : public CObject, public CTableDataBase
{
public:
    static CTableDataGCAssembly* CreateObject(SConstScopedObject& object, ICreateParams* params);

    enum EColType {
        eSeq_Id = 0,
        eRefSeq_Acc,
        eGenBank_Acc,
        eChromosome,
        eRole,
        eStatus,
        eParent,
        eUnit,
        ePatchType,
        eMaxColNum
    };

    virtual void       LoadData() {}
    virtual ColumnType GetColumnType(size_t col) const;
    virtual string     GetColumnLabel(size_t col) const;

    virtual size_t GetRowsCount() const;
    virtual size_t GetColsCount() const;

    virtual void GetStringValue(size_t row, size_t col, string& value) const;
    virtual long   GetIntValue(size_t row, size_t col) const;
    virtual double GetRealValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetObjectValue(size_t row, size_t col) const;
    virtual SConstScopedObject GetRowObject(size_t row) const;

private:
    void Init();

    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
    vector< CConstRef<CGC_Sequence> >  m_Seqs;
};


static ITableData::ColumnType s_ColTypes[CTableDataGCAssembly::eMaxColNum] = {
    ITableData::kObject,// sequence id
    ITableData::kObject,// refseq accession
    ITableData::kObject,// genbank accession
    ITableData::kString,// chromosome
    ITableData::kString,// sequence roles (optional, can be more than one role)
    ITableData::kString,// placed/unplaced status
    ITableData::kObject,// parent
    ITableData::kString,// assembly unit
    ITableData::kString // patch type
};


static const string s_ColNames[CTableDataGCAssembly::eMaxColNum] = {
    "Sequence ID",
    "RefSeq Accession",
    "GenBank Accession",
    "Chromosome",
    "Role",
    "Role qualifier",
    "Parent",
    "Assembly unit",
    "Patch type"
};


void initCTableDataGCAssembly()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(ITableData).name(),
            CGC_Assembly::GetTypeInfo(),
            new CObjectInterfaceFactory<CTableDataGCAssembly>());
}


CTableDataGCAssembly* CTableDataGCAssembly::CreateObject(SConstScopedObject& object, ICreateParams*)
{
    CTableDataGCAssembly* table_data = new CTableDataGCAssembly();
    table_data->m_Object = object.object;
    table_data->m_Scope  = object.scope;
    table_data->Init();
    return table_data;
}


void CTableDataGCAssembly::Init()
{
    const CGC_Assembly& assm = dynamic_cast<const CGC_Assembly&>(*m_Object);
    CGC_Assembly& nonconst_assm = const_cast<CGC_Assembly&>(assm);
    nonconst_assm.CreateHierarchy();

    CGC_Assembly::TSequenceList chromosomes;
    assm.GetMolecules(chromosomes, CGC_Assembly::eChromosome);
    //CGC_Assembly::TSequenceList scaffold_seqs;
    //assm.GetMolecules(scaffold_seqs, CGC_Assembly::eScaffold);
    CGC_Assembly::TSequenceList top_level_seqs;
    assm.GetMolecules(top_level_seqs, CGC_Assembly::eTopLevel);
    set< CConstRef<CGC_Sequence> > unique_seqs;

    ITERATE(CGC_Assembly::TSequenceList, it, chromosomes) {
        if (unique_seqs.insert(*it).second) {
            m_Seqs.push_back(*it);
        }
    }

    //ITERATE(CGC_Assembly::TSequenceList, it, scaffold_seqs) {
    //    if (unique_seqs.insert(*it).second) {
    //        m_Seqs.push_back(*it);
    //    }
    //}

    ITERATE(CGC_Assembly::TSequenceList, it, top_level_seqs) {
        if (unique_seqs.insert(*it).second) {
            m_Seqs.push_back(*it);
        }
    }
}


ITableData::ColumnType CTableDataGCAssembly::GetColumnType(size_t col) const
{
    if (col < CTableDataGCAssembly::eMaxColNum)
        return s_ColTypes[col];

    return ITableData::kNone;
}


string CTableDataGCAssembly::GetColumnLabel(size_t col) const
{
    if (col < CTableDataGCAssembly::eMaxColNum)
        return s_ColNames[col];

    return kEmptyStr;
}


size_t CTableDataGCAssembly::GetRowsCount() const
{
    return m_Seqs.size();
}


size_t CTableDataGCAssembly::GetColsCount() const
{
    return CTableDataGCAssembly::eMaxColNum;
}

void CTableDataGCAssembly::GetStringValue(size_t row, size_t col, string& value) const
{
    value.resize(0);
    if (row >= m_Seqs.size()) return;

    const CGC_Sequence& gc_seq = *m_Seqs[row];
    EColType type = (EColType)(col);
    switch (type) {
        case eChromosome:
            {{
                CConstRef<CGC_Replicon> replicon = gc_seq.GetReplicon();
                if (replicon  &&  replicon->CanGetName()) {
                    value = replicon->GetName();
                }
            }}
            break;
        case eStatus:
            {{
                CGC_TaggedSequences::TState state = gc_seq.GetParentRelation();
                switch (state) {
                    case CGC_TaggedSequences::eState_placed:
                        value = "placed";
                        break;
                    case CGC_TaggedSequences::eState_unlocalized:
                        value = "unlocalized";
                        break;
                    case CGC_TaggedSequences::eState_unplaced:
                        value = "unplaced";
                        break;
                    case CGC_TaggedSequences::eState_aligned:
                        value = "aligned";
                        break;
                    case CGC_TaggedSequences::eState_bits:
                        value = "bits";
                        break;
                    default:
                        break;
                }
            }}
            break;
        case eRole:
            if (gc_seq.CanGetRoles()) {
                ITERATE(CGC_Sequence::TRoles, r_iter, gc_seq.GetRoles()) {
                    string role;
                    switch(*r_iter) {
                        case eGC_SequenceRole_chromosome:
                            role = "chromosome";
                            break;
                        case eGC_SequenceRole_scaffold:
                            role = "scaffold";
                            break;
                        case eGC_SequenceRole_component:
                            role = "component";
                            break;
                        //case eGC_SequenceRole_top_level:
                        //    role = "top level";
                        //    break;
                        case eGC_SequenceRole_pseudo_scaffold:
                            role = "pseudo scaffold";
                            break;
                        case eGC_SequenceRole_submitter_pseudo_scaffold:
                            role = "submitter pseudo scaffold";
                            break;
                        default:
                            break;
                    }

                    if ( !role.empty() ) {
                        if (!value.empty() ) {
                            value += ", ";
                        }
                        value += role;
                    }

                }
            }
            break;
        case eSeq_Id:
            {{
                const CSeq_id& seq_id = gc_seq.GetSeq_id();
                seq_id.GetLabel(&value, CSeq_id::eContent);
            }}
            break;
        case eGenBank_Acc:
            {{
                CConstRef<CSeq_id> seq_id =
                    gc_seq.GetSynonymSeq_id(CGC_TypedSeqId::e_Genbank,
                    CGC_SeqIdAlias::e_Public);
                if (seq_id) {
                    seq_id->GetLabel(&value, CSeq_id::eContent);
                }
            }}
            break;
        case eRefSeq_Acc:
            {{
                CConstRef<CSeq_id> seq_id =
                    gc_seq.GetSynonymSeq_id(CGC_TypedSeqId::e_Refseq,
                    CGC_SeqIdAlias::e_Public);
                if (seq_id) {
                    seq_id->GetLabel(&value, CSeq_id::eContent);
                }
            }}
            break;
        case eParent:
            {{
                CConstRef<CGC_Sequence> parent = gc_seq.GetParent();
                if (parent) {
                    CConstRef<CSeq_id> seq_id(&parent->GetSeq_id());
                    if (seq_id->IsLocal()) {
                        // This must be a chromosome number. We want to use the real seq-id.
                        // Check the first 30 sequences if anyone matches this
                        CConstRef<CSeq_id> p_synonym =
                            parent->GetSynonymSeq_id(CGC_TypedSeqId::e_Private, CGC_SeqIdAlias::e_None);
                        for (size_t i = 0; i < 30  &&  i < m_Seqs.size(); ++i) {
                            CConstRef<CSeq_id> curr_synonym =
                                m_Seqs[i]->GetSynonymSeq_id(CGC_TypedSeqId::e_Private, CGC_SeqIdAlias::e_None);
                            if (curr_synonym->Match(*p_synonym)) {
                                seq_id.Reset(&m_Seqs[i]->GetSeq_id());
                                break;
                            }
                        }
                    }

                    CSeq_id_Handle sih = sequence::GetId(*seq_id, *m_Scope, sequence::eGetId_Best);
                    if (sih) {
                        seq_id = sih.GetSeqId();
                    }

                    seq_id->GetLabel(&value, CSeq_id::eContent);
                }
            }}
            break;
        case eUnit:
            {{
                CConstRef<CGC_AssemblyUnit> unit = gc_seq.GetAssemblyUnit();
                if (unit) {
                    value = unit->GetAccession();
                }
            }}
            break;
        case ePatchType:
            {{
                if (gc_seq.IsSetPatch_type()) {
                    value = gc_seq.GetTypeInfo_enum_EPatch_type()->FindName(gc_seq.GetPatch_type(), true);
                }
            }}
            break;
        default:
            break;
    }
}


long CTableDataGCAssembly::GetIntValue(size_t /*row*/, size_t /*col*/) const
{
    long value = 0;
    return value;
}


double CTableDataGCAssembly::GetRealValue(size_t /*row*/, size_t /*col*/) const
{
    double value = 0.0;
    return value;
}


SConstScopedObject CTableDataGCAssembly::GetObjectValue(size_t row, size_t col) const
{
    SConstScopedObject value;
    if (row >= m_Seqs.size()) return value;

    const CGC_Sequence& gc_seq = *m_Seqs[row];
    CConstRef<CSeq_id> seq_id;
    EColType type = (EColType)(col);

    switch (type) {
        case eSeq_Id:
            // if either GenBank_Acc or RefSeq_Acc is present, seq-id is just
            // a synonym of either of them. To avoid duplication, we don't report
            // this id.
            if ( !gc_seq.GetSynonymSeq_id(CGC_TypedSeqId::e_Genbank, CGC_SeqIdAlias::e_Public)  &&
                !gc_seq.GetSynonymSeq_id(CGC_TypedSeqId::e_Refseq, CGC_SeqIdAlias::e_Public) ) {
                seq_id.Reset(&gc_seq.GetSeq_id());
            }
            break;
        case eGenBank_Acc:
            seq_id = gc_seq.GetSynonymSeq_id(CGC_TypedSeqId::e_Genbank,
                CGC_SeqIdAlias::e_Public);
            break;
        case eRefSeq_Acc:
            seq_id = gc_seq.GetSynonymSeq_id(CGC_TypedSeqId::e_Refseq,
                CGC_SeqIdAlias::e_Public);
            break;
        case eParent:
            {{
                CConstRef<CGC_Sequence> parent = gc_seq.GetParent();
                if (parent) {
                    seq_id.Reset(&parent->GetSeq_id());
                    if (seq_id->IsLocal()) {
                        // This must be a chromosome number. We want to use the real seq-id
                        // Check the first 30 sequences if anyone matches this
                        CConstRef<CSeq_id> p_synonym =
                            parent->GetSynonymSeq_id(CGC_TypedSeqId::e_Private, CGC_SeqIdAlias::e_None);
                        for (size_t i = 0; i < 30  &&  i < m_Seqs.size(); ++i) {
                            CConstRef<CSeq_id> curr_synonym =
                                m_Seqs[i]->GetSynonymSeq_id(CGC_TypedSeqId::e_Private, CGC_SeqIdAlias::e_None);
                            if (curr_synonym->Match(*p_synonym)) {
                                seq_id.Reset(&m_Seqs[i]->GetSeq_id());
                                break;
                            }
                        }
                    }
                }
            }}
            break;
        default:
            break;
    }

    if (seq_id) {
        value.object = seq_id;
        value.scope = m_Scope;
    }

    return value;
}


SConstScopedObject CTableDataGCAssembly::GetRowObject(size_t row) const
{
    SConstScopedObject value;
    /// no use gc-sequence for now, disable it
    //if (row >= m_Seqs.size()) return value;
    //value.object = m_Seqs[row];
    //value.scope = m_Scope;
    return value;
}

END_NCBI_SCOPE

