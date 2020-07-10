/*  $Id: str_url_parser.cpp 27109 2012-12-20 15:42:17Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/grid_widget/str_url_parser.hpp>

BEGIN_NCBI_SCOPE

CStrWithUrlContainer::~CStrWithUrlContainer()
{
    ITERATE(vector<TLine>, it, m_Lines) {
        ITERATE(TLine, it2, *it) {
            delete *it2;
        }
    }
}

void CStrWithUrlContainer::AddText(CStrWithUrlText* text)
{
    if (m_Lines.empty()) {
        NewLine();
    }
    m_Lines.back().push_back(text);
}


void CStrWithUrlContainer::NewLine()
{
    m_Lines.push_back(TLine());
}

namespace {

class CTagHandler : public wxHtmlTagHandler
{
public:
    wxString GetSupportedTags() { return wxT("A,BR"); }
    bool HandleTag(const wxHtmlTag& tag);
};

bool CTagHandler::HandleTag(const wxHtmlTag& tag)
{
    CStrWithUrlParser* parser = dynamic_cast<CStrWithUrlParser*>(m_Parser);
    CStrWithUrlContainer* container = parser->GetContainer();

    if (tag.GetName() == wxT("BR")) {
        container->NewLine();
    }
    else if (tag.GetName() == wxT("A")) {
        wxString text(tag.GetBeginIter(), tag.GetEndIter1()), url;
        if (tag.HasParam(wxT("HREF"))) {
            url = tag.GetParam(wxT("HREF"));
        }
        container->AddText(new CStrWithUrlLink(text, url));
    }
    return true;
}

} // namespace

CStrWithUrlParser::CStrWithUrlParser() : m_Parsed()
{
    AddTagHandler(new CTagHandler);
}

void CStrWithUrlParser::AddText(const wxString& txt)
{
    m_Parsed->AddText(new CStrWithUrlText(txt));
}

wxObject* CStrWithUrlParser::GetProduct()
{
    return m_Parsed;
}

void CStrWithUrlParser::InitParser(const wxString& source)
{
    wxHtmlParser::InitParser(source);
    m_Parsed = new CStrWithUrlContainer();
}

void CStrWithUrlParser::DoneParser()
{
    m_Parsed = NULL;
    wxHtmlParser::DoneParser();
}

namespace {

class CStrWithUrlStripParser : public wxHtmlParser
{
public:
    CStrWithUrlStripParser();
    wxString& GetText() { return m_Text; }
    virtual void AddText(const wxString& txt) { m_Text += txt; }
    virtual wxObject* GetProduct() { return 0; }

protected:
    wxString m_Text;
};

class CTagStripHandler : public wxHtmlTagHandler
{
public:
    wxString GetSupportedTags() { return wxT("A,BR"); }
    bool HandleTag(const wxHtmlTag& tag)
    {
        CStrWithUrlStripParser* parser = dynamic_cast<CStrWithUrlStripParser*>(m_Parser);
        if (tag.GetName() == wxT("BR")) {
            parser->GetText() += wxT('\n');
        }
        else if (tag.GetName() == wxT("A")) {
            parser->GetText() += wxString(tag.GetBeginIter(), tag.GetEndIter1());
        }
        return true;
    }
};

CStrWithUrlStripParser::CStrWithUrlStripParser()
{
    AddTagHandler(new CTagStripHandler);
}

} // namespace

void CStrWithUrlParser::StripHtml(wxString& text)
{
    wxString::size_type pos = text.find(wxT('<'));
    if (pos == wxString::npos)
        return;

    pos = text.find(wxT('>'), pos + 1);
    if (pos == wxString::npos)
        return;

    CStrWithUrlStripParser parser;
    parser.Parse(text);
    text = parser.GetText();
}


END_NCBI_SCOPE
