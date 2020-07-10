/*  $Id: gui_object_info_bioseq_set.cpp 41372 2018-07-18 20:45:38Z rudnev $
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

#include <objects/seqset/Bioseq_set.hpp>
#include <objmgr/align_ci.hpp>

#include "gui_object_info_bioseq_set.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void initCGuiObjectInfoBioseq_set()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CBioseq_set::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoBioseq_set>());
}

CGuiObjectInfoBioseq_set* CGuiObjectInfoBioseq_set::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiObjectInfoBioseq_set* gui_info = new CGuiObjectInfoBioseq_set();
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;
    return gui_info;
}

static bool s_HasAlignment(const CBioseq_set& bst, CScope* scope)
{
    if(scope)   {
        CBioseq_set_Handle bsh = scope->GetBioseq_setHandle(bst);
        if(bsh) {
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

string CGuiObjectInfoBioseq_set::GetSubtype() const
{
    const CBioseq_set& bioseq_set = dynamic_cast<const CBioseq_set&>(*m_Object);
    bool aln;

    switch(bioseq_set.GetClass()) {
    case CBioseq_set::eClass_nuc_prot:
        return "DNA + Protein Translation (Nuc-Prot)";
    case CBioseq_set::eClass_segset:
        return "Segmented Sequence";
    case CBioseq_set::eClass_gen_prod_set:
        return "Genomic Products";
    case CBioseq_set::eClass_pop_set:
        aln = s_HasAlignment(bioseq_set, m_Scope);
        return aln ? "Population Study" : "Population Study (Aligned)";
    case CBioseq_set::eClass_phy_set:
        aln = s_HasAlignment(bioseq_set, m_Scope);
        return aln ? "Phylogenetic Study" : "Phylogenetic Study (Aligned)";
    case CBioseq_set::eClass_eco_set:
        aln = s_HasAlignment(bioseq_set, m_Scope);
        return aln ? "Ecological Study" : "Ecological Study (Aligned)";
    case CBioseq_set::eClass_mut_set:
        aln = s_HasAlignment(bioseq_set, m_Scope);
        return aln ? "Mutation Study" : "Mutation Study (Aligned)";
    default:
        break;
    }

    return "";
}

string CGuiObjectInfoBioseq_set::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Object, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

void CGuiObjectInfoBioseq_set::GetToolTip(ITooltipFormatter& tooltip, string& /*t_title*/, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    string t_text;
    CLabel::GetLabel(*m_Object, &t_text, CLabel::eDescriptionBrief, m_Scope);
    tooltip.AddRow(t_text);
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
}

END_NCBI_SCOPE
