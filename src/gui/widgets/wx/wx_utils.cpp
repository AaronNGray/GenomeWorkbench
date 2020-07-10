/*  $Id: wx_utils.cpp 44433 2019-12-18 15:53:19Z shkeda $
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
#include <ncbi_pch.hpp>

#include <util/checksum.hpp>

#include <gui/utils/string_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/text_report_dlg.hpp>

#include <gui/utils/menu_item.hpp>
#include <gui/utils/rgba_color.hpp>

#include <connect/ncbi_conn_test.hpp>
#include <corelib/ncbiexec.hpp>
#include <corelib/ncbi_system.hpp>

#include <wx/app.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/colordlg.h>
#include <wx/dc.h>
#include <wx/listctrl.h>
#include <wx/frame.h>
#include <wx/filename.h>

#include <wx/renderer.h>
#include <wx/dcmemory.h>
#include <wx/imaglist.h>

#ifdef __WXMAC__
#import <Cocoa/Cocoa.h>
#endif

#ifdef __WXMAC_CARBON__
#include <Carbon/carbon.h>
#endif

#include <wx/display.h>


BEGIN_NCBI_SCOPE

wxString FindExeFile(const wxString& exeFile)
{
    wxString notFound;

    if (exeFile.IsEmpty())
        return notFound;
    
    if (::wxIsAbsolutePath(exeFile)) {
        if (::wxFileExists(exeFile))
            return exeFile;
        return notFound;
    }

    wxFileName fileName(exeFile);
    if (!fileName.GetPath().IsEmpty())
        return notFound;

    wxPathList pathList;
    pathList.AddEnvList(wxT("PATH"));
    return pathList.FindValidPath(exeFile);
}


void ReportIDError(const string& id_label, bool is_local, const string& title)
{
    string str("<html><body>Failed to retrieve sequence for id ");
    str += id_label;
    if (is_local) {
        str += "<br><br>";
        str += "For successful data display Genome Workbench requires that annotations and the reference sequences are imported into the same project.";
        str += "<br><br>";
        str += "For more details: <a href=https://www.ncbi.nlm.nih.gov/tools/gbench/tutorial27/>https://www.ncbi.nlm.nih.gov/tools/gbench/tutorial27/</a>";
    }
    str += "</body></html>";
    CTextReportDlg dlg(NULL, wxID_ANY, ToWxString(title));
    dlg.SetText(ToWxString(str));
    dlg.ShowModal();

}

void ReportMemoryUsage()
{
    CCurrentProcess::SMemoryUsage usage;
    if (CCurrentProcess::GetMemoryUsage(usage)) {
        static int reported = -1;
        int used = usage.resident / 1024 / 1024;
        if (reported != used) {
            reported = used;
            LOG_POST(Info << "Memory used: " << reported << "MB");
        }
    }
}

void OpenFileBrowser(const wxString& path)
{
    wxFileName fileName(path);
    fileName.MakeAbsolute();

    string file(fileName.GetFullPath().ToUTF8());
    string dir(fileName.GetPathWithSep().ToUTF8());

#ifdef __WXMSW__
    CExec::SpawnLP(CExec::eNoWait, "explorer", "/select,", file.c_str(), nullptr);
#elif defined(__WXOSX__)
    CExec::SpawnLP(CExec::eNoWait, "open", "-R", file.c_str(), nullptr);
#elif defined(NCBI_OS_LINUX)
    try { // Cinnamon
        CExec::SpawnLP(CExec::eNoWait, "nemo", file.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    try { // GNOME
        CExec::SpawnLP(CExec::eNoWait, "nautilus", "--new-window", file.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    try { // KDE
        CExec::SpawnLP(CExec::eNoWait, "dolphin", "--select", file.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    try { // XFCE
        CExec::SpawnLP(CExec::eNoWait, "thunar", dir.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    try { // MATE
        CExec::SpawnLP(CExec::eNoWait, "caja", "--browser", dir.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    try { // LXDE
        CExec::SpawnLP(CExec::eNoWait, "pcmanfm", "-n", dir.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    try { // KDE
        CExec::SpawnLP(CExec::eNoWait, "konqueror", "--select", file.c_str(), nullptr);
        return;
    }
    catch (const CExecException&) {}

    CExec::SpawnLP(CExec::eNoWait, "xdg-open", dir.c_str(), nullptr);
#endif
}

string GetMD5Digest(const CChecksum& cs)
{
    string result;
    unsigned char digest[16];
    cs.GetMD5Digest(digest);
    for (int i = 0; i < 16; ++i) {
        char buf[4];
        sprintf(buf, "%02X", (int)digest[i]);
        result += buf;
    }
    return result;
}

string GetVideoId()
{
    CChecksum cs(CChecksum::eMD5);

    unsigned displays = wxDisplay::GetCount();
    _TRACE("Displays: " << displays);
    string str = NStr::Int8ToString((int)displays);
    cs.AddLine(str);

    for (unsigned i = 0; i < displays; ++i) {
        wxDisplay display(i);
        wxRect rect = display.GetGeometry();

        _TRACE("Display" << i << ": " << rect.x << "," << rect.y
                              << " " << rect.width << "x" << rect.height);

        str = NStr::Int8ToString(rect.x);
        cs.AddLine(str);
        str = NStr::Int8ToString(rect.y);
        cs.AddLine(str);
        str = NStr::Int8ToString(rect.width);
        cs.AddLine(str);
        str = NStr::Int8ToString(rect.height);
        cs.AddLine(str);
    }

    return GetMD5Digest(cs);
}

wxString FnToWxString(const string& s)
{
    if (s.empty())
        return wxEmptyString;

    string decoded = s;
    size_t src_size = s.size();
    if (src_size > 2 && s[0] == '[' && s[src_size - 1] == ']')
        decoded = CStringUtil::base64Decode(s.substr(1, src_size-2));
    else
        decoded = s;

    return wxString::FromUTF8(decoded.c_str());
}

string FnToStdString(const wxString& s)
{
    if (s.empty())
        return "";

    if (s.IsAscii())
        return string(s.ToAscii());

    return "[" + CStringUtil::base64Encode(string(s.ToUTF8())) + "]";
}

/// converts an absolute path to a relative one based on current workspace
/// directory
wxString ToRelativePath(const wxString& base, const wxString& abs_path)
{
    if (abs_path.StartsWith(wxT("http://")) || abs_path.StartsWith(wxT("https://")) || abs_path.StartsWith(wxT("ftp://")))
        return abs_path;

    wxFileName absolute(abs_path);
    if (absolute.IsRelative())
        return abs_path;

    if (base.size() != 0) {
        wxString ws_dir = wxFileName(base).GetPath();
        wxFileName relative(abs_path);
        relative.MakeRelativeTo(ws_dir);
        return relative.GetFullPath();
    }
    return abs_path;
}

/// converts relative path from the current workspace directory to an
/// absolute path
wxString ToAbsolutePath(const wxString& base, const wxString& rel_path)
{
    if (rel_path.StartsWith(wxT("http://")) || rel_path.StartsWith(wxT("https://")) || rel_path.StartsWith(wxT("ftp://")))
        return rel_path;

    wxFileName absolute(rel_path);
    if (absolute.IsAbsolute())
        return rel_path;

    if (base.size() != 0) {
        wxString ws_dir = wxFileName(base).GetPath();
        absolute.MakeAbsolute(ws_dir);
        return absolute.GetFullPath();
    }

    return rel_path;
}


wxFileArtProvider* s_FileArtProvider = NULL;
bool sf_FileArtProviderPushed = false;

void InitDefaultFileArtProvider( const wxString& dir )
{
    if( !sf_FileArtProviderPushed ){
        wxArtProvider::Push( GetDefaultFileArtProvider() );
        sf_FileArtProviderPushed = true;
    }

    GetDefaultFileArtProvider()->AddDirectory( dir );
}


wxFileArtProvider* GetDefaultFileArtProvider()
{
    if( s_FileArtProvider == NULL ){
        s_FileArtProvider = new wxFileArtProvider();
    }
    return s_FileArtProvider;
}


void FromArrayString( const wxArrayString& in, vector<string>& out )
{
    size_t n = in.GetCount();
    out.reserve(n);
    for( size_t i = 0; i < n; i++ ){
        out.push_back( ToStdString(in[i]) );
    }
}


void ToArrayString( const vector<string>& in, wxArrayString& out )
{
    size_t n = in.size();
    out.Alloc(n);
    for( size_t i = 0; i < n; i++ ){
        out.Add( ToWxString(in[i]) );
    }
}


\
// creates wxWidgets menu from a hierarchy of CMenuItems
wxMenu* CreateMenu( const CMenuItem* item )
{
    if( item && item->IsSubmenu() ){
        wxMenu* menu = new wxMenu();

        for( CMenuItem::TChildItem_CI it =  item->SubItemsBegin();
            it != item->SubItemsEnd(); 
            ++it
        ){

            CMenuItem& ch_item = *(*it)->GetValue();
            wxString wx_label = ToWxString(ch_item.GetLabel());
            CMenuItem::EType type = ch_item.GetType();

            wxMenuItem* wx_item = NULL;

            if( ch_item.IsSubmenu() ){
                // add a submenu
                wxMenu* ch_menu = CreateMenu(&ch_item);
                _ASSERT(ch_menu);
                menu->Append(wxID_ANY, wx_label, ch_menu);
            } else {
                // add a regular menu item
                int item_id = ch_item.GetCommand();
                wxItemKind kind = wxITEM_NORMAL;
                wxString wx_help = ToWxString(ch_item.GetTooltip());

                if(type == CMenuItem::eItem)   {
                    if(ch_item.IsRadioType())   {
                        kind = wxITEM_RADIO;
                    } else if(ch_item.IsCheckType())    {
                        kind = wxITEM_CHECK;
                    }
                } else {
                    _ASSERT(type == CMenuItem::eSeparator);
                    item_id = wxID_SEPARATOR;
                    kind = wxITEM_SEPARATOR;
                }

                wx_item = new wxMenuItem(menu, item_id, wx_label, wx_help, kind);

                // add an icon (if any)
                const string& icon = ch_item.GetImageAlias();
                if( ! icon.empty()) {
                    wxString wx_alias = ToWxString(icon);
                    wxBitmap bmp = wxArtProvider::GetBitmap(wx_alias);
                    if(bmp.IsOk())   {
                        wx_item->SetBitmap(bmp);
                        SetMenuItemMarginWidth(wx_item, &bmp);
                    }
                }

                menu->Append(wx_item);
            }
        }
        return menu;
    }
    return NULL;
}


wxMenuItem* FindSubItem( wxMenu& menu, const wxString& text )
{
    wxMenuItemList& items = menu.GetMenuItems();
    ITERATE( wxMenuItemList, it, items ){
        wxMenuItem* item = *it;
        if( item->GetItemLabel() == text ){
            return item;
        }
    }

    return NULL;
}


static wxMenuItemList::const_iterator s_FindSeparator( 
    wxMenu& menu, const wxString& text 
){
    wxMenuItemList& items = menu.GetMenuItems();
    ITERATE( wxMenuItemList, it, items ){
        wxMenuItem* item = *it;
        if( item->IsSeparator() && item->GetItemLabel() == text ){
            return it;
        }
    }

    return items.end();
}


static wxMenuItemList::const_iterator s_FindSubItem( 
    wxMenuItemList::const_iterator begin,
    wxMenuItemList::const_iterator end,
    const wxString& text,
    bool skip_named_groups
){
    bool named_group = false;
    for( wxMenuItemList::const_iterator it = begin; it != end; it++ ){
        wxMenuItem* sub_item = *it;
        wxString s = sub_item->GetItemLabel();

        if( skip_named_groups ){
            if( sub_item->IsSeparator() ){
                named_group = !s.empty();

            } else if ( 
                !(named_group  &&  skip_named_groups)  
                && (s == text)
            ){
                return it;
            }
        } else if( s == text ){
            return it;
        }
    }
    return end;
}


static void s_InsertInUnnamedGroup( wxMenu& menu, wxMenuItem& item )
{
    bool named = false;
    // find the last separator in the menu
    wxMenuItemList& items = menu.GetMenuItems();
    ITERATE( wxMenuItemList, it, items ){
        const wxMenuItem* it_item = *it;
        if( it_item->IsSeparator() ){
            named = ! it_item->GetItemLabel().empty();
            break;
        }
    }

    if( named ){
        // add an unnamed separator to terminate the previous group
        menu.Append( new wxMenuItem( &menu, wxID_SEPARATOR )); 
    }
    menu.Append( &item );
}


static wxString s_GetCleanMenuText( const wxMenuItem& item )
{
    wxString s = item.GetItemLabel();

#ifdef __WXGTK__
    for (size_t i = 0;  i < s.size();  ++i) {
        if (s[i] == '_') {
            s[i] = '&';
        }
    }
#endif

    return s;
}


static wxMenuItem* s_CloneMenuItem( wxMenu& menu, const wxMenuItem& item )
{
    wxString s = s_GetCleanMenuText( item );
    wxMenu* sub_menu = item.GetSubMenu();
    wxMenu* sub_menu_clone = sub_menu ? CloneMenu(*sub_menu) : NULL;

    wxMenuItem* clone =
        wxMenuItem::New(&menu, item.GetId(), s, item.GetHelp(),item.GetKind(), sub_menu_clone);

    // Attempt to avoid bug in wxwidgets 2.9
    // causing improper error log posts
    // 'SetMenuItemInfo' failed with error 0x000005b0 (a menu item was not found.)
#if wxUSE_OWNER_DRAWN
    clone->SetOwnerDrawn();
#endif
    
    // #### GTK hack
    //clone->Enable(item.IsEnabled());

    // item.GetKind() != wxITEM_RADIO - a workaround to prevent an assert
    // in wxMenuItem::Check. The function expects item to exist in its parent
    // menu items but this item doesn't.
#ifndef __WXGTK__
    if( item.IsCheckable() && item.GetKind() != wxITEM_RADIO ){
        clone->Check( item.IsChecked() );
    }
#endif

    wxBitmap bitmap = item.GetBitmap();
    if( bitmap.IsOk() ){
        clone->SetBitmap( bitmap );
        SetMenuItemMarginWidth( clone, &bitmap );
    }

    return clone;
}


wxMenu* CloneMenu( const wxMenu& menu )
{
    wxMenu* new_menu = new wxMenu( menu.GetTitle() );

    const wxMenuItemList& items = menu.GetMenuItems();
    ITERATE( wxMenuItemList, it, items ){
        const wxMenuItem* item = *it;

        wxMenuItem* new_item = s_CloneMenuItem( *new_menu, *item );
        new_menu->Append( new_item );
    }

    return new_menu;
}


/// This function merges the content of menu_2 into menu_1 based on
/// equivalence of item labels
/// Menu trees are merged based on item's labels. Labels specify unique path
/// to an item (similarly to paths in file systems).
void Merge( wxMenu& menu_1, const wxMenu& menu_2 )
{
    typedef wxMenuItemList::const_iterator TIter;

    bool named_group = false;
    wxMenuItemList& items_1 = menu_1.GetMenuItems();

    // [grp_begin, grb_end) defines a range of items in menu_1 that represent
    // the current named group (starting from the named separator)
    TIter grp_begin = items_1.begin();
    TIter grp_end = items_1.end();

    const wxMenuItemList& items_2 = menu_2.GetMenuItems();
    ITERATE( wxMenuItemList, it_2, items_2 ){

        const wxMenuItem* sub_item = *it_2;
        wxString label = sub_item->GetItemLabel();

        if( sub_item->IsSeparator() ){
            // separator indicates end of the current group and beginning of a new one
            named_group = !label.empty();
            if( named_group ){
                // find this group in this item
                grp_begin = s_FindSeparator( menu_1, label );
                if( grp_begin != items_1.end() ){
                    // find the end of the group
                    grp_end = grp_begin;
                    for( grp_end++;  grp_end != items_1.end();  grp_end++ ){
                        const wxMenuItem* it_item = *grp_end;
                        if( it_item->IsSeparator() ){
                            break;
                        }
                    }
                } else {
                    //the group does not exist - create a new one
                    wxString clean_label = s_GetCleanMenuText( *sub_item );
                    menu_1.Append( wxID_SEPARATOR, clean_label );
                    grp_end = grp_begin = s_FindSeparator( menu_1, clean_label );
                    grp_end++;
                }
            } else {
                grp_begin = items_1.begin();
                grp_end = items_1.end();
            }
        } else {
            // not a separator - item that potentially can be merged
            // find candidate within the group
            TIter it_target = s_FindSubItem( grp_begin, grp_end, label, named_group );

            if( it_target != grp_end ){
                // found equivalent items
                wxMenuItem* target_sub_item = *it_target;
                wxMenu* sub_menu_1 = target_sub_item->GetSubMenu();
                wxMenu* sub_menu_2 = sub_item->GetSubMenu();

                if( sub_menu_1 && sub_menu_2 ){
                    // both items are submenus - merge them recursively
                    Merge( *sub_menu_1, *sub_menu_2 );
                }
            } else {
                // did not find a target - add a new item
                wxMenuItem* clone = s_CloneMenuItem( menu_1, *sub_item );

                if( named_group ){
                    _ASSERT( grp_begin != items_1.end() );
                    if( grp_end != items_1.end() ){
                        // insert the new item into this menu after "grp_end" position
                        wxMenuItem* sub_item = *grp_end;
                        for( size_t pos = 0;  pos < menu_1.GetMenuItemCount(); pos++ ){
                            wxMenuItem* pos_item = menu_1.FindItemByPosition( pos );
                            if( pos_item == sub_item ){
                                menu_1.Insert( pos, clone );
                                //grp_end = grp_end->GetNext();
                                break;
                            }
                        }
                    } else {
                        menu_1.Append( clone );
                    }
                } else {
                    // insert the item into menu_1, may need to insert an unnamed separator
                    s_InsertInUnnamedGroup( menu_1, *clone );
                }
            }
        }
    }
}


void CleanupSeparators( wxMenu& menu )
{
    for( size_t i = 0;  i < menu.GetMenuItemCount(); ){
        wxMenuItem* item = menu.FindItemByPosition(i);
        bool remove = false;
        if( item->IsSeparator() ){
            if( i == 0 || i == menu.GetMenuItemCount() - 1 ){
                // no separators allowed at the beginning or at then end of a menu
                remove = true;
            } else {
                wxMenuItem* next_item = menu.FindItemByPosition( i + 1 );
                // if the next item it separator too - delete this one
                remove = next_item->IsSeparator();
            }
        } else if( item->IsSubMenu() ){
            CleanupSeparators( *item->GetSubMenu() );
        }
        if( remove ){
            menu.Remove( item );
        } else {
            i++;
        }
    }
}

void UseDefaultMarginWidth( wxMenu& menu )
{
#ifdef NCBI_OS_MSWIN
#if wxUSE_OWNER_DRAWN
    // fix a bug in wxWidgets on Windows (must be applied on first item)
    // adds a dummy item to resets default menuitem margin
    // widths and destroy it immediately
    wxMenuItem* item;
    item = menu.Append( 20000, wxT("Dummy") );
    item->SetMarginWidth( item->GetDefaultMarginWidth() );
    menu.Destroy( item );
#endif
#endif
}


void SetMenuItemMarginWidth( wxMenuItem* item, wxBitmap * bmp )
{
#ifdef NCBI_OS_MSWIN
    item->SetMarginWidth( bmp->GetWidth() );
#endif
}

BEGIN_EVENT_TABLE( CCommandToFocusHandler, wxEvtHandler )
    EVT_MENU( wxID_ANY, CCommandToFocusHandler::OnMenuEvent )
    EVT_UPDATE_UI( wxID_ANY, CCommandToFocusHandler::OnUpdateUIEvent )
END_EVENT_TABLE()

void CCommandToFocusHandler::OnCommandEvent( wxCommandEvent& event )
{
    if( mf_Reentry ){
        event.Skip();
        return;
    }
    CBoolGuard _guard( mf_Reentry );

    wxWindow* focused = wxWindow::FindFocus();

    if( m_Window != NULL ){
        wxWindow* wid = focused;
        while( wid != NULL ){
            if( wid == m_Window ){
                break;  
            }
            wid = wid->GetParent();
        }

        if( wid == NULL ){
            focused = NULL;
        }
    }

    if( focused == NULL ){
        event.Skip();
        return;
    }

    focused->GetEventHandler()->
#      if( wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9 )
          wxEvtHandler::ProcessEvent( event )
#      else
          ProcessEvent( event )
#      endif
    ;

    //! the trick above is against v2.8 bug in wxScrollHelperEvtHandler::ProcessEvent()
    //! It un-skips unprocessed command events unconditionally, which is wrong
    //! They fixed it in in v2.9.
}

void CCommandToFocusHandler::OnMenuEvent( wxCommandEvent& event )
{
    OnCommandEvent( event );
}

void CCommandToFocusHandler::OnUpdateUIEvent( wxUpdateUIEvent& event )
{
    OnCommandEvent( event );
}

string ToString( const wxRect& rc )
{
    string s = "x = " + NStr::IntToString(rc.x);
    s += ", y = "  + NStr::IntToString(rc.y);
    s += ", w = "  + NStr::IntToString(rc.width);
    s += ", h = "  + NStr::IntToString(rc.height);
    return s;
}


wxRect GetScreenRect( const wxWindow& win )
{
    wxRect rc = win.GetRect();
    if( win.GetParent() ){
        wxPoint pos = rc.GetLeftTop();
        wxPoint screen_pos = win.GetParent()->ClientToScreen( pos );
        rc.SetLeftTop( screen_pos );
    }
    return rc;
}

void WindowSetText(const wxWindow& win, const wxChar* text)
{
#if defined(NCBI_OS_MSWIN)
    WXHWND hwnd = win.GetHWND();
    SetWindowText((HWND)hwnd, text);
#endif
}


static unsigned char s_Average( unsigned char c1, unsigned char c2, double ratio )
{
    double res = ratio * c1 + (1.0 - ratio) * c2;
    return (unsigned char) res;
}


wxColour GetAverage( const wxColor& c1, const wxColor& c2, double ratio )
{
    unsigned char r = s_Average( c1.Red(), c2.Red(), ratio );
    unsigned char g = s_Average( c1.Green(), c2.Green(), ratio );
    unsigned char b = s_Average( c1.Blue(), c2.Blue(), ratio );
    unsigned char a = s_Average( c1.Alpha(), c2.Alpha(), ratio );
    return wxColour( r, g, b, a );
}


int TruncTextLength( wxDC& dc, const wxString& s, int w, Ewx_Truncate trunc )
{
    static wxString sc_ellipsis(wxT("..."));

    wxSize sz = dc.GetTextExtent(s);
    int full_w = sz.x;
    int n = (int)s.size();

    if( full_w <= w && trunc != ewxTruncate_EllipsisAlways ){
        // the text fits in the space provided
        return n;
    } else {
        int w_av = 0; // width available
        int len = n;

        switch( trunc ){
        case ewxTruncate_Empty:
            w_av = w;
            break;
        case ewxTruncate_Ellipsis:
        case ewxTruncate_EllipsisAlways:
            wxSize el_sz = dc.GetTextExtent(sc_ellipsis);
            w_av = w - el_sz.x;
            break;
        }

        if( w_av > 0 ){
            // adjust string length
            wxArrayInt widths;
            dc.GetPartialTextExtents(s, widths);

            // approximate start position
            double K = ((double) w_av) / full_w;
            len = (int) (K * n);
            len = min(len, n - 1);
            int new_w = widths[len];

            if(new_w > w_av)    {
                for( ; new_w > w_av  &&  --len > 0; ){
                    new_w = widths[len];
                }
            } else {
                for( ; new_w < w_av  &&  ++len < n; ){
                    new_w = widths[len];
                }
                if( new_w > w_av ){
                    len--;
                }
            }
        } else {
            len = 0;
        }

        return len;
    }
}


wxString TruncateText( wxDC& dc, const wxString& s, int w, Ewx_Truncate trunc )
{
    static wxString sc_ellipsis(wxT("..."));

    int len = TruncTextLength(dc, s, w, trunc);
    int n = (int)s.size();

    wxString res(s, len);
    if((trunc == ewxTruncate_Ellipsis  &&  len < n)  || (trunc == ewxTruncate_EllipsisAlways)) {
        res += sc_ellipsis;
    }
    return res;
}


/// divides given "text" into lines, so that every line has width less or equal
/// to "w". Returns length of lines in "line_lens".
void  WrapText(wxDC& dc, const string& text, int w, vector<int>& line_lens)
{
    static string spaces(" \t\n");

    int text_len = (int)text.size();
    const char* s = text.c_str();
    for( int pos = 0; pos < text_len; )   {
        int line_start = pos;
        // search for the end of the first word
        string::size_type after_word_end =
            text.find_first_of(spaces, line_start);
        if(after_word_end == string::npos)    {
            after_word_end = text_len;
        }
        int line_len = (int)(after_word_end - line_start);

        int t_h, line_w;
        dc.GetTextExtent(ToWxString(s + line_start, line_len), &line_w, &t_h);

        //int line_w = (int) ceil(fl_width(s + line_start, line_len));

        if(line_w < w)   {
            /// the first word fits in "w", let see if we can fit more words here
            int p = (int)after_word_end; // pointer to next character
            for( ;  p < text_len;  p++  )    {
                if(spaces.find(s[p]) != string::npos) { // space character

                    wxString wx_s = ToWxString(s + line_start, p - line_start);
                    int cw = 0;
                    dc.GetTextExtent(wx_s, &cw, &t_h);
                    //int cw = (int) ceil(fl_width(s + line_start, p - line_start));
                    if(cw > w) {
                        break;
                    }
                } else {
                    // p point to a non-space char, skip to the end of word
                    after_word_end = text.find_first_of(spaces, p);
                    if (after_word_end == string::npos)    {
                        after_word_end = text_len;
                    }
                    wxString wx_s = ToWxString(s + line_start, after_word_end - line_start);
                    int cw = 0;
                    dc.GetTextExtent(wx_s, &cw, &t_h);
                    //int cw = (int) ceil(fl_width(s + line_start, after_word_end - line_start));
                    if(cw > w)  {
                        break;
                    } else {
                        p = (int)(after_word_end - 1);
                    }
                }
            }
            line_len = p - line_start;
        } else {
            /// the first word is longer then "w", truncate it
            const char* ss = s + line_start;
            wxString wx_ss = ToWxString(ss, line_len - 1);
            line_len = TruncTextLength(dc, wx_ss, w, ewxTruncate_Empty);
            if(line_len == 0)   { // impossible to truncate
                line_len = text_len - line_start;
            }
        }
        line_lens.push_back(line_len);

        pos += line_len;
    }
}


wxBitmap CwxSplittingArtProvider::CreateBitmap(const wxArtID& anId,
                                               const wxArtClient& aClient,
                                               const wxSize& aSize )
{
    if( aClient != wxART_OTHER ){
        return wxNullBitmap;
    }

    int pos = anId.Find( wxT("::") );
    if( pos == wxNOT_FOUND ){
        return wxNullBitmap;
    }

    wxString new_cli = anId.Mid( 0, pos );
    wxString new_id = anId.Mid( pos+2 );

    if( new_cli.IsSameAs( wxT("menu") ) ){
        new_cli = wxART_MENU;
    } else if( new_cli.IsSameAs( wxT("wm") ) ){
        new_cli = wxART_FRAME_ICON;
    /*
    } else if( new_cli.IsSameAs( "view" ) ){
    } else if( new_cli.IsSameAs( "symbol" ) ){
    } else if( new_cli.IsSameAs( "tree" ) ){
    } else if( new_cli.IsSameAs( "map" ) ){
    */
    } else {
        new_cli += wxT("_C");
    }

    return wxArtProvider::GetBitmap( new_id, new_cli, aSize );
}


wxColour ConvertColor(const CRgbaColor& c2)
{
    return wxColour(c2.GetRedUC(), c2.GetGreenUC(), c2.GetBlueUC(), c2.GetAlphaUC());
}


CRgbaColor ConvertColor(const wxColour& c)
{
    return CRgbaColor(c.Red(), c.Green(), c.Blue(), c.Alpha());
}


void SaveWindowRectToRegistry(const wxRect& rc, CRegistryWriteView view)
{
    view.Set("x", rc.x);
    view.Set("y", rc.y);
    view.Set("width", rc.width);
    view.Set("height", rc.height);
}


void LoadWindowRectFromRegistry(wxRect& rc, const CRegistryReadView& view)
{
    rc.x = view.GetInt("x", rc.x);
    rc.y = view.GetInt("y", rc.y);
    rc.width = view.GetInt("width", rc.width);
    rc.height = view.GetInt("height", rc.height);
}

void CorrectWindowRect(wxTopLevelWindow* win, wxRect& rc)
{
#ifdef NCBI_OS_MSWIN
    int disp_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int disp_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int disp_w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int disp_h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    rc.width = rc.width > disp_w ? disp_w : rc.width;
    rc.height = rc.height > disp_h ? disp_h : rc.height;
    rc.x = rc.x < disp_x + 1 ? disp_x + 1 : rc.x;
    rc.y = rc.y < disp_y + 1 ? disp_y + 1 : rc.y;
#elif __WXOSX_COCOA__
    // Make sure size (on mac) is within valid x && y boundaries even
    // in the case of multiple monitors.  With multiple monitors,
    // (0,0) is the upper left corner of whichever monitor has
    // the menubar.  X values of secondary monitors behave as you 
    // expect (negative to left, positive to the right).  But y
    // values do the opposite of what window screen coordinates do -
    // a monitor below the monitor with the menubar has a negative
    // base y coordinate, and a monitor above the monitor with 
    // the menubar has a positive base y coordinate.
     
    int monitors = [NSScreen screens].count;
    int maxx = 0;
    int maxy = 0;
    int minx = 0;
    int miny = 0;
    int menubar_height;

    for (int i=0; i<monitors; ++i) {
        NSScreen* s = [[NSScreen screens] objectAtIndex:i];
        NSRect visible_frame = [s visibleFrame];
        //_TRACE("monitor: " << i << " x: " << visible_frame.origin.x  << " y: " << visible_frame.origin.y <<
        //        " width: " << visible_frame.size.width << " height: " << visible_frame.size.height);

        // (x,y) will be the lower right (max) screen coordinates of the current monitor
        int x = visible_frame.origin.x + visible_frame.size.width;

        int flipped_y = -visible_frame.origin.y; // If not the base monitor, flip the y value (base is 0)
        int y = flipped_y + visible_frame.size.height;

        if (x > maxx)
            maxx = x;
        if (visible_frame.origin.x < minx)
            minx = visible_frame.origin.x;

        if (y > maxy)
            maxy = y;
        if (flipped_y < miny)
            miny = flipped_y;
    }

    // Trim the x direction:
    if (rc.x + rc.width > maxx) {
         rc.x = maxx - rc.width;
    }
    if (rc.x < minx) {
         rc.x = minx;
         if (rc.width > maxx-minx)
             rc.width = maxx-minx;
    }
    
    // Trim the y direction:
    if (rc.y + rc.height > maxy) {
         rc.y = maxy - rc.height;
    }
    if (rc.y < miny) {
        rc.y = miny;
        if (rc.height > maxy-miny)
            rc.height = maxy-miny;
    }

    // Account for the menubar (which is only on 1 monitor for multiple
    // monitors but for simplicity we just keep y below the menubar height
    // without regard for which monitor it's on. 
    GetMacDragWindowBounds(win, menubar_height, maxy, maxx);
    if (rc.y < menubar_height) {
        rc.y = menubar_height;
        if (rc.y + rc.height > maxy) { 
            rc.height -= menubar_height;
        }
    }
#endif
    //_TRACE("Actual Screen: (x,y): " << rc.x << "," << rc.y << ") (w,h): " << rc.width << "," << rc.height << ")");
}


// TODO move to a proper place
bool NcbiChooseColor(wxWindow* parent, CRgbaColor& color)   
{
    wxColourData data;
    data.SetColour(wxColour(color.GetRedUC(), color.GetGreenUC(), color.GetBlueUC()));

    wxColourDialog dialog(parent, &data);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxColourData retData = dialog.GetColourData();
        wxColour col = retData.GetColour();
        color.Set(col.Red(), col.Green(), col.Blue()); //TODO provide operator
        return true;
    }
    return false;
}

wxWindow* FindChildWindowById( long id, wxWindow* parent )
{
    wxWindow *res = NULL;

    ITERATE( wxWindowList, wit, parent->GetChildren() ){
        wxWindow *child = *wit;

        if( child->GetId() == id ){
            res = child;
            break;
        }
    }
    return res;
}

void BroadcastCommandToChildWindows(wxWindow* window, int cmd_id, int cmd_data)
{  
    // Create a command event but do not have it propogate to parent windows (since
    // we are iteratively sending the command to windows downward from the parent)
    // (we use a button command because it is convienant - it is not actually 
    //  a button click)
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, cmd_id);
    evt.SetInt(cmd_data);

    evt.StopPropagation(); 

    window->GetEventHandler()->ProcessEvent(evt);

    wxWindowList& list = window->GetChildren();
    ITERATE( wxWindowList, wit, list ){
        wxWindow *current = *wit;       
        BroadcastCommandToChildWindows( current, cmd_id, cmd_data );
    }
}

void CFixGenericListCtrl::ConnectToControl( wxListCtrl& listCtrl )
{
    _ASSERT(!m_ListCtrl);

    wxClassInfo* classInfo = wxClassInfo::FindClass(wxT("wxGenericListCtrl"));
    if (classInfo == 0 || !listCtrl.IsKindOf(classInfo))
        return;

    ITERATE( wxWindowList, wit, listCtrl.GetChildren() ){
        wxWindow *child = *wit;

        if (child->GetName() == wxT("panel")) {
            child->Bind(wxEVT_CHILD_FOCUS, &CFixGenericListCtrl::OnChildFocus, this);
            m_ListCtrl = &listCtrl;
            //LOG_POST(Info << "CFixGenericListCtrl::ConnectToControl: Connected");
        }
    }
}

void CFixGenericListCtrl::OnChildFocus( wxCommandEvent& event )
{
    _ASSERT(m_ListCtrl);
    if( m_ListCtrl ){
        m_ListCtrl->GetEventHandler()->ProcessEvent( event );
    }
}

void AddChildWindowAbove(wxFrame* parent, wxWindow* child)
{
#ifdef __WXOSX_COCOA__
    wxNonOwnedWindow* nowin = dynamic_cast<wxNonOwnedWindow*>(child);
    if (nowin == NULL)
        return;
    NSWindow* mac_win = nowin->GetWXWindow();

    if (mac_win != NULL) {
        NSWindow* mac_window = parent->GetWXWindow();
        [mac_window addChildWindow:mac_win ordered:NSWindowAbove];
    }
#endif
}

void AddChildWindowBelow(wxFrame* parent, wxWindow* child)
{
#ifdef __WXOSX_COCOA__
    wxNonOwnedWindow* nowin = dynamic_cast<wxNonOwnedWindow*>(child);
    if (nowin == NULL)
        return;
    NSWindow* mac_win = nowin->GetWXWindow();

    if (mac_win != NULL) {
        NSWindow* mac_window = parent->GetWXWindow();
        [mac_window addChildWindow:mac_win ordered:NSWindowBelow];
    }
#endif
}

void RemoveChildWindow(wxFrame* parent, wxWindow* child)
{
#ifdef __WXOSX_COCOA__
    wxNonOwnedWindow* nowin = dynamic_cast<wxNonOwnedWindow*>(child);
    if (nowin == NULL)
        return;
    NSWindow* mac_win = nowin->GetWXWindow();

    if (mac_win != NULL) {
        NSWindow* mac_window = parent->GetWXWindow();
        [mac_window removeChildWindow:mac_win];
    }
#endif
}

void GetMacDragWindowBounds(wxTopLevelWindow* win, int& menubar_height, int& maxy, int& maxx)
{
#ifdef __WXMAC__
    // Get menu bar height (menubar at top of screen) works on both cocoa and carbon.
    CGFloat menuBarHeight = [[NSApp mainMenu] menuBarHeight];
    menubar_height = (int) menuBarHeight;

#ifdef __WXMAC_CARBON__
     // This does not take the dock into account so you can still get stuck behind it,
     // but it does work on carbon.
     HIRect r;
     HIWindowGetAvailablePositioningBounds(kCGNullDirectDisplay,
                                           kHICoordSpaceScreenPixel,
                                           &r);
     maxy = r.size.height - menubar_height;

     // on carbon, menubar height dosn't keep the title bar in the open so we double it.
     menubar_height *= 2;
#else // COCOA
    // This gives a smaller area and should keep windows from being stuck behind the dock. 
    // Cocoa version only.
    NSWindow* nswin =  win->GetWXWindow();
    NSScreen* screen = [nswin deepestScreen];

    if (screen != NULL) {
        NSRect visible_frame = [screen visibleFrame];
        maxy = visible_frame.size.height;
        maxx = visible_frame.size.width;
    }
#endif
#endif
}

// Mac-specific function to read keyboard without having keys from an event
// handy - only checks if cmd-key is down or not.
bool GetMacOptionKeyDown()
{
#ifdef __WXMAC__        
    if (NSEventModifierFlagOption & [ NSEvent modifierFlags ]) {
        return true;
    }
#endif

    return false;
}

// This fixes the problem described in:
// http://trac.wxwidgets.org/ticket/10168
// which is really a windows vista/ windows 7 problem rather than a wx problem.
// The problem only occurs when the aero theme is disabled.  Probably a bug in windows
// compositing since they kind of have taken over the front buffer with latest versions
bool DlgGLWinOverlayFix(wxWindow* win) 
{
    // fix is trivial, just resize (but do in a way to keep size same over time)
    // I also tried to just send a size event, but that does not work.
#ifdef __WXMSW__
#ifdef _VISTA_WORKAROUND_
    static int size_change = 1;
    size_change *= -1;

    int w,h;
    win->GetSize(&w, &h);
    win->SetSize(w+size_change, h);
    //_TRACE("Applied OpenGL Dialog Overaly Fix");
    return true;
#endif
#endif
    return false;
}

wxSize GetDisplayPPI() 
{
    wxSize s(72, 72);

#ifdef __WXOSX_COCOA__
    NSScreen* screen = [NSScreen mainScreen];
    if (screen != NULL) {
        NSDictionary *description = [screen deviceDescription];
        if (description != NULL) {
            NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
            CGSize displayPhysicalSize = CGDisplayScreenSize(
                           [[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);

            s.SetWidth(int((displayPixelSize.width/displayPhysicalSize.width)*25.4f));
            s.SetHeight(s.GetWidth());
            // there being 25.4 mm in an inch
        }
    }
    
    _TRACE("Screen DPI: (w,h): " << s.GetWidth() << "," << s.GetHeight() << ")");
    return s;
#else
    return wxGetDisplayPPI();
#endif
}

bool RunningInsideNCBI()
{
    static bool initialized = false;
    static bool insideNCBI = false;

    if (initialized)
        return insideNCBI;
    initialized = true;

    wxString p = CSysPath::ResolvePathExisting(wxT("<home>/emulate_external"));
    if (!p.empty())
        return insideNCBI;

    insideNCBI = CConnTest::IsNcbiInhouseClient();
    return insideNCBI;
}

wxImageList* CreateCheckboxImages(wxWindow* wnd)
{
    wxRendererNative& renderer = wxRendererNative::Get();

    // Get the native size of the checkbox

    int width = renderer.GetCheckBoxSize(wnd).GetWidth();
    int height = renderer.GetCheckBoxSize(wnd).GetHeight();

    wxImageList* imageList = new wxImageList(width, height, TRUE);

    wxBitmap unchecked_bmp(width, height),
        checked_bmp(width, height),
        unchecked_disabled_bmp(width, height),
        checked_disabled_bmp(width, height);

    wxMemoryDC renderer_dc;

    wxColor bkgColor = wnd->GetBackgroundColour();

    // Unchecked
    renderer_dc.SelectObject(unchecked_bmp);
    renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(bkgColor));
    renderer_dc.Clear();
    renderer.DrawCheckBox(wnd, renderer_dc, wxRect(0, 0, width, height), 0);

    // Checked
    renderer_dc.SelectObject(checked_bmp);
    renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(bkgColor));
    renderer_dc.Clear();
    renderer.DrawCheckBox(wnd, renderer_dc, wxRect(0, 0, width, height), wxCONTROL_CHECKED);

    // Unchecked and Disabled
    renderer_dc.SelectObject(unchecked_disabled_bmp);
    renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(bkgColor));
    renderer_dc.Clear();
    renderer.DrawCheckBox(wnd, renderer_dc, wxRect(0, 0, width, height), 0 | wxCONTROL_DISABLED);

    // Checked and Disabled
    renderer_dc.SelectObject(checked_disabled_bmp);
    renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(bkgColor));
    renderer_dc.Clear();
    renderer.DrawCheckBox(wnd, renderer_dc, wxRect(0, 0, width, height), wxCONTROL_CHECKED | wxCONTROL_DISABLED);

    // Deselect the renderers Object
    renderer_dc.SelectObject(wxNullBitmap);

    // the add order must respect the wxCLC_XXX_IMGIDX defines in the headers !
    imageList->Add(unchecked_bmp);
    imageList->Add(checked_bmp);
    imageList->Add(unchecked_disabled_bmp);
    imageList->Add(checked_disabled_bmp);

    return imageList;
}


END_NCBI_SCOPE
