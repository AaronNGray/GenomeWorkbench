#ifndef GUI_WIDGETS_GRID_WIDGET__STR_URL_PARSER__HPP
#define GUI_WIDGETS_GRID_WIDGET__STR_URL_PARSER__HPP

/*  $Id: str_url_parser.hpp 27109 2012-12-20 15:42:17Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <wx/html/htmlpars.h>

BEGIN_NCBI_SCOPE

class CStrWithUrlText
{
public:
    CStrWithUrlText(const wxString& text) : m_Text(text) {}
    virtual ~CStrWithUrlText() {}

    wxString GetText() const { return m_Text; }
private:
    wxString m_Text;
};

class CStrWithUrlLink : public CStrWithUrlText
{
public:
    CStrWithUrlLink(const wxString& text, const wxString& url)
        : CStrWithUrlText(text), m_Url(url) {}

    wxString GetUrl() const { return m_Url; }
private:
    wxString m_Url;
};

class CStrWithUrlContainer : public wxObject
{
public:
    ~CStrWithUrlContainer();

    void AddText(CStrWithUrlText* text);
    void NewLine();

    typedef vector<CStrWithUrlText*> TLine;

    const vector<TLine>& GetLines() const { return m_Lines; }

private:
    vector<TLine> m_Lines;
};

class CStrWithUrlParser : public wxHtmlParser
{
public:
    CStrWithUrlParser();

    CStrWithUrlContainer* GetContainer() { return m_Parsed; }

    virtual void AddText(const wxString& txt);

    virtual void InitParser(const wxString& source);
    virtual void DoneParser();
    virtual wxObject* GetProduct();

    static void StripHtml(wxString& text);

protected:
    CStrWithUrlContainer* m_Parsed;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_GRID_WIDGET__STR_URL_PARSER__HPP
