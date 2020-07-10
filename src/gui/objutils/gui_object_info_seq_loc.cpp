/*  $Id: gui_object_info_seq_loc.cpp 41372 2018-07-18 20:45:38Z rudnev $
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

class CGuiObjectInfoSeq_loc : public CObject, public IGuiObjectInfo
{
public:
    static CGuiObjectInfoSeq_loc* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Location"; }
    virtual string GetSubtype() const { return ""; }
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

void initCGuiObjectInfoSeq_loc()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CSeq_loc::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoSeq_loc>());
}

CGuiObjectInfoSeq_loc* CGuiObjectInfoSeq_loc::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiObjectInfoSeq_loc* gui_info = new CGuiObjectInfoSeq_loc();
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;
    gui_info->m_Subtype = kUnknown;

    const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(object.object.GetPointer());
    const CSeq_id* seqId = seqLoc->GetId();
    if (seqId) {
        try {
            if (seqId->IsGi())
                seqId = sequence::GetId(*seqId, *object.scope, sequence::eGetId_Best).GetSeqId();

            ESubtype subType = kUnknown;
            CSeq_id::EAccessionInfo info = seqId->IdentifyAccession();
            if (info == CSeq_id::eAcc_general) {
                const CSeq_id::TGeneral& gen = seqId->GetGeneral();
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
                CBioseq_Handle handle = object.scope->GetBioseqHandle(*seqId);
                if (handle) {
                    if (handle.IsProtein())
                        subType = kProtein;
                    else if (handle.IsNucleotide())
                        subType = kDNA;
                }
            }

            gui_info->m_Subtype = subType;
        } NCBI_CATCH("CGuiObjectInfoSeq_loc::CreateObject");
    }

    return gui_info;
}

string CGuiObjectInfoSeq_loc::GetIcon() const
{
    switch (m_Subtype) {
    case kDNA:
        return "symbol::sequence_id_dna";
    case kProtein:
        return "symbol::sequence_id_protein";
    default:
        return "symbol::sequence_id";
    }
}

string CGuiObjectInfoSeq_loc::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Object, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

void CGuiObjectInfoSeq_loc::GetToolTip(ITooltipFormatter& tooltip, string& /*t_title*/, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    string t_text;
    CLabel::GetLabel(*m_Object, &t_text, CLabel::eDescriptionBrief, m_Scope);
    tooltip.AddRow(t_text);
}

END_NCBI_SCOPE
