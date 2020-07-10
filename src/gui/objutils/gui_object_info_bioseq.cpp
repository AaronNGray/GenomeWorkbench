/*  $Id: gui_object_info_bioseq.cpp 41372 2018-07-18 20:45:38Z rudnev $
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
#include "gui_object_info_bioseq.hpp"
#include <gui/objutils/label.hpp>
#include <gui/objutils/tooltip.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void initCGuiObjectInfoBioseq()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CBioseq::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoBioseq>());
}

CGuiObjectInfoBioseq* CGuiObjectInfoBioseq::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiObjectInfoBioseq* gui_info = new CGuiObjectInfoBioseq();
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;
    return gui_info;
}

string CGuiObjectInfoBioseq::GetSubtype() const
{
    const CBioseq& bioseq = dynamic_cast<const CBioseq&>(*m_Object);
    if(bioseq.IsNa())
        return "DNA";
    else if(bioseq.IsAa())
        return "Protein";
    return "";
}

string CGuiObjectInfoBioseq::GetIcon() const
{
    const CBioseq& bioseq = dynamic_cast<const CBioseq&>(*m_Object);
    if(bioseq.IsNa())
        return "symbol::sequence_dna";
    else if(bioseq.IsAa())
        return "symbol::sequence_protein";
    return "symbol::sequence";
}


string CGuiObjectInfoBioseq::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Object, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}


void CGuiObjectInfoBioseq::GetToolTip(ITooltipFormatter& tooltip, string& /*t_title*/,
                                      TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    string t_text;
    CLabel::GetLabel(*m_Object, &t_text, CLabel::eDescriptionBrief, m_Scope);
    tooltip.AddRow(t_text);
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
}

END_NCBI_SCOPE
