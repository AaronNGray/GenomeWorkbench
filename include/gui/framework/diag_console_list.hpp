#ifndef GUI_FRAMEWORK___DIAG_CONSOLE_LIST_HPP
#define GUI_FRAMEWORK___DIAG_CONSOLE_LIST_HPP

/*  $Id: diag_console_list.hpp 39929 2017-11-27 21:17:26Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>
#include <deque>

#include <gui/gui_export.h>

#include <gui/objutils/reg_settings.hpp>
#include <gui/utils/command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
#include <wx/listctrl.h>
////@end includes


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleCache
class CDiagConsoleCache
{
public:
    CDiagConsoleCache(size_t size) : m_Size(size), m_AccessTime(0) {}

    int GetItemImage(size_t index);
    wxString GetItemMessage(size_t index);
    wxString GetItemTime(size_t index);
    wxString GetItemSource(size_t index);

private:
    class CItem {
    public:
        unsigned long m_AccessTime = 0;
        int m_Image = -1;
        wxString m_Message;
        wxString m_Time;
        wxString m_Source;
    };

    typedef map<size_t, CItem> TData;

    const CItem& x_GetItem(size_t index);

    const size_t m_Size;
    unsigned long m_AccessTime;
    TData m_Data;
};


///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleList
class NCBI_GUIFRAMEWORK_EXPORT CDiagConsoleList :
    public wxListCtrl,
    public CEventHandler,
    public IRegSettings
{
    DECLARE_EVENT_MAP();
public:
    enum {
        eShowErrors = 1,
        eShowWarnings = 2,
        eShowInfo = 4,
        eShowOther = 8,
        eShowFlags = 0x0F,
        eShowNCBI = 0x10,
        eShowWxWdidgets = 0x20,
        eShowNCBIWxWdidgets = 0x30,
        eShowAll = 0x3F
    };



    CDiagConsoleList();
    CDiagConsoleList(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBORDER_NONE );

    virtual ~CDiagConsoleList();

    /// Creation
    bool Create(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBORDER_NONE );

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    size_t GetFilter() const { return m_Filter; }
    void SetFilter(size_t filter);

    wxString GetItemRawMsg(long item) const;

    void OnContextMenu( wxContextMenuEvent& event );

protected:
    virtual void AppendMenuItems( wxMenu& aMenu );
    bool ShouldPropagateContextMenu() const { return false; }

    void    x_OnNewItem(CEvent*);
    void    x_ReloadMessages();

    virtual int OnGetItemImage(long item) const;
    virtual wxString OnGetItemText(long item, long column) const;

    tuple<size_t, size_t> x_GetIndex(long item) const
    {
        size_t line = 0, index = m_Items[item];
        while (item > 0 && index == m_Items[--item])
            ++line;
        return make_tuple(index, line);
    }

protected:
    typedef deque<size_t> TItems;

    size_t m_Filter;
    string m_RegPath;
    TItems m_Items;
    mutable CDiagConsoleCache m_Cache;
    CFixGenericListCtrl m_FixGenericListCtrl;

    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___DIAG_CONSOLE_LIST_HPP
