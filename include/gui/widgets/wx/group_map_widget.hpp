#ifndef GUI_WIDGETS_WX___GROUP_MAP_WIDGET__HPP
#define GUI_WIDGETS_WX___GROUP_MAP_WIDGET__HPP

/*  $Id: group_map_widget.hpp 25493 2012-03-27 18:28:58Z kuznets $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <gui/widgets/wx/map_item.hpp>
#include <gui/widgets/wx/map_control.hpp>

//#include <gui/utils/event_handler.hpp>

class wxDC;

BEGIN_NCBI_SCOPE

class IMapItem;

///////////////////////////////////////////////////////////////////////////////
/// CGroupMapWidget
class NCBI_GUIWIDGETS_WX_EXPORT CGroupMapWidget :
        public wxPanel
{
    typedef wxPanel TParent;
public:
    /// @name Definitions of data exchange elements
    /// @{
    struct SItemDescr {
        string m_Label;
        string m_IconAlias;
        string m_Description;
        bool   m_ShowDefalt;
    };

    typedef vector<SItemDescr>  TItemDescrVector;

    struct SGroupDescr {
        string  m_Label;
        bool    m_Expanded;
        TItemDescrVector    m_Items;

        SGroupDescr(const string& label)    : m_Label(label), m_Expanded(true) {};
    };

    typedef vector<SGroupDescr> TGroupDescrVector;
    /// @}

    CGroupMapWidget(wxWindow* parent,
                      wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxTAB_TRAVERSAL,
                      const wxString& name = wxT("groupmapwidget"));
    virtual ~CGroupMapWidget();

    virtual void    Init(const TGroupDescrVector& groups);
    virtual void    Clear();

    virtual void    AddGroup(const string& label, bool expand = true);
    virtual void    AddItem(const string& group_label, const string& label,
                            const string& icon_alias, const string& descr,
                            bool showDefault);

    virtual void    GetGroupLabels(vector<string>& groups) const;
    virtual bool    IsExpanded(const string& group) const;

    string          GetDefaultItem() const { return m_DefaultItem; }
    void            SetDefaultItem(const string& label) { m_DefaultItem = label; }

    virtual string  GetSelectedItem() const;
    string  GetHotItem() const;
    string  GetItemLabel(int index) const;

    virtual void    ExpandGroup(const string& label);

    virtual CMapControl*  GetMapControl();

    void   SetInputCategory(const string& category) { m_InputCategory = category; }
    string GetInputCategory() const { return m_InputCategory; }

    /// CGroupItem ////////////////////////////////////////////////////////////
    class NCBI_GUIWIDGETS_WX_EXPORT CGroupItem : public CMapItem
    {
    public:
        enum EDefaultState {
            eDefaultHide,
            eDefaultYes,
            eDefaultNo
        };

        CGroupItem(const string& label, wxBitmap& icon, const string& descr,
                   EDefaultState def,
                   CGroupMapWidget& widget, bool group);

        bool    IsExpanded() const;
        void    Expand(bool exp);

        /// @name IMapItem implementation
        /// @{
        virtual void    Layout(wxDC& dc, SwxMapItemProperties& props);
        virtual int     PreferredHeight(wxDC& dc, SwxMapItemProperties& props, int width);
        virtual void    Draw(wxDC& dc, int state, SwxMapItemProperties& props);
        virtual void    OnDefaultAction();
        virtual bool    OnHotTrack(const wxPoint& ms_pos);
        virtual void    OnMouseDown(const wxPoint& ms_pos);
        virtual void    OnLeftDoubleClick(const wxPoint& ms_pos);
        /// @}

        EDefaultState   GetDefaultState() const { return m_DefaultState; }
        void            SetDefaultState(EDefaultState state) { m_DefaultState = state; }

        string          GetFullDescr() const;

    protected:
        virtual int     x_GetItemShift() const;
        virtual int     x_PreferredSeparatorHeight(wxDC& dc) const;

        virtual void    x_DrawBackground(wxDC& dc, const wxRect& rc, int state, SwxMapItemProperties& props);

    protected:
        CGroupMapWidget& m_Widget;
        bool     m_Expanded;
        bool     m_HotLabel;
        int      m_LabelRealW;
        bool     m_HotDef;
        int      m_DefWidth;
        EDefaultState m_DefaultState;
        bool     m_ClickOnSpace;
    };

    // Current - Hot or Selected item
    CConstRef<CGroupItem> GetCurrentItem() const;

    friend class CGroupMapWidget::CGroupItem;
    
    typedef CMapControl::TItemRef TItemRef;
    typedef vector<TItemRef>   TMapItems;

protected:
    struct SGroup  {
        CRef<CGroupMapWidget::CGroupItem>    m_GroupItem;
        TMapItems   m_Items;
    };

    typedef vector<SGroup*> TGroups;

    int     x_GetGroupIndex(const string& label) const;

    void    x_OnToggleGroup(CGroupItem& item);
    void    x_OnSelectItem(CGroupItem& item);
    void    x_OnMakeDefaultItem(CGroupItem& item);

    TGroups m_Groups;

    CMapControl*    m_Control;

    wxBitmap m_OpenIcon;
    wxBitmap m_ClosedIcon;

    string   m_InputCategory;
    string   m_DefaultItem;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___GROUP_MAP_WIDGET__HPP
