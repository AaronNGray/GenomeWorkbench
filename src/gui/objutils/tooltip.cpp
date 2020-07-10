/*  $Id: tooltip.cpp 44708 2020-02-27 16:13:12Z rudnev $
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
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/utils.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

CIRef<ITooltipFormatter> ITooltipFormatter::CreateTooltipFormatter(ETooltipFormatters eFormatter)
{
    switch(eFormatter)
    {
    case eTooltipFormatter_CSSTable:
        return CCSSTableTooltipFormatter::CreateTooltipFormatter();
    case eTooltipFormatter_Html:
        return CHtmlTooltipFormatter::CreateTooltipFormatter();
    case eTooltipFormatter_Text:
        return CTextTooltipFormatter::CreateTooltipFormatter();
    }
    NCBI_ASSERT(false, "Unknown tooltip formatter type!");
	return null;
}

static void s_AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent, ITooltipFormatter* pFormatter)
{
    const string pmURL("/pubmed/");
    string pmlinks;
    if (!pmids.empty()) {
        vector<string> tokens;
        NStr::Split(pmids, ",", tokens);
        bool bFirst = true;
        ITERATE(vector<string>, pmidIter, tokens) {
            string pmid = *pmidIter;
            NStr::TruncateSpacesInPlace(pmid);
            // check that the string looks like a valid PubmedID (integer > 0)
            int ipmid = NStr::StringToInt(pmid, NStr::fConvErr_NoThrow);
            if(ipmid > 0) {
                if (bFirst) {
                    bFirst = false;
                    pmlinks = pFormatter->CreateNcbiLink(pmid, pmURL+pmid);
                }
                else {
                    pmlinks += ", " + pFormatter->CreateNcbiLink(pmid, pmURL+pmid);
                }
            }
        }
        if(!pmlinks.empty()) {
            pFormatter->StartRow();
            if(isBulletColPresent) {
                pFormatter->AddBulletCol();
            }
            pFormatter->AddTagCol("PubMed:");
            pFormatter->AddValueCol(pmlinks);
            pFormatter->FinishRow();
        }
    }
}


static void s_MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent, ITooltipFormatter* pFormatter)
{
    if(isGoToPresent)
        return;
    pFormatter->StartRow();
    if(isBulletColPresent)
        pFormatter->AddBulletCol();
    pFormatter->AddTagCol("Go to:");
    pFormatter->FinishRow();

    isGoToPresent = true;
}



CIRef<ITooltipFormatter> CCSSTableTooltipFormatter::CreateTooltipFormatter()
{
    return CIRef<ITooltipFormatter>(new CCSSTableTooltipFormatter());
}

CIRef<ITooltipFormatter> CCSSTableTooltipFormatter::CreateInstance()
{
    return CIRef<ITooltipFormatter>(new CCSSTableTooltipFormatter());
}

void CCSSTableTooltipFormatter::StartRow()
{
    m_sTooltipText += "<tr class=\"sv-bintrack\">";
}

void CCSSTableTooltipFormatter::AddBulletCol(const string& sBulletSrc)
{
    m_sTooltipText += "<td class=\"sv-bintrack-bullet\">";
    if (!sBulletSrc.empty())
        m_sTooltipText += "<img class=\"sv-bintrack\" src=\"" + sBulletSrc + "\"/>";
    m_sTooltipText += "</td>";
}


void CCSSTableTooltipFormatter::AddTagCol(const string& sContents, const string& sBulletSrc)
{
    m_sTooltipText += "<td class=\"sv-bintrack-tag\">";
    if (!sBulletSrc.empty())
        m_sTooltipText += "<img class=\"sv-bintrack\" src=\"" + sBulletSrc + "\"/>&nbsp;";
    m_sTooltipText += sContents + "</td>";
}

void CCSSTableTooltipFormatter::AddValueCol(const string& sContents, unsigned width, bool isNoWrap)
{
    string styleAttr;
    if (isNoWrap || width) {
        styleAttr = " style=\"";
        styleAttr += isNoWrap ? "white-space:nowrap;" : "";
        styleAttr += "\"";
    }
    m_sTooltipText += string("<td class=\"sv-bintrack-value\"") +
                        styleAttr + 
                        string(">") +
                        sContents + "</td>";
}

void CCSSTableTooltipFormatter::FinishRow()
{
    m_sTooltipText += "</tr>";
}

void CCSSTableTooltipFormatter::AddDividerRow(unsigned colspan)
{
    m_sTooltipText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack\" colspan=\"" + NStr::UIntToString(colspan) + "\"><hr></td></tr>";
}

void CCSSTableTooltipFormatter::AddRow(const string& sContents, unsigned colspan)
{
    m_sTooltipText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack-tag\" colspan=\"" + NStr::UIntToString(colspan) + "\">" + sContents + "</td></tr>";
}

void CCSSTableTooltipFormatter::AddRow(const string& sTag, const string& sValue, unsigned valueColWidth, bool)
{
    m_sTooltipText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack-tag\">";
    m_sTooltipText += sTag;
    m_sTooltipText += "</td><td class=\"sv-bintrack-value\"";
    m_sTooltipText += ">";
    m_sTooltipText += sValue;
    m_sTooltipText += "</td></tr>";
}

void CCSSTableTooltipFormatter::AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth)
{
    m_sLinksText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack-tag\">";
    m_sLinksText += sTag;
    m_sLinksText += "</td><td class=\"sv-bintrack-value\"";
    m_sLinksText += ">";
    m_sLinksText += sValue;
    m_sLinksText += "</td></tr>";
}

void CCSSTableTooltipFormatter::AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth)
{
    m_sLinksText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack-tag\">";
    m_sLinksText += sTag;
    m_sLinksText += "</td><td class=\"sv-bintrack-value\"";
    m_sLinksText += "><a href=\"";
    m_sLinksText += sUrl;
    m_sLinksText += "\">";
    m_sLinksText += sText;
    m_sLinksText += "</a></td></tr>";
}

void CCSSTableTooltipFormatter::AddLinksTitle(const string& sTitle)
{
    m_sLinksText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack-tag\" align=\"right\" valign=\"top\" style=\"white-space: nowrap\">[<i>" + sTitle + "</i>]</td><td class=\"sv-bintrack\" colspan=\"0\"/></tr>";
}

void CCSSTableTooltipFormatter::AddSectionRow(const string& sContents)
{
    m_sTooltipText += "<tr class=\"sv-bintrack\"><td class=\"sv-bintrack-value\" align=\"right\" valign=\"top\" style=\"white-space: nowrap\">[<i>" + sContents + "</i>]</td><td class=\"sv-bintrack\" colspan=\"0\"/></tr>";
}

string CCSSTableTooltipFormatter::Render()
{
    if (IsEmpty())
        return "";
    string sWholeTooltip;
    if (!m_sTooltipText.empty()) {
        sWholeTooltip += "<table class=\"sv-bintrack\" style=\"border-spacing:2px\">" + m_sTooltipText + "</table>";
    }
    if (!m_sLinksText.empty()) {
        if (!m_sTooltipText.empty())
            sWholeTooltip += "<br/>&nbsp";
        sWholeTooltip += "<table class=\"sv-bintrack\" style=\"border-spacing:2px\">" + m_sLinksText + "</table>";
        m_sLinksText.clear();
    }
    m_sTooltipText.clear();
    return sWholeTooltip;
}

bool CCSSTableTooltipFormatter::IsEmpty() const
{
    return m_sTooltipText.empty();
}

void CCSSTableTooltipFormatter::AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent)
{
    s_AddPubmedLinksRow(pmids, isGoToPresent, isBulletColPresent, this);
}

void CCSSTableTooltipFormatter::MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent)
{
    s_MaybeAddGoToRow(isGoToPresent, isBulletColPresent, this);
}

string CCSSTableTooltipFormatter::CreateLink(const string& sText, const string& sUrl) const
{
    return "<a class=\"sv-bintrack\" href=\"" + sUrl + "\" target=\"_blank\">"+ sText + "</a>";
}

string CCSSTableTooltipFormatter::CreateNcbiLink(const string& sText, const string& sUrl) const
{
    return CreateLink(sText, sUrl);
}

void CCSSTableTooltipFormatter::Append(const ITooltipFormatter& tooltip)
{
    const CCSSTableTooltipFormatter* pCSSTableFormatter = dynamic_cast<const CCSSTableTooltipFormatter*>(&tooltip);
    if (!pCSSTableFormatter)
        return;

    m_sTooltipText += pCSSTableFormatter->m_sTooltipText;
    m_sLinksText += pCSSTableFormatter->m_sLinksText;
}

CIRef<ITooltipFormatter> CHtmlTooltipFormatter::CreateTooltipFormatter()
{
    return CIRef<ITooltipFormatter>(new CHtmlTooltipFormatter());
}

CIRef<ITooltipFormatter> CHtmlTooltipFormatter::CreateInstance()
{
    return CIRef<ITooltipFormatter>(new CHtmlTooltipFormatter());
}

void CHtmlTooltipFormatter::StartRow()
{
    m_sTooltipText += "<tr>";
}

void CHtmlTooltipFormatter::AddBulletCol(const string& sBulletSrc)
{
    m_sTooltipText += "<td valign=\"top\" align=\"right\" nowrap>";
    if(!sBulletSrc.empty()) {
        m_sTooltipText += "<img src=\"" + CSeqUtils::GetNcbiBaseUrl() + sBulletSrc +"\"/>&nbsp;";
    }
    m_sTooltipText += "</td>";
}


void CHtmlTooltipFormatter::AddTagCol(const string& sContents, const string& sBulletSrc)
{
    m_sTooltipText += "<td valign=\"top\" align=\"right\" nowrap>";
    if (!sBulletSrc.empty()) 
        m_sTooltipText += "<img src=\"" + CSeqUtils::GetNcbiBaseUrl() + sBulletSrc + "\"/>&nbsp;";
    m_sTooltipText += "<span style=\"font-weight:bold\">" + sContents + "</span></td>";
}

void CHtmlTooltipFormatter::AddValueCol(const string& sContents, unsigned width, bool isNoWrap)
{
    string widthAttr;
    if (width) {
        widthAttr = " width=\"" + NStr::UIntToString(width) + "\"";
    }
    m_sTooltipText += string("<td valign=\"top\"") +
    string(isNoWrap ? " nowrap" : "") +
    widthAttr +
    string(">") +
    sContents +
    "</td>";
}

void CHtmlTooltipFormatter::FinishRow()
{
    m_sTooltipText += "</tr>";
}

void CHtmlTooltipFormatter::AddDividerRow(unsigned colspan)
{
    m_sTooltipText += "<tr><td colspan=\"" + NStr::UIntToString(colspan) + "\"><hr></td></tr>";
}

void CHtmlTooltipFormatter::AddRow(const string& sContents, unsigned colspan)
{
    m_sTooltipText += "<tr><td colspan=\"" + NStr::UIntToString(colspan) + "\">" + sContents + "</td></tr>";
}

void CHtmlTooltipFormatter::AddRow(const string& sTag, const string& sValue, unsigned valueColWidth, bool noSpaceInsert)
{
    string spacedValue(sValue);
	if(!noSpaceInsert) {
		x_InsertSpaces(spacedValue);
	}
    m_sTooltipText += "<tr><td align=\"right\" valign=\"top\" nowrap><span style=\"font-weight:bold\">";
    m_sTooltipText += sTag;
    m_sTooltipText += "</span></td><td";
    if (valueColWidth)
        m_sTooltipText += " width=\"" + NStr::UIntToString(valueColWidth) + "\">";
    else
        m_sTooltipText += ">";
    m_sTooltipText += spacedValue;
    m_sTooltipText += "</td></tr>";
}

void CHtmlTooltipFormatter::AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth)
{
    m_sLinksText += "<tr><td align=\"right\" valign=\"top\" nowrap><span style=\"font-weight:bold\">";
    m_sLinksText += sTag;
    m_sLinksText += "</span></td><td ";
    if (valueColWidth)
        m_sLinksText += " width=\"" + NStr::UIntToString(valueColWidth) + "\">";
    else
        m_sLinksText += ">";
    m_sLinksText += sValue;
    m_sLinksText += "</td></tr>";
}

void CHtmlTooltipFormatter::AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth)
{
    m_sLinksText += "<tr><td align=\"right\" valign=\"top\" nowrap><span style=\"font-weight:bold\">";
    m_sLinksText += sTag;
    m_sLinksText += "</span></td><td";
    if (valueColWidth)
        m_sLinksText += " width=\"" + NStr::UIntToString(valueColWidth) + "\"><a href=\"";
    else
        m_sLinksText += "><a href=\"";
    m_sLinksText += sUrl;
    m_sLinksText += "\">";
    m_sLinksText += sText;
    m_sLinksText += "</a></td></tr>";
}

void CHtmlTooltipFormatter::AddLinksTitle(const string& sTitle)
{
    m_sLinksText += "<tr><td align=\"right\" valign=\"top\" nowrap>[<i>" + sTitle + "</i>]</td><td colspan=\"0\"/></tr>";
}

void CHtmlTooltipFormatter::AddSectionRow(const string& sContents)
{
    m_sTooltipText += "<tr><td align=\"right\" valign=\"top\" nowrap>[<i>" + sContents + "</i>]</td><td colspan=\"0\"/></tr>";
}

string CHtmlTooltipFormatter::Render()
{
    if (IsEmpty())
        return "";
    string sWholeTooltip;
    if (!m_sTooltipText.empty()) {
        sWholeTooltip += "<table margin=\"0\" padding=\"0\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">" + m_sTooltipText + "</table>";
    }
    if (!m_sLinksText.empty()) {
        if (!m_sTooltipText.empty())
            sWholeTooltip += "<br/>&nbsp";
        sWholeTooltip += "<table margin=\"0\" padding=\"0\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">" + m_sLinksText + "</table>";
        m_sLinksText.clear();
    }
    m_sTooltipText.clear();
    return sWholeTooltip;
}

bool CHtmlTooltipFormatter::IsEmpty() const
{
    return m_sTooltipText.empty();
}

void CHtmlTooltipFormatter::AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent)
{
    s_AddPubmedLinksRow(pmids, isGoToPresent, isBulletColPresent, this);
}

void CHtmlTooltipFormatter::MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent)
{
    s_MaybeAddGoToRow(isGoToPresent, isBulletColPresent, this);
}

string CHtmlTooltipFormatter::CreateLink(const string& sText, const string& sUrl) const
{
	string sSpacedText(sText);
	x_InsertSpaces(sSpacedText);
	return "<a href=\"" + sUrl + "\" target=\"_blank\">"+ sSpacedText + "</a>";
}

string CHtmlTooltipFormatter::CreateNcbiLink(const string& sText, const string& sUrl) const
{
    return CreateLink(sText, NStr::StartsWith(sUrl, CSeqUtils::GetNcbiBaseUrl()) 
                                ? sUrl : CSeqUtils::GetNcbiBaseUrl() + sUrl);
}

string CHtmlTooltipFormatter::CreateGenomicLink(const string& sText, const string& sUrl) const
{
    string sUpdateURL = NStr::Replace(sUrl, "/projects/sviewer/", "");
    return CreateLink(sText, string("$GENOMIC_LINK$") + sUpdateURL);
}

void CHtmlTooltipFormatter::Append(const ITooltipFormatter& tooltip)
{
    const CHtmlTooltipFormatter* pHtmlFormatter = dynamic_cast<const CHtmlTooltipFormatter*>(&tooltip);
    if (!pHtmlFormatter)
        return;

    m_sTooltipText += pHtmlFormatter->m_sTooltipText;
    m_sLinksText += pHtmlFormatter->m_sLinksText;
}

inline void CHtmlTooltipFormatter::x_FindAllOccurences(const string &input, char search, vector<size_t> &occurences) const
{
    size_t pos = input.find(search);
    while (string::npos != pos) {
        occurences.push_back(pos);
        pos = input.find(search, pos + 1);
    }
}

unsigned CHtmlTooltipFormatter::x_InsertSpaces(string &input, size_t start, size_t end, size_t fragment_length) const
{
    size_t length = (end < input.length() ? end : input.length() - 1) - start + 1;
    unsigned fragments = (unsigned)round(length / (double)fragment_length);
    unsigned insertions(0);
    for (unsigned i = 1; i <= fragments; ++i) {
        size_t pos = start + i*fragment_length + insertions;
        if (pos > input.length())
            break;
        
        input.insert(pos, 1, ' ');
        insertions++;
    }
    return insertions;
}

void CHtmlTooltipFormatter::x_InsertSpaces(string &input, size_t fragment_length) const
{
    if (input.length() <= fragment_length)
        return;

    vector<size_t> spaces;
    x_FindAllOccurences(input, ' ', spaces);
    x_FindAllOccurences(input, '\t', spaces);
    x_FindAllOccurences(input, '\n', spaces);
    
    if (spaces.empty()) {
        x_InsertSpaces(input, 0, input.length() - 1, fragment_length);
    }
    else {
        std::sort(spaces.begin(), spaces.end());
        spaces.push_back(input.length() - 1);
        unsigned i = 0;
        size_t start = 0;
        size_t end = 0;
        unsigned insertions = 0;
        do {
            start = end + insertions;
            end = spaces[i++] + insertions;
            if ((end - start + 1) > fragment_length)
                insertions += x_InsertSpaces(input, start + insertions, end + insertions, fragment_length);
        }
        while (i < spaces.size());        
    }
}

CIRef<ITooltipFormatter> CTextTooltipFormatter::CreateTooltipFormatter()
{
    return CIRef<ITooltipFormatter>(new CTextTooltipFormatter());
}

CIRef<ITooltipFormatter> CTextTooltipFormatter::CreateInstance()
{
    return CIRef<ITooltipFormatter>(new CTextTooltipFormatter());
}


void CTextTooltipFormatter::StartRow()
{
}

void CTextTooltipFormatter::AddBulletCol(const string& sBulletSrc)
{
    m_sTooltipText += sBulletSrc.empty() ? "   " : " * ";
}


void CTextTooltipFormatter::AddTagCol(const string& sContents, const string& sBulletSrc)
{
    if (!sBulletSrc.empty())
        m_sTooltipText += " * ";
    m_sTooltipText += sContents + " ";
}

void CTextTooltipFormatter::AddValueCol(const string& sContents, unsigned /*width*/, bool /* isNoWrap */)
{
    m_sTooltipText += sContents;
}

void CTextTooltipFormatter::FinishRow()
{
    m_sTooltipText += "\n";
}

void CTextTooltipFormatter::AddDividerRow(unsigned colspan)
{
    m_sTooltipText += "\n-----------------------\n";
}

void CTextTooltipFormatter::AddRow(const string& sContents, unsigned)
{
    m_sTooltipText += sContents + "\n";
}

void CTextTooltipFormatter::AddRow(const string& sTag, const string& sValue, unsigned, bool)
{
    m_sTooltipText += sTag;
    m_sTooltipText += "\t";
    m_sTooltipText += sValue;
}

void CTextTooltipFormatter::AddLinkRow(const string& sTag, const string& sValue, unsigned valueColWidth)
{
    m_sLinksText += sTag;
    m_sLinksText += "\t";
    m_sLinksText += sValue;
}

void CTextTooltipFormatter::AddLinkRow(const string& sTag, const string& sText, const string& sUrl, unsigned valueColWidth)
{
    m_sLinksText += sTag;
    m_sLinksText += "\t";
    m_sLinksText += CreateLink(sText, sUrl);
}

void CTextTooltipFormatter::AddLinksTitle(const string& sTitle)
{
    m_sLinksText += sTitle + "\n";
}

void CTextTooltipFormatter::AddSectionRow(const string& sContents)
{
    m_sTooltipText += sContents + "\n";
}

string CTextTooltipFormatter::Render()
{
    string sWholeTooltip(m_sTooltipText);
    if (!m_sLinksText.empty()) {
        sWholeTooltip += "\n";
        sWholeTooltip += m_sLinksText;
        m_sLinksText.clear();
    }
    m_sTooltipText.clear();
    return sWholeTooltip;
}

bool CTextTooltipFormatter::IsEmpty() const
{
    return m_sTooltipText.empty();
}


void CTextTooltipFormatter::AddPubmedLinksRow(const string& pmids, bool& isGoToPresent, bool isBulletColPresent)
{
    s_AddPubmedLinksRow(pmids, isGoToPresent, isBulletColPresent, this);
}

string CTextTooltipFormatter::CreateLink(const string& sText, const string& sUrl) const
{
    return sText;
}

string CTextTooltipFormatter::CreateNcbiLink(const string& sText, const string& sUrl) const
{
    return CreateLink(sText, NStr::StartsWith(sUrl, CSeqUtils::GetNcbiBaseUrl())
        ? sUrl : CSeqUtils::GetNcbiBaseUrl() + sUrl);
}

void CTextTooltipFormatter::MaybeAddGoToRow(bool& isGoToPresent, bool isBulletColPresent)
{
    s_MaybeAddGoToRow(isGoToPresent, isBulletColPresent, this);
}

void CTextTooltipFormatter::Append(const ITooltipFormatter& tooltip)
{
    const CTextTooltipFormatter* pTextFormatter = dynamic_cast<const CTextTooltipFormatter*>(&tooltip);
    if (!pTextFormatter)
        return;

    m_sTooltipText += pTextFormatter->m_sTooltipText;
    m_sLinksText += pTextFormatter->m_sLinksText;
}

END_NCBI_SCOPE
