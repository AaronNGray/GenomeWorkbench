/*  $Id: gui_object_info_other_feats.cpp 41372 2018-07-18 20:45:38Z rudnev $
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
*     Gui Object Info classes.
*
*/

#include <objects/seqfeat/Imp_feat.hpp>
#include <gui/objutils/tooltip.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/// Import feature
class CGuiObjectInfoImpFeature : public CGuiObjectInfoSeq_feat
{
public:
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
};

void CGuiObjectInfoImpFeature::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p, bool* isGeneratedBySvc) const
{
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    string type_name;
    feature::GetLabel(*m_Feat, &type_name, feature::fFGL_Content, m_Scope.GetPointer());
    tooltip.AddRow(GetSubtype() + ':', type_name);

    if (m_Feat->GetData().GetImp().IsSetDescr()) {
        tooltip.AddRow("Title:", m_Feat->GetData().GetImp().GetDescr());
    }

    if (m_Feat->IsSetComment()  &&  m_Feat->GetComment() != t_title) {
        tooltip.AddRow("Comment:", m_Feat->GetComment());
    }

    x_AddLocationRows(tooltip);
    x_AddPositionRows(tooltip, at_p);
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
}


/// region feature
class CGuiObjectInfoRegionFeature : public CGuiObjectInfoSeq_feat
{
public:
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
};

void CGuiObjectInfoRegionFeature::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p, bool* isGeneratedBySvc) const
{
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    string type_name;
    feature::GetLabel(*m_Feat, &type_name, feature::fFGL_Content, m_Scope.GetPointer());
    tooltip.AddRow(GetSubtype() + ':', type_name);

    if (m_Feat->IsSetComment()  &&  m_Feat->GetComment() != t_title) {
        tooltip.AddRow("Comment:", m_Feat->GetComment());
    }

    if (m_Feat->IsSetQual()) {
        ITERATE(CSeq_feat::TQual, it, m_Feat->GetQual()) {
            tooltip.AddRow((*it)->GetQual() + ':', (*it)->GetVal());
        }
    }
    
    x_AddLocationRows(tooltip);
    x_AddPositionRows(tooltip, at_p);
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
}


/// seq feature
class CGuiObjectInfoSeqFeature : public CGuiObjectInfoSeq_feat
{
public:
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
};

void CGuiObjectInfoSeqFeature::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    
    tooltip.AddRow(GetSubtype() + ':', t_title);

    if (m_Feat->IsSetComment()  &&  m_Feat->GetComment() != t_title) {
        tooltip.AddRow("Comment:", m_Feat->GetComment());
    }

    if (m_Feat->IsSetQual()) {
        ITERATE(CSeq_feat::TQual, it, m_Feat->GetQual()) {
            tooltip.AddRow((*it)->GetQual() + ':', (*it)->GetVal());
        }
    }
    
    x_AddLocationRows(tooltip);

    if (m_Location) {
        size_t intervals = 0;
        switch (m_Location->Which())
        {
        case CSeq_loc::e_Int:
            intervals = 1;
            break;

        case CSeq_loc::e_Packed_int:
            intervals = m_Location->GetPacked_int().Get().size();
            break;

        case CSeq_loc::e_Packed_pnt:
            intervals = m_Location->GetPacked_pnt().GetPoints().size();
            break;

        case CSeq_loc::e_Mix:
            /// FIXME: this may not always be correct -
            /// a mix may be a mix of complex intervals
            intervals = m_Location->GetMix().Get().size();
            break;
        default:
            break;
        }
        if (intervals > 0) {
            tooltip.AddRow("Intervals:", NStr::SizetToString(intervals));
        }
    }

    x_AddPositionRows(tooltip, at_p);
}


/// site feature
class CGuiObjectInfoSiteFeature : public CGuiObjectInfoSeq_feat
{
public:
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
};


void CGuiObjectInfoSiteFeature::GetToolTip(ITooltipFormatter& tooltip, string& t_title,TSeqPos at_p, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    string type_name;
    feature::GetLabel(*m_Feat, &type_name, feature::fFGL_Content, m_Scope.GetPointer());
    tooltip.AddRow(GetSubtype() + ':', type_name);

    if (m_Feat->IsSetComment()) {
        tooltip.AddRow("Title:", m_Feat->GetComment());
    }

    x_AddLocationRows(tooltip);
    x_AddPositionRows(tooltip, at_p);
}


END_NCBI_SCOPE

