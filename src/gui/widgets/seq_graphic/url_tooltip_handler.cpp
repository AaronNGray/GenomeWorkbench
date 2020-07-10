/* $Id: url_tooltip_handler.cpp 43343 2019-06-17 20:41:12Z evgeniev $
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
* Author:  Vladislav Evgeniev
*
* File Description: Defines a class to handle URL clicks in GBench
*/

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/url_tooltip_handler.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <gui/utils/view_event.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include <wx/msgdlg.h>
#include <wx/app.h>

#include "unaligned_region_dlg.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const TSignedSeqPos basesThreshold(30000);
const TSignedSeqPos offset(200);

//!! this needs to be fixed, as of now this is grossly incorrect
//!! e.g., a call to GetParamValue("https://www.ncbi.nlm.nih.gov?id=NC_000007.13&v=94292473:94292473&mk=94292473|rs1272819133|008000", "v") returns 
//!! "?id=NC_000007.13" instead of the expected "94292473:94292473"
//!! luckily, no such calls below so far, but the potential bomb exists
string GetParamValue(const string &href, const string &name)
{

    size_t namePos = href.find(name);
    if (string::npos == namePos)
        return string();
    size_t nextPos = href.find('&', namePos);
    if (string::npos == nextPos)
        nextPos = href.length();
    return href.substr(namePos + name.length() + 1, nextPos - namePos - name.length() - 1);
}

bool CURLTooltipHandler::ProcessURL(const string &href)
{
    const string unalignedRegionHref = "$UNALIGNED_REGION$";
    const string genomicLinkHref = "$GENOMIC_LINK$";
    if (string::npos != href.find(unalignedRegionHref)) {
        x_ParseUnalignedRegionHref(href);
        return true;
    }
    else if (string::npos != href.find(genomicLinkHref)) {
        x_ParseGenomicLinkHref(href);
        return true;
    }
    return false;
}


void CURLTooltipHandler::x_ParseUnalignedRegionHref(const string &href)
{
    string seqIdStr = GetParamValue(href, "id");
    TSignedSeqPos from(0);
    TSignedSeqPos to(0);
    bool    polyA(false);
    string value = GetParamValue(href, "from");
    if (!value.empty())
        from = NStr::StringToUInt(value, NStr::fConvErr_NoThrow);
    value = GetParamValue(href, "to");
    if (!value.empty())
        to = NStr::StringToUInt(value, NStr::fConvErr_NoThrow);
    value = GetParamValue(href, "polyA");
    if ("true" == value)
        polyA = true;
    bool reverse(false);
    value = GetParamValue(href, "reverse");
    if ("true" == value)
        reverse = true;
    bool flip(false);
    value = GetParamValue(href, "flip");
    if ("true" == value)
        flip = true;
    x_DisplayUnalignedRegion(seqIdStr, from, to, polyA, reverse, flip);
}

void CURLTooltipHandler::x_ParseGenomicLinkHref(const string &href)
{
    string seqIdStr = GetParamValue(href, "id");
    CRef<CSeq_id> id(new CSeq_id);
    try {
        id->Set(seqIdStr);
    }
    catch (const CException &error) {
        LOG_POST(Error << "x_ParseGenomicLinkHref: " << error.what());
        return;
    }
    CRef<CUser_object> params(new CUser_object());
    CRef<CObject_id> type(new CObject_id());
    type->SetStr("GraphicalViewParams");
    params->SetType(*type);

    CUser_object::TData& data = params->SetData();

    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("Settings");
        param->SetLabel(*label);
        string url(x_ReformatUrlMarkerParam(href.substr(href.find('?') + 1)));
        param->SetData().SetStr(url);
        data.push_back(param);
    }

    COpenGraphicViewEvent evt(SConstScopedObject(id, &m_Scope), params);
    m_Handler.Send(&evt, CEventHandler::eDispatch_Default, CEventHandler::ePool_Parent);
}

void CURLTooltipHandler::x_ReverseSequence(string& seq, bool is_protein, bool reverse, bool flipped_strands)
{
    string tmp_seq{ seq };
    if (reverse) {
        if (is_protein) {
            // protein sequence, just reverse it
            CSeqManip::Reverse(tmp_seq, CSeqUtil::e_Iupacaa, 0, tmp_seq.length(), seq);
        }
        else {
            // nucleotide sequence, get reverse and complement
            if (!flipped_strands) {
                CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna, 0, tmp_seq.length(), seq);
            }
        }
    }
    else {
        if (!is_protein && flipped_strands) {
            CSeqManip::ReverseComplement(tmp_seq, CSeqUtil::e_Iupacna, 0, tmp_seq.length(), seq);
        }
    }
}

void CURLTooltipHandler::x_DisplayUnalignedRegion(const std::string &seq_id, TSignedSeqPos from, TSignedSeqPos to, bool polyA, bool reverse, bool flipped_strands)
{
    CSeq_id id;
    try {
        id.Set(seq_id);
    }
    catch (const CException &error) {
        LOG_POST(Error << "x_DisplayUnalignedRegion: " << error.what());
        return;
    }

    CBioseq_Handle bsh = m_Scope.GetBioseqHandle(id);
    if (!bsh) {
        wxMessageBox("Failed to retrieve sequence: " + seq_id);
        return;
    }

    CSeqVector vec(bsh, CBioseq_Handle::eCoding_Iupac);
    CUnalignedRegionDlg dialogUnalignedRegion(wxTheApp->GetTopWindow());

    if (reverse != flipped_strands) {
        x_DisplayReverseStrandRegion(vec, from, to, dialogUnalignedRegion, bsh.IsAa(), reverse, flipped_strands);
    }
    else {
        x_DisplayForwardStrandRegion(vec, from, to, dialogUnalignedRegion, bsh.IsAa(), reverse, flipped_strands);
    }

    if (polyA) {
        dialogUnalignedRegion.SetTitle(dialogUnalignedRegion.GetTitle() + " - poly(A)-tail");
    }
    dialogUnalignedRegion.ShowModal();
}

void CURLTooltipHandler::x_DisplayForwardStrandRegion(const CSeqVector &vec, TSignedSeqPos from, TSignedSeqPos to, CUnalignedRegionDlg &dlgUnalignedRegion, bool is_protein, bool reverse, bool flipped_strands)
{
    size_t hiddenBasesCount(0);
    if ((to - from) > basesThreshold)
        hiddenBasesCount = to - from - basesThreshold;

    string seq_data;
    TSignedSeqPos displayFrom(from);
    TSignedSeqPos displayTo(to);
    

    if (0 == from) {
        if (hiddenBasesCount) {
            displayFrom = 0;
            displayTo = basesThreshold / 2;
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);

            wxString hiddenBasesText;
            hiddenBasesText.Printf("\n...(%ld more unaligned bases)...\n", (long)hiddenBasesCount);
            dlgUnalignedRegion.WriteNormalText(hiddenBasesText);

            displayFrom = to - basesThreshold / 2;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }
        else {
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }

        displayFrom = to;
        displayTo = (to + offset) >= vec.size() ? (vec.size() - 1) : (to + offset);
        seq_data.clear();
        vec.GetSeqData(displayFrom, displayTo, seq_data);
        x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
        dlgUnalignedRegion.WriteNormalText(seq_data);
    }
    else {
        displayFrom = (from - offset) > 0 ? (from - offset) : 0;
        displayTo = from;
        vec.GetSeqData(displayFrom, displayTo, seq_data);
        x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
        dlgUnalignedRegion.WriteNormalText(seq_data);

        if (hiddenBasesCount) {
            displayFrom = from;
            displayTo = from + basesThreshold / 2;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);

            wxString hiddenBasesText;
            hiddenBasesText.Printf("\n...(%ld more unaligned bases)...\n", (long)hiddenBasesCount);
            dlgUnalignedRegion.WriteNormalText(hiddenBasesText);

            displayFrom = to - basesThreshold / 2;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }
        else {
            displayFrom = from;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }
    }
}

void CURLTooltipHandler::x_DisplayReverseStrandRegion(const CSeqVector &vec, TSignedSeqPos from, TSignedSeqPos to, CUnalignedRegionDlg &dlgUnalignedRegion, bool is_protein, bool reverse, bool flipped_strands)
{
    size_t hiddenBasesCount(0);
    if ((to - from) > basesThreshold)
        hiddenBasesCount = to - from - basesThreshold;

    string seq_data;
    TSignedSeqPos displayFrom(from);
    TSignedSeqPos displayTo(to);

    if (0 == from) {
        displayFrom = to;
        displayTo = (to + offset) >= vec.size() ? (vec.size() - 1) : (to + offset);
        vec.GetSeqData(displayFrom, displayTo, seq_data);
        x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
        dlgUnalignedRegion.WriteNormalText(seq_data);

        if (hiddenBasesCount) {
            displayFrom = to - basesThreshold / 2;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);

            wxString hiddenBasesText;
            hiddenBasesText.Printf("\n...(%ld more unaligned bases)...\n", (long)hiddenBasesCount);
            dlgUnalignedRegion.WriteNormalText(hiddenBasesText);

            displayFrom = 0;
            displayTo = basesThreshold / 2;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }
        else {
            displayFrom = from;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }
    }
    else {

        if (hiddenBasesCount) {
            displayFrom = to - basesThreshold / 2;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);

            wxString hiddenBasesText;
            hiddenBasesText.Printf("\n...(%ld more unaligned bases)...\n", (long)hiddenBasesCount);
            dlgUnalignedRegion.WriteNormalText(hiddenBasesText);

            displayFrom = from;
            displayTo = from + basesThreshold / 2;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }
        else {
            displayFrom = from;
            displayTo = to;
            seq_data.clear();
            vec.GetSeqData(displayFrom, displayTo, seq_data);
            x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
            dlgUnalignedRegion.WriteRedText(seq_data);
        }

        displayFrom = (from - offset) > 0 ? (from - offset) : 0;
        displayTo = from;
        seq_data.clear();
        vec.GetSeqData(displayFrom, displayTo, seq_data);
        x_ReverseSequence(seq_data, is_protein, reverse, flipped_strands);
        dlgUnalignedRegion.WriteNormalText(seq_data);
    }
}

// Converts the marker position to zero-based coordinates
string CURLTooltipHandler::x_ReformatUrlMarkerParam(const std::string &url_params)
{
    vector<string> params;
    NStr::Split(url_params, "&", params);
    for (auto& param : params) {
        size_t pos = param.find("mk=");
        if (string::npos == pos)
            continue;

        vector<string> marker;
        NStr::Split(param.substr(3), "|", marker);

        if (3 != marker.size())
            break;

        TSeqPos mpos = NStr::StringToNumeric<TSeqPos>(marker[0]);

        param = "mk=";
        param += NStr::NumericToString<TSeqPos>(--mpos);
        param += '|';
        param += marker[1];
        param += '|';
        param += marker[2];

        break;
    }
    string result;
    if (0 == params.size())
        return result;

    result = params[0];

    for (size_t i = 1; i < params.size(); ++i) {
        result += '&';
        result += params[i];
    }

    return result;
}

END_NCBI_SCOPE
