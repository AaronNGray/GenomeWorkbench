/*  $Id: gui_object_info_component.cpp 44739 2020-03-04 20:37:02Z evgeniev $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/gui_object_info.hpp>
#include <gui/objutils/gui_object_info_component.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/link_utils.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqfeat/Org_ref.hpp>
#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CGuiObjectInfoComponent* CGuiObjectInfoComponent::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(object.object.GetPointer());
    if (nullptr == seqLoc) {
        return nullptr;
    }
    const CSeq_id* seqId = seqLoc->GetId();
    if (nullptr == seqId) {
        return nullptr;
    }

    CGuiObjectInfoComponent* gui_info = new CGuiObjectInfoComponent();
    gui_info->m_Object = object.object;
    gui_info->m_Scope = object.scope;
    gui_info->m_Id.Reset(seqId);

    return gui_info;
}

string CGuiObjectInfoComponent::GetIcon() const
{
    return "symbol::sequence_id";
}

string CGuiObjectInfoComponent::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Id, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

void CGuiObjectInfoComponent::GetToolTip(ITooltipFormatter& tt, string& /*t_title*/, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }

    string label;
    CLabel::GetLabel(*m_Id, &label, CLabel::eContent, m_Scope);
    CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*m_Id);
    if (!bsh) {
        tt.AddRow(label);
    }
    else {
        tt.AddRow(label + ':', sequence::CDeflineGenerator().GenerateDefline(bsh));

        try {
            const COrg_ref& org_ref = sequence::GetOrg_ref(bsh);
            label.clear();
            org_ref.GetLabel(&label);
            tt.AddRow("Organism:", label);
        }
        catch (const CException&) {
        }

        CConstRef<CBioSource> bs;
        CSeqdesc_CI desc_it(bsh, CSeqdesc::e_Source);
        if (desc_it) {
            bs.Reset(&desc_it->GetSource());
        }
        if (bs  && bs->IsSetSubtype()) {
            tt.AddSectionRow("Subtype");
            ITERATE(CBioSource::TSubtype, iter, bs->GetSubtype()) {
                const CSubSource& sub = **iter;

                if (!sub.IsSetSubtype() || !sub.IsSetName())
                    continue;

                string tag = CSubSource::GetSubtypeName(sub.GetSubtype());
                if (!tag.empty()) {
                    replace(tag.begin(), tag.end(), '_', '-');
                    tag[0] = toupper(tag[0]);
                    tag += ':';
                }
                string value = sub.GetName();
                if (sub.IsSetAttrib()) {
                    value += " (";
                    value += sub.GetAttrib();
                    value += ")";
                }
                tt.AddRow(tag, value);
            }
        }
    }
}

void CGuiObjectInfoComponent::GetLinks(ITooltipFormatter& links, bool /*no_ncbi_base*/) const
{
    CSeq_id_Handle shdl = sequence::GetId(*m_Id, *m_Scope, sequence::eGetId_Best);
    if (!shdl)
        return;
    CLinkUtils::TLinksMap links_map;
    CLinkUtils::AddSequenceLinks(shdl, "", *m_Scope, links_map, 0, 0, false);

    if (links_map.empty())
        return;

    links.AddLinksTitle("Links & Tools");

    for (auto it_type = links_map.begin(); it_type != links_map.end(); ++it_type) {
        CLinkUtils::TLinks& links_by_name = it_type->second;

        for (auto it_name = links_by_name.begin(); it_name != links_by_name.end(); ++it_name) {
            string tmp_links;

            for (size_t i = 0; i < it_name->second.size(); ++i) {
                // This gives a separate row to each link:
                //tmp_links += CSeqUtils::CreateLinkRow((*iter).first, (*uiter).first, (*uiter).second);
                if (i > 0) tmp_links += ",&nbsp;";
                tmp_links += links.CreateLink(it_name->second[i].first, it_name->second[i].second);
            }

            links.AddLinkRow(it_name->first + ':', tmp_links, 250);
        }
    }
}

END_NCBI_SCOPE
