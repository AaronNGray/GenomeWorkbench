/*  $Id: gui_object_info_primer_feat.cpp 41372 2018-07-18 20:45:38Z rudnev $
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
*     Gui Object Info class specifically for BLAST primer features stored
*     as clone features.
*
*/

#include <gui/objutils/tooltip.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoPrimer : public CGuiObjectInfoSeq_feat
{
public:
    virtual string GetSubtype() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
};


string CGuiObjectInfoPrimer::GetSubtype() const
{
    return "Primer";
}


void CGuiObjectInfoPrimer::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    // Brief description goes first
    // Ad-hoc processing for CloneRefs interpreted as primer pairs (must have qual type=primer)
    string forward, reverse, val;
    if (m_Feat->IsSetExts()) {
        const CSeq_feat::TExts& exts = m_Feat->GetExts();
        ITERATE (CSeq_feat::TExts, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "link") {
                CConstRef<CUser_field> uf = (*iter)->GetFieldRef("href");
                if (uf) {
                    val = uf->GetData().GetStr();
                }
            }
        }
        if ( !val.empty() ) {
            //t_text += " <a class=\"sv-bintrack\" href=\"" + val + "\">Details</a>";
            tooltip.AddRow(tooltip.CreateLink("Details", val));
        }
    }

    // Location should contain two ranges for forward and reverse primers
    // Use the original location in case m_Location has been modified due to
    // multiple seq-ids even though that shouldn't happen
    const CSeq_loc& p_loc = m_Feat->GetLocation();
    if (p_loc.IsMix()) {
        const CSeq_loc_mix::Tdata& locs = p_loc.GetMix().Get();
        int count = 0;
        ITERATE (CSeq_loc_mix::Tdata, iter, locs) {
            TSeqRange range = (*iter)->GetTotalRange();
            TSeqPos from    = range.GetFrom() + 1;
            TSeqPos to      = range.GetTo() + 1;
            switch (count++)
            {
            case 0:
                forward += " " + NStr::IntToString(from) + ".." + NStr::IntToString(to) +
                    " length " + NStr::IntToString(to-from+1);
                break;
            case 1:
                reverse += " " + NStr::IntToString(from) + ".." + NStr::IntToString(to) +
                    " length " + NStr::IntToString(to-from+1);
                break;
            default:
                // do nothing - malformed primer pair
                break;
            }
        }

        if (m_Feat->IsSetComment()  &&  m_Feat->GetComment() != t_title) {
            tooltip.AddRow("Comment:", m_Feat->GetComment());
        }

        val = m_Feat->GetNamedQual("FW_TM"); if (!val.empty()) forward += " Tm " + val;
        val = m_Feat->GetNamedQual("RV_TM"); if (!val.empty()) reverse += " Tm " + val;
        val = m_Feat->GetNamedQual("FW_GC"); if (!val.empty()) forward += " GC " + val + '%';
        val = m_Feat->GetNamedQual("RV_GC"); if (!val.empty()) reverse += " GC " + val + '%';
        val = m_Feat->GetNamedQual("FW_SEQ"); if (!val.empty()) forward += " Seq " + val;
        val = m_Feat->GetNamedQual("RV_SEQ"); if (!val.empty()) reverse += " Seq " + val;
        if (!forward.empty()) {
            tooltip.AddRow("Forward:", forward);
        }
        if (!reverse.empty()) {
            tooltip.AddRow("Reverse:", reverse);
        }
    } 

    if (m_Location) {
        tooltip.AddRow("PCR product length:", NStr::IntToString(m_Location->GetTotalRange().GetLength(), NStr::fWithCommas));
    }
}



END_NCBI_SCOPE
