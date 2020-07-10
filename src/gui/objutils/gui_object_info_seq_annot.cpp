/*  $Id: gui_object_info_seq_annot.cpp 41372 2018-07-18 20:45:38Z rudnev $
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

#include <objects/seq/Seq_annot.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoSeq_annot : public CObject, public IGuiObjectInfo
{
public:
    static CGuiObjectInfoSeq_annot* CreateObject(SConstScopedObject& object, ICreateParams* params);

    virtual string GetType() const { return "Annotation"; }
    virtual string GetSubtype() const;
    virtual string GetLabel() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& /*links*/, bool /*no_ncbi_base*/) const {};
    virtual string GetIcon() const;
    virtual string GetViewCategory() const;

private:
    CConstRef<CObject> m_Object;
    mutable CRef<objects::CScope> m_Scope;
};

void initCGuiObjectInfoSeq_annot()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CSeq_annot::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoSeq_annot>());
}

CGuiObjectInfoSeq_annot* CGuiObjectInfoSeq_annot::CreateObject(
    SConstScopedObject& object, ICreateParams* /*params*/)
{
    CGuiObjectInfoSeq_annot* gui_info = new CGuiObjectInfoSeq_annot();
    gui_info->m_Object = object.object;
    gui_info->m_Scope  = object.scope;
    return gui_info;
}

string CGuiObjectInfoSeq_annot::GetSubtype() const
{
    const CSeq_annot& annot = dynamic_cast<const CSeq_annot&>(*m_Object);

    switch(annot.GetData().Which()) {
    case CSeq_annot::TData::e_Align:
        return "Alignments";
    case CSeq_annot::TData::e_Ftable:
        return "Features";
    case CSeq_annot::TData::e_Graph:
        return "Graphs";
    case CSeq_annot::TData::e_Locs:
        return "Locations";
    case CSeq_annot::TData::e_Ids:
        return "Sequence IDs";
    case CSeq_annot::TData::e_Seq_table:
        return "Table";
    default:
        break;
    }
    return "";
}

string CGuiObjectInfoSeq_annot::GetIcon() const
{
    const CSeq_annot& annot = dynamic_cast<const CSeq_annot&>(*m_Object);

    switch(annot.GetData().Which()) {
    case CSeq_annot::TData::e_Align:
        return "symbol::annotation_alignments";
    case CSeq_annot::TData::e_Ftable:
        return "symbol::annotation_features";
    case CSeq_annot::TData::e_Graph:
        return "symbol::annotation_graphs";
    case CSeq_annot::TData::e_Locs:
        return "symbol::locations";
    case CSeq_annot::TData::e_Ids:
        return "symbol::annotation_sequence_ids";
    default:
        break;
    }
    return "symbol::annotation";
}


string CGuiObjectInfoSeq_annot::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Object, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

void CGuiObjectInfoSeq_annot::GetToolTip(ITooltipFormatter& tooltip, string& /*t_title*/, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    string t_text;
    CLabel::GetLabel(*m_Object, &t_text, CLabel::eDescriptionBrief, m_Scope);
    tooltip.AddRow(t_text);
}

string CGuiObjectInfoSeq_annot::GetViewCategory() const
{
    const CSeq_annot& annot = dynamic_cast<const CSeq_annot&>(*m_Object);

    switch(annot.GetData().Which()) {
    case CSeq_annot::TData::e_Align:
        return "Alignment";
    case CSeq_annot::TData::e_Ftable:
        return "Features";
    case CSeq_annot::TData::e_Graph:
        return "Generic";
    case CSeq_annot::TData::e_Locs:
        return "Sequence";
    case CSeq_annot::TData::e_Ids:
        return "Sequence";
    case CSeq_annot::TData::e_Seq_table:
        return "Generic";
    default:
        break;
    }
    return "Generic";
}

END_NCBI_SCOPE
