/*  $Id: bins_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
* Authors:  Melvin Quintos, Dmitry Rudnev
*
* File Description:
*    CBinsGlyph -- Glyph to represent the bins track
*
*/

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_snp/bins/bins_glyph.hpp>
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

string CBinsGlyph::GenerateBinSignature(const string& title,
                                        const string& annot_name,
                                        TGi gi,
                                        TSeqRange range,
                                        NSnpBins::TBinType BinType)
{
    char buff[128];
    Uint2 type;

    // create type
    type = (CObjFingerprint::eSnpTrack) << 8;
    type |= BinType;

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
// CBinsGlyph::CBinsGlyph()
//
CBinsGlyph::CBinsGlyph(
        const TDensityMap& mapValues,
        const CSeq_loc& loc,
        objects::CScope * scope)
    : m_Location(&loc)
    , m_Map(mapValues)
    , m_Type(NSnpBins::eGAP)
    , m_bHasObjectList(false)
    , m_Scope(scope)
{
}

bool CBinsGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    return true;
}


void CBinsGlyph::GetHTMLActiveAreas(CSeqGlyph::TAreaVector* p_areas) const
{
    bool isFlipped = m_Context->IsFlippedStrand();

    if (m_bHasObjectList == false) {
        x_BuildNonEmptyBinList();
    }
    double              window  = m_Map.GetWindow();
    const TSeqPos       start   = m_Map.GetStart();

    ITERATE(TMappedBinList, iNonEmptyBins, m_NonEmptyBins) {

        TSeqRange range = (*iNonEmptyBins)->obj->range;
        // the same formula as in x_Draw so the generated areas for drawing and javascript are as close as possible
        TVPUnit x1 = GetRenderingContext()->SeqToScreenXClipped(start + (*iNonEmptyBins)->SequentialNumber*window);
        TVPUnit x2 = GetRenderingContext()->SeqToScreenXClipped(start + ((*iNonEmptyBins)->SequentialNumber+1)*window);

        if (isFlipped) {
            x1 = -x1;
            x2 = -x2;
        }

        TMappedBinList::const_iterator itNextBin = iNonEmptyBins;
        itNextBin++;
        if ((itNextBin != m_NonEmptyBins.end()) && (((*iNonEmptyBins)->SequentialNumber+1) == (*itNextBin)->SequentialNumber)) {
            x2 -= 1;
        }

        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);
        area.m_Bounds.SetHorz(x1,x2);
        area.m_Flags = CHTMLActiveArea::fNoSelection | CHTMLActiveArea::fNoPin;
        area.m_SeqRange = range;
        area.m_Signature = (*iNonEmptyBins)->obj->signature;
        p_areas->push_back(area);
    }
}


bool CBinsGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
	return !tt.IsEmpty();
}


void CBinsGlyph::GetTooltip(const TModelPoint& pos, ITooltipFormatter& tt, string& /*t_title*/) const
{
    // Determine bin number
    // bin number is floor (pos/window)
    float           window  = m_Map.GetWindow();
    TSeqPos         start   = m_Map.GetStart();
    unsigned int    bin     = (unsigned int) floor( (pos.X()-start)/window );

    // Make sure 'bin' falls within [0, m_Map.GetBins)
    bin = max<int>(0, bin);
    bin = min<int>(bin, m_Map.GetBins()-1);

    if (m_Map[bin].obj.Empty()) return;

	//!! 0 as GI is temporary to allow for compilation for SV-2020
    NSnpGui::GetBinTooltip(*m_Map[bin].obj, CIRef<ITooltipFormatter>(&tt), ZERO_GI, x_GetColorTheme());
}


bool CBinsGlyph::IsClickable() const
{
    // TODO
    return false;
}

TSeqRange CBinsGlyph::x_GetBinRange(unsigned int bin) const
{
    // determine selected bin
    // bin number is floor (pos/window)
    double window  = m_Map.GetWindow();
    TSeqPos start = m_Map.GetStart();

    TSeqPos startPos = (TSeqPos) (
                         start              // start position
                       + bin*window
                       );

    TSeqPos endPos   =  (TSeqPos) (window != 1 ?
                             start                  // start position
                             + (bin+1)*window - 1
                           :
                             startPos
                       );

    TSeqRange range(startPos, endPos);
    return range;
}

string CBinsGlyph::x_GetColorTheme() const
{
    // getting to the color scheme
    const CSeqGlyph* pImmediateParentGlyph(GetParent());
    const CSeqGlyph* pSecondParentGlyph(pImmediateParentGlyph->GetParent());

    //!! add a check for pSecondParentGlyph being indeed a CBinsTrack
    const CBinsTrack* pBinsTrack(reinterpret_cast<const CBinsTrack*>(pSecondParentGlyph));

    return pBinsTrack->GetGlobalConfig()->GetColorTheme();
}

void CBinsGlyph::x_BuildNonEmptyBinList() const
{
    // for every non-zero bin, generate the bin's signature and range
    if(!m_bHasObjectList) {
        TGi gi = sequence::GetGiForId(*(m_Location->GetId()), *m_Scope);

        NON_CONST_ITERATE(TDensityMap, iBin, m_Map) {
            if(iBin->obj.NotEmpty()) {
                iBin->obj->signature = GenerateBinSignature(GetTitle(),
                                                      GetAnnotName(),
                                                      gi,
                                                      iBin->obj->range,
                                                      iBin->obj->type);
                m_NonEmptyBins.push_back(&(*iBin));
            }
        }
        m_bHasObjectList = true;
    }
}

void CBinsGlyph::x_Draw() const
{
    IRender& gl = GetGl();
    const bool isFlipped = m_Context->IsFlippedStrand();
    const int binHeight = NSnpGui::c_BinHeight;
    const int binWidth  = NSnpGui::c_BinWidth;

    TModelUnit top = GetTop();

    TModelUnit line_y1 = 1 + top;
    TModelUnit line_y2 = line_y1 + binHeight;

    // SETUP COMMON attributes
    double              window  = m_Map.GetWindow();
    const TSeqPos       start   = m_Map.GetStart();
    const size_t        numBins = m_Map.GetBins();
    //const TModelUnit    offset  = m_Context->GetOffset();

    // Y values are the same
    TModelUnit y1 = line_y1;
    TModelUnit y2 = line_y2;

    TModelUnit x1 = 0.0;
    TModelUnit x2 = 0.0;

    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Bold, 8);
    CGlTextureFont font_plus(CGlTextureFont::eFontFace_Helvetica_Bold, 10);
	CRgbaColor colorBin;
	CRgbaColor colorText;
	string text;

    string sColorTheme(x_GetColorTheme());
    string sColorBinKey;
    string sColorKey("Default");

    CGuiRegistry& registry(CGuiRegistry::GetInstance());

    // Render the colored bins and texts over them
    for(size_t bin = 0;  bin < numBins; bin++)
    {
        if (m_Map[bin].obj.NotEmpty()) {
            switch(m_Type) {
				case NSnpBins::eCITED:
                    sColorBinKey = "Cited";
                    break;
                case NSnpBins::eCLIN:
                    sColorBinKey = "Clinical";
                    sColorKey = NSnpGui::ClinSigAsColorKey(m_Map[bin].obj->m_SigEntry->ClinSigID);
                    break;
                case NSnpBins::eIND:
                    sColorBinKey = "Individual";
                    break;
                case NSnpBins::eGAP:
                case NSnpBins::eGCAT:
                default:
                    sColorBinKey = "PValue";
                    sColorKey = NSnpGui::PValueAsColorKey(m_Map[bin].obj->m_SigEntry->pvalue);
                    break;
            }

            CRegistryReadView ColorView(CSGConfigUtils::GetColorReadView(registry, "GBPlugins.BinTrack", sColorBinKey, sColorTheme));
            CSGConfigUtils::GetColor(ColorView, sColorKey, colorBin);
            // draw the bin
            x1 = start + bin*(window);
            x2 = start + (bin+1)*(window);

            // Draw filled box
            gl.ColorC(colorBin);
            m_Context->DrawQuad(x1, y1, x2, y2);

            gl.Color4f(.76f,.76f,.76f, 1.0f);
            m_Context->DrawRect(x1, y1, x2, y2);

			// draw the bin text
			colorText = colorBin.ContrastingColor();
            gl.ColorC(colorText);
            int count = m_Map[bin].obj->count;
            x1   = ((bin)*(window) + start);
            if( count > 9)
                text = "+";
            else
                NStr::IntToString(text, count);

            int dx = (isFlipped) ? 2 : -2;

            TModelUnit w = ((binWidth + dx)/2)*(m_Context->GetScale());
            TModelUnit h = gl.TextHeight(&font);

            if (text == "+") {
                m_Context->TextOut(&font_plus, text.c_str(),
                    x1+w, y2 - (binHeight - h) * 0.5, true);
            }
            else {
                m_Context->TextOut(&font, text.c_str(),
                    x1+w, y2 - (binHeight - h) * 0.5, true);
            }
        }
    }
    gl.Color4f(0.0f, 0.0f,0.0f, 1.0f);
}

void CBinsGlyph::x_UpdateBoundingBox()
{
    SetHeight(NSnpGui::c_BinHeight);
    TSeqRange range = GetRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
}

TSeqRange CBinsGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}

END_NCBI_SCOPE
