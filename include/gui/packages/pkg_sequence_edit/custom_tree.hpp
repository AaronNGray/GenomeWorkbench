/*  $Id: custom_tree.hpp 42739 2019-04-08 19:21:14Z kachalos $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Sema
*/

#include <corelib/ncbistd.hpp>
#include <wx/window.h>

BEGIN_NCBI_SCOPE

class CCustomTree;

class CCustomTreeItem
{
public:
    virtual string GetText() const = 0;
    size_t GetLevel() const { return m_Level; }
    bool IsExpanded() const { return m_Expanded; }
    bool IsChecked() const { return m_Checked; }
    bool IsActive() const { return m_Active; }
    vector<CCustomTreeItem*> GetChildren() { return m_List; }
protected:
    CCustomTreeItem(CCustomTreeItem* parent, bool x = false, bool c = false, bool a = false) :
        m_Parent(parent), m_Level(parent ? parent->m_Level + 1 : 0), m_Expanded(x), m_Checked(c), m_Active(a) {}
    virtual ~CCustomTreeItem();
    vector<CCustomTreeItem*> m_List;
    CCustomTreeItem* m_Parent;
    size_t m_Level;
    bool m_Expanded;
    bool m_Checked;
    bool m_Active;

friend class CCustomTree;
};


class CCustomTree : public wxWindow
{
    DECLARE_EVENT_TABLE()
public:
    CCustomTree(wxWindow* parent, wxWindowID id, bool drawcheckboxes, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr) :
        wxWindow(parent, id, pos, size, style, name), m_DrawCB(drawcheckboxes), m_Current(0), m_ScrollBar(false), m_Start(0), m_Fullheight(0), m_CB_width(0), m_CB_height(0), m_SB_width(0), m_LN_height(0), m_MIN_width(0), m_MIN_height(0) {}
    ~CCustomTree() { Clear(); }
    void Clear();
    void Add(CCustomTreeItem* item) { m_List.push_back(item); }
    void Finish() { CollectAll(); }
    void CheckAll(bool check) { for (size_t i = 0; i < m_List.size(); i++) CheckItem(m_List[i], check); }
    void UpdateScrollbar();
    CCustomTreeItem* GetCurrentItem() { return m_Current; }
    void SetCurrentItem(CCustomTreeItem* p);
    vector<CCustomTreeItem*> GetRootItems() { return m_List; }
    vector<CCustomTreeItem*> GetAllItems() { return m_All; }
    vector<CCustomTreeItem*> GetVisibleItems() { return m_Visible; }
    void ExpandItem(CCustomTreeItem* item, bool expand);
    void CheckItem(CCustomTreeItem* item, bool check, bool propagate = true);

protected:
    void Paint(wxDC& dc);
    void OnPaint(wxPaintEvent & evt);
    void OnSize(wxSizeEvent& evt){ m_Fullheight = 0; UpdateScrollbar(); Refresh(); }
    void OnScroll(wxScrollWinEvent& evt);
    void OnWheel(wxMouseEvent& evt);
    void OnSetFocus(wxFocusEvent& evt);
    void OnKillFocus(wxFocusEvent& evt);
    void OnClick(wxMouseEvent& evt);
    void OnDblClick(wxMouseEvent& evt);
    void GetMetrix() { if (!m_CB_width) QueryMetrix(); }
    void CollectAll();
    void CollectAll(CCustomTreeItem* item);
    void CollectVisible();
    void CollectVisible(CCustomTreeItem* item);
    void QueryMetrix();
    static vector<string> Split(string str, wxDC& dc, int width);
    int FullHeight();
    int ItemHeight(const CCustomTreeItem& item, wxDC& dc, int width);
    void DrawExpander(wxDC& dc, wxRect& rect, bool open, bool highlited);  // wxWidgets 3.1.0 has a native function for this; can remove it after the upgrade
    void PropagateCheck(CCustomTreeItem* item, bool check);
    void CheckParent(CCustomTreeItem* item, bool check);

    vector<CCustomTreeItem*> m_List;
    vector<CCustomTreeItem*> m_All;
    vector<CCustomTreeItem*> m_Visible;
    CCustomTreeItem* m_Current;
	bool m_DrawCB;
    bool m_ScrollBar;
    int m_Start;
    int m_Fullheight;
    int m_CB_width;     // checkbox
    int m_CB_height;
    int m_SB_width;     // scrollbar
    int m_LN_height;    // text
    int m_MIN_width;    // text
    int m_MIN_height;   // text
};


END_NCBI_SCOPE
