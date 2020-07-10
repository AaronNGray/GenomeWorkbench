#ifndef GUI_WIDGETS_WX___UTILS__HPP
#define GUI_WIDGETS_WX___UTILS__HPP

/*  $Id: wx_utils.hpp 44433 2019-12-18 15:53:19Z shkeda $
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
 *      A temporary place for various GUI utitlities
 */

#include <gui/gui.hpp>

#include <gui/objutils/registry.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/artprov.h>
#include <wx/colour.h>
#include <wx/utils.h>
#include <wx/arrstr.h>
#include <wx/event.h>
#include <wx/string.h>

#include <util/unicode.hpp>


class wxMenu;
class wxMenuItem;
class wxString;
class wxMenuBar;
class wxRect;
class wxWindow;
class wxDC;
class wxColour;
class wxListCtrl;
class wxTopLevelWindow;

class wxFileArtProvider;
class wxImageList;

#ifdef __WXMSW__
    #define wxTR_GBENCH_LINES wxTR_LINES_AT_ROOT
#else
    #define wxTR_GBENCH_LINES wxTR_NO_LINES
#endif

BEGIN_NCBI_SCOPE

struct  SMenuItemRec;
class   CMenuItem;
class   CRgbaColor;


NCBI_GUIWIDGETS_WX_EXPORT wxString FindExeFile(const wxString& exeFile);

NCBI_GUIWIDGETS_WX_EXPORT void ReportMemoryUsage();

NCBI_GUIWIDGETS_WX_EXPORT void ReportIDError(const string& id_label, bool is_local, const string& title = "Error message");

NCBI_GUIWIDGETS_WX_EXPORT void OpenFileBrowser(const wxString& path);

class CChecksum;
NCBI_GUIWIDGETS_WX_EXPORT string GetMD5Digest(const CChecksum& cs);

/// Returns id of video configuration to be used for saving settings
NCBI_GUIWIDGETS_WX_EXPORT string GetVideoId();

NCBI_GUIWIDGETS_WX_EXPORT   //bool InitDefaultFileArtProvider(wxFileArtProvider* provider);
    void    InitDefaultFileArtProvider(const wxString& dir);
NCBI_GUIWIDGETS_WX_EXPORT   wxFileArtProvider* GetDefaultFileArtProvider();

// Copy "in" to "out"
NCBI_GUIWIDGETS_WX_EXPORT   void    FromArrayString(const wxArrayString& in, vector<string>& out);
NCBI_GUIWIDGETS_WX_EXPORT   void    ToArrayString(const vector<string>& out, wxArrayString& in);


/// Creates a wxMenu object replicating the structure of CMenuItem
NCBI_GUIWIDGETS_WX_EXPORT wxMenu* CreateMenu(const CMenuItem* item);

/// Find a subitem of the given menu by text
NCBI_GUIWIDGETS_WX_EXPORT wxMenuItem* FindSubItem(wxMenu& menu, const wxString& text);

/// merges all items form menu_2 into menu_1, preserving the structure if possible
NCBI_GUIWIDGETS_WX_EXPORT void Merge(wxMenu& menu_1, const wxMenu& menu_2);

/// Removes extra separators (in the begining or at the end of the menu, ot those
/// that precede other separators)
NCBI_GUIWIDGETS_WX_EXPORT void CleanupSeparators(wxMenu& menu);

/// create a copy of the given menu
NCBI_GUIWIDGETS_WX_EXPORT wxMenu* CloneMenu(const wxMenu& menu);

/// Using default menu item margin width.
/// The is a work-around for a wxWidget bug on failing to reset to default 
/// margin width after using any user-owned bitmap image in menu item.
/// The failure will cause the default radio menu item bitmap image disappear.
NCBI_GUIWIDGETS_WX_EXPORT void UseDefaultMarginWidth(wxMenu& menu);

/// Set margin width for menu item.
NCBI_GUIWIDGETS_WX_EXPORT void SetMenuItemMarginWidth(wxMenuItem* item, wxBitmap * bmp);

class NCBI_GUIWIDGETS_WX_EXPORT CCommandToFocusHandler : public wxEvtHandler
{
public:
    CCommandToFocusHandler( wxWindow* window = NULL )
        : m_Window( window )
        , mf_Reentry( false )
    {
    }

protected:
    void OnCommandEvent( wxCommandEvent& event );
    void OnMenuEvent( wxCommandEvent& event );
    void OnUpdateUIEvent( wxUpdateUIEvent& event );

private:
    wxWindow* m_Window;
    bool mf_Reentry;

    DECLARE_EVENT_TABLE()
};

class CBoolGuard {
public:
    CBoolGuard( bool& flag ) : m_Flag( flag ) { m_Flag = true; }
    ~CBoolGuard() { m_Flag = false; }

    operator bool() { return m_Flag; }

private:
    bool& m_Flag;
};

inline string ToStdString(const wxString& s)
{
    return string(s.ToAscii());
}

inline string ToAsciiStdString(const wxString& input)
{
  const wxScopedCharBuffer line(input.ToUTF8());
  string output = utf8::UTF8ToAsciiString(line, (const utf8::SUnicodeTranslation*) NULL);
  return output;
}

inline wxString ToWxString(const string& s)
{
    string clean(s);
    std::replace_if(clean.begin(), clean.end(),
        [](const char& c) -> bool { return c < 0 || c > 127; }, '?');
    return wxString::FromAscii(clean.c_str());
}

inline wxString ToWxString(const char* s, size_t len)
{
    string clean(s, len);
    std::replace_if(clean.begin(), clean.end(),
        [](const char& c) -> bool { return c < 0 || c > 127; }, '?');
    return wxString::FromAscii(clean.c_str());
}

inline ostream& operator <<( ostream& os, const wxCharBuffer& buff )
{
    return os << buff.data();
}

inline const CNcbiDiag& operator <<( const CNcbiDiag& diag, const wxString& line )
{
    return diag << line.ToUTF8().data();
}

// Conversion functions to save filenames to ASN
NCBI_GUIWIDGETS_WX_EXPORT wxString FnToWxString(const string& s);
NCBI_GUIWIDGETS_WX_EXPORT string FnToStdString(const wxString& s);

/// converts an absolute path to a relative one based on current workspace
/// directory
NCBI_GUIWIDGETS_WX_EXPORT wxString ToRelativePath(const wxString& base, const wxString& abs_path);
/// converts relative path from the current workspace directory to an
/// absolute path
NCBI_GUIWIDGETS_WX_EXPORT wxString ToAbsolutePath(const wxString& base, const wxString& rel_path);

NCBI_GUIWIDGETS_WX_EXPORT string ToString(const wxRect& rc);

// get window bounds in screen coordinates
NCBI_GUIWIDGETS_WX_EXPORT wxRect GetScreenRect(const wxWindow& win);

/// Windows specific function, equivalent of SetWindowText() WinAPI
NCBI_GUIWIDGETS_WX_EXPORT void WindowSetText(const wxWindow& win, const wxChar* text);


NCBI_GUIWIDGETS_WX_EXPORT
    wxColour GetAverage(const wxColor& c1, const wxColor& c2, double ratio);

/// Truncate options for drawing text with FLTK API
enum    Ewx_Truncate    {
    ewxTruncate_Empty,    /// truncate text if needed
    ewxTruncate_Ellipsis, /// truncate text if needed, add "..." if truncated
    ewxTruncate_EllipsisAlways /// truncate text if needed, add "..." even if not truncated
};


/// returns number of characters representing a truncated version of "s"
/// which fits in the gievn width "w"
/// "trunc" controls truncation, if "trunc" == eTruncate_Ellipsis
/// "..." is appended to the end of the truncated string.
NCBI_GUIWIDGETS_WX_EXPORT
int TruncTextLength(wxDC& dc, const wxString& s, int w,
                    Ewx_Truncate trunc = ewxTruncate_Ellipsis);


/// truncates given string so that its length is less or equal "w"
/// "trunc" controls truncation, if "trunc" == eTruncate_Ellipsis
/// "..." is appended to the end of the truncated string.
NCBI_GUIWIDGETS_WX_EXPORT
wxString TruncateText(wxDC& dc, const wxString& s, int w,
                    Ewx_Truncate trunc = ewxTruncate_Ellipsis);

/// divides given "text" into lines, so that every line has width less or equal
/// to "w". Returns length of lines in "line_lens".
NCBI_GUIWIDGETS_WX_EXPORT
    void WrapText(wxDC& dc, const string& text, int w, vector<int>& line_lens);

///////////////////////////////////////////////////////////////////////////////
/// CwxSplittingArtProvider - an adapter for old-style image aliases
class NCBI_GUIWIDGETS_WX_EXPORT CwxSplittingArtProvider :
    public wxArtProvider
{
    virtual wxBitmap CreateBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size);
};


NCBI_GUIWIDGETS_WX_EXPORT wxColour ConvertColor(const CRgbaColor& color);

NCBI_GUIWIDGETS_WX_EXPORT CRgbaColor ConvertColor(const wxColour& color);

NCBI_GUIWIDGETS_WX_EXPORT
    bool NcbiChooseColor(wxWindow* parent, CRgbaColor& color);

NCBI_GUIWIDGETS_WX_EXPORT
    void SaveWindowRectToRegistry(const wxRect& rc, CRegistryWriteView view);

NCBI_GUIWIDGETS_WX_EXPORT
    void LoadWindowRectFromRegistry(wxRect& rc, const CRegistryReadView& view);

NCBI_GUIWIDGETS_WX_EXPORT
    void CorrectWindowRect(wxTopLevelWindow* win, wxRect& rc);

NCBI_GUIWIDGETS_WX_EXPORT
    wxWindow* FindChildWindowById(long id, wxWindow* parent);

/// Sends command event with id 'cmd_id' to window and all its children
NCBI_GUIWIDGETS_WX_EXPORT
    void BroadcastCommandToChildWindows(wxWindow* window, int cmd_id, int cmd_data=0);

/// For OSX Cocoa, puts child window in front of (visually) parent
NCBI_GUIWIDGETS_WX_EXPORT
    void AddChildWindowAbove(wxFrame* parent, wxWindow* child);
/// For OSX Cocoa, puts child window behind the parent
NCBI_GUIWIDGETS_WX_EXPORT
    void AddChildWindowBelow(wxFrame* parent, wxWindow* child);
/// For OSX Cocoa, removes child window connection (for layering) to parent
NCBI_GUIWIDGETS_WX_EXPORT
    void RemoveChildWindow(wxFrame* parent, wxWindow* child);
/// Returns window boundaries for mac (top menubar width and max height)
NCBI_GUIWIDGETS_WX_EXPORT
    void GetMacDragWindowBounds(wxTopLevelWindow* win, int& menubar_height, int& maxy, int &maxx);
/// For mac, return an ID for the current space (of "Spaces" fame)
NCBI_GUIWIDGETS_WX_EXPORT
    bool GetMacOptionKeyDown();
/// Fix a problem on windows where after a dialog overlays an opengl window the opengl
/// window may be constrained to just the area under the dialog until some window-changing event.
/// Problem only occurs if Aero theme is Not being used.
NCBI_GUIWIDGETS_WX_EXPORT
    bool DlgGLWinOverlayFix(wxWindow* win);
/// Mac always seems to return 72 for screen PPI so use wx function for non-mac and on mac
/// divide the displays number of pixels by its physical size to get actual PPI
NCBI_GUIWIDGETS_WX_EXPORT
    wxSize GetDisplayPPI();

NCBI_GUIWIDGETS_WX_EXPORT
bool RunningInsideNCBI();

//
// This class can be used with wxListCtrl
// In case the current platform uses wxGenericListCtrl
// CFixGenericListCtrl will allow wxChildFocusEvent to propagate up the windows hierarchy.
// In case the current platform doesn't use wxGenericControl this class does nothing.
// This partially fixes pane activation issue. GB-726
//

class NCBI_GUIWIDGETS_WX_EXPORT CFixGenericListCtrl : public wxEvtHandler
{
public:
    CFixGenericListCtrl() : m_ListCtrl(0) {}
    void ConnectToControl(wxListCtrl& listCtrl);

private:
    void OnChildFocus(wxCommandEvent& event);

    wxListCtrl* m_ListCtrl;
};

NCBI_GUIWIDGETS_WX_EXPORT wxImageList* CreateCheckboxImages(wxWindow* wnd);

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___UTILS__HPP
