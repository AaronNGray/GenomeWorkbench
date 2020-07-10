/*  $Id: gui_object_info_seq_id.cpp 41372 2018-07-18 20:45:38Z rudnev $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/tooltip.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/general/Dbtag.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoSeq_id : public CObject, public IGuiObjectInfo
{
public:
    static CGuiObjectInfoSeq_id* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Sequence ID"; }
    virtual string GetSubtype() const;
    virtual string GetLabel() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& /*links*/, bool /*no_ncbi_base*/) const {};
    virtual string GetIcon() const;
    virtual string GetViewCategory() const { return "Sequence"; }

private:
    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;

    enum ESubtype {
        kUnknown,
        kDNA,
        kProtein
    } m_Subtype;
};

void initCGuiObjectInfoSeq_id()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CSeq_id::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoSeq_id>());
}

CGuiObjectInfoSeq_id* CGuiObjectInfoSeq_id::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CRef<CGuiObjectInfoSeq_id> gui_info(new CGuiObjectInfoSeq_id());
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;

    const CSeq_id* seq_id_ptr = dynamic_cast<const CSeq_id*>(object.object.GetPointer());
    if (seq_id_ptr->IsGi()) {
        CSeq_id_Handle seh;

        try {
            seh = sequence::GetId(*seq_id_ptr, *object.scope, sequence::eGetId_Best);
        } NCBI_CATCH("CGuiObjectInfoSeq_id::CreateObject");

        if (seh)
            seq_id_ptr = seh.GetSeqId();
    }

    ESubtype subType = kUnknown;
    CSeq_id::EAccessionInfo info = seq_id_ptr->IdentifyAccession();
    if (info == CSeq_id::eAcc_general) {
        const CSeq_id::TGeneral& gen = seq_id_ptr->GetGeneral();
        if (gen.GetDb() == "TRACE_ASSM") {
            subType = kDNA;
        }
    }

    if (subType == kUnknown) {
        if (info & CSeq_id::fAcc_nuc)
            subType = kDNA;
        else if (info & CSeq_id::fAcc_prot)
            subType = kProtein;
    }

    if (subType == kUnknown) {
        CBioseq_Handle handle;

        try {
            handle = object.scope->GetBioseqHandle(*seq_id_ptr);
        } NCBI_CATCH("CGuiObjectInfoSeq_id::CreateObject");

        if (handle) {
            if (handle.IsProtein())
                subType = kProtein;
            else if (handle.IsNucleotide())
                subType = kDNA;
        }
    }

    gui_info->m_Subtype = subType;

    return gui_info.Release();
}

string CGuiObjectInfoSeq_id::GetSubtype() const
{
    switch(m_Subtype) {
    case kDNA:
        return "DNA";
    case kProtein:
        return "Protein";
    default:
        return "";
    }
}

string CGuiObjectInfoSeq_id::GetIcon() const
{
    switch(m_Subtype) {
    case kDNA:
        return "symbol::sequence_id_dna";
    case kProtein:
        return "symbol::sequence_id_protein";
    default:
        return "symbol::sequence_id";
    }
}

string CGuiObjectInfoSeq_id::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Object, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

void CGuiObjectInfoSeq_id::GetToolTip(ITooltipFormatter& tooltip, string& /*t_title*/, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    string t_text;
    CLabel::GetLabel(*m_Object, &t_text, CLabel::eDescriptionBrief, m_Scope);
    tooltip.AddRow(t_text);
}

END_NCBI_SCOPE
