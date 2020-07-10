/*  $Id: gmark_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
* Authors:  Dmitry Rudnev
*
* File Description:
*    CGeneMarkerGlyph -- Glyph that represents Gene markers encoded similar to SNP bins
*
*/

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_snp/bins/gmark_glyph.hpp>
#include <gui/packages/pkg_snp/bins/bins_track.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>
#include <util/checksum.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

string CGeneMarkerGlyph::GenerateGeneMarkerSignature(const string& title,
                                        const string& annot_name,
                                        TGi gi,
                                        TSeqRange range,
                                        const string& trackSubType)
{
    char buff[128];
    Uint2 type;

    // create type
    type = (CObjFingerprint::eSnpTrack) << 8;
    type |= NSnpBins::eGAP;

    // create checksum
    CChecksum cs(CChecksum::eCRC32);
    Uint4 checksum(0);
    if(!title.empty()) {
        cs.AddLine(title);
        checksum = cs.GetChecksum();
    }

    CChecksum cs_name(CChecksum::eCRC32);
    Uint4 checksum_name;
    cs_name.AddLine(annot_name);
    checksum_name = cs_name.GetChecksum();

    sprintf(buff, "gi|%d-%08x-%08x-%04x-%08x-%08x",
            GI_TO(int, gi), range.GetFrom(), range.GetTo(),
            type, checksum, checksum_name);

    return string(buff);
}

//
// CGeneMarkerGlyph::CGeneMarkerGlyph()
//
CGeneMarkerGlyph::CGeneMarkerGlyph(
        const TGeneMarkers& GeneMarkers,
        const CSeq_loc& loc,
        objects::CScope * scope)
    : m_Location(&loc)
    , m_GeneMarkers(GeneMarkers)
    , m_bHasSignatures(false)
    , m_Scope(scope)
{
}

bool CGeneMarkerGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    return true;
}


void CGeneMarkerGlyph::GetHTMLActiveAreas(CSeqGlyph::TAreaVector* p_areas) const
{
    bool isFlipped = m_Context->IsFlippedStrand();

    if (m_bHasSignatures == false) {
        x_BuildSignatures();
    }
    ITERATE(TGeneMarkers, iGeneMarkers, m_GeneMarkers) {
        TSeqRange range = (*iGeneMarkers)->range;
        // the same formula as in x_Draw so the generated areas for drawing and javascript are as close as possible
        TVPUnit x1 = GetRenderingContext()->SeqToScreenXClipped(range.GetFrom());
        TVPUnit x2 = GetRenderingContext()->SeqToScreenXClipped(range.GetTo()+1);

        if (isFlipped) {
            x1 = -x1;
            x2 = -x2;
        }

        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);
        area.m_Bounds.SetHorz(x1,x2);
        area.m_Flags = CHTMLActiveArea::fNoSelection | CHTMLActiveArea::fNoPin;
        area.m_SeqRange = range;
        area.m_Signature = (*iGeneMarkers)->signature;
        p_areas->push_back(area);
    }
}


bool CGeneMarkerGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
	return !tt.IsEmpty();
}


void CGeneMarkerGlyph::GetTooltip(const TModelPoint& pos, ITooltipFormatter& tt, string& /*t_title*/) const
{
    // find the gene marker that contains this position
    ITERATE(TGeneMarkers, iGeneMarkers, m_GeneMarkers) {
        TSeqRange range = (*iGeneMarkers)->range;

        if(range.GetFrom() <= pos.X() && pos.X() <= range.GetTo()) {
            //!! 0 as GI is temporary to allow for compilation for SV-2020
            NSnpGui::GetBinTooltip(*(*iGeneMarkers), CIRef<ITooltipFormatter>(&tt), ZERO_GI, x_GetColorTheme());

            break;
        }
    }
}


bool CGeneMarkerGlyph::IsClickable() const
{
    // TODO
    return false;
}

string CGeneMarkerGlyph::x_GetColorTheme() const
{
    // getting to the color scheme
    const CSeqGlyph* pImmediateParentGlyph(GetParent());
    const CSeqGlyph* pSecondParentGlyph(pImmediateParentGlyph->GetParent());

    //!! add a check for pSecondParentGlyph being indeed a CBinsTrack
    const CBinsTrack* pBinsTrack(reinterpret_cast<const CBinsTrack*>(pSecondParentGlyph));

    return pBinsTrack->GetGlobalConfig()->GetColorTheme();
}

void CGeneMarkerGlyph::x_BuildSignatures() const
{
    // for every non-zero bin, generate the bin's signature and range
    if(!m_bHasSignatures) {
        TGi gi = sequence::GetGiForId(*(m_Location->GetId()), *m_Scope);
        NON_CONST_ITERATE(TGeneMarkers, iGeneMarkers, m_GeneMarkers) {
            (*iGeneMarkers)->signature = GenerateGeneMarkerSignature(GetTitle(),
                                                      GetAnnotName(),
                                                      gi,
                                                      (*iGeneMarkers)->range,
                                                      (*iGeneMarkers)->m_SigEntry->trackSubType);
        }
        m_bHasSignatures = true;
    }
}

void CGeneMarkerGlyph::x_Draw() const
{
    IRender& gl = GetGl();
    const int binHeight = NSnpGui::c_BinHeight;

    TModelUnit top = GetTop();

    TModelUnit line_y1 = 1 + top;
    TModelUnit line_y2 = line_y1 + binHeight;

    // Y values are the same
    TModelUnit y1 = line_y1;
    TModelUnit y2 = line_y2;

    TModelUnit x1 = 0.0;
    TModelUnit x2 = 0.0;

	CRgbaColor colorBin;

    string sColorTheme(x_GetColorTheme());
    string sColorBinKey;
    string sColorKey("Default");

    CGuiRegistry& registry(CGuiRegistry::GetInstance());

    // Render the colored Gene marker bars
    ITERATE(TGeneMarkers, iGeneMarkers, m_GeneMarkers) {
    {
        sColorBinKey = "PValue";
        sColorKey = NSnpGui::PValueAsColorKey((*iGeneMarkers)->m_SigEntry->pvalue);

        CRegistryReadView ColorView(CSGConfigUtils::GetColorReadView(registry, "GBPlugins.BinTrack", sColorBinKey, sColorTheme));
        CSGConfigUtils::GetColor(ColorView, sColorKey, colorBin);
        TSeqRange range = (*iGeneMarkers)->range;
            // draw the bar
            x1 = range.GetFrom();
            x2 = range.GetTo() + 1;

            // Draw filled box
            gl.ColorC(colorBin);
            m_Context->DrawQuad(x1, y1, x2, y2);

            gl.Color4f(.76f,.76f,.76f, 1.0f);
            m_Context->DrawRect(x1, y1, x2, y2);
        }
    }
    gl.Color4f(0.0f, 0.0f,0.0f, 1.0f);
}

void CGeneMarkerGlyph::x_UpdateBoundingBox()
{
    SetHeight(NSnpGui::c_BinHeight);
    TSeqRange range = GetRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
}

TSeqRange CGeneMarkerGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}

END_NCBI_SCOPE
