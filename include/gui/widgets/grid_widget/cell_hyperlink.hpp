#ifndef GUI_WIDGETS_GRID_WIDGET__CELL_HYPERLINK__HPP
#define GUI_WIDGETS_GRID_WIDGET__CELL_HYPERLINK__HPP

/*  $Id: cell_hyperlink.hpp 27003 2012-12-06 15:58:20Z katargir $
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

#include <wx/gdicmn.h>

BEGIN_NCBI_SCOPE

class CCellHyperlinks
{
public:
    virtual ~CCellHyperlinks() {}

    class CHyperlink
    {
    public:
        CHyperlink(const wxRect& rect, const wxString& url)
            : m_Rect(rect), m_URL(url) {}

        bool HitTest(const wxPoint& pt) const { return m_Rect.Contains(pt); }
        wxString GetURL() const { return m_URL; }

    private:
        wxRect   m_Rect;
        wxString m_URL;
    };

    const CHyperlink* HitTest(const wxPoint& pt) const;

    bool Empty() const { return m_Links.empty(); }

    void Add(const wxRect& rect, const wxString& url) { m_Links.push_back(CHyperlink(rect, url)); }

private:
    vector<CHyperlink> m_Links;
};


class CGridHyperlinks
{
public:
    ~CGridHyperlinks();

    void SetLink(int col, int row, CCellHyperlinks* links);
    const CCellHyperlinks* GetLink(int col, int row) const;

private:
    class CColHyperlinks
    {
    public:
        ~CColHyperlinks();
        void SetLink(int row, CCellHyperlinks* links);
        const CCellHyperlinks* GetLink(int row) const;

    private:
        vector<CCellHyperlinks*> m_CellLinks;
    };

    vector<CColHyperlinks*> m_ColLinks;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_GRID_WIDGET__CELL_HYPERLINK__HPP
