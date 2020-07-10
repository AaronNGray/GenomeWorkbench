#ifndef GUI_WIDGETS_WX___TEST_CLIENTS__HPP
#define GUI_WIDGETS_WX___TEST_CLIENTS__HPP

/*  $Id: test_clients.hpp 42108 2018-12-20 20:35:52Z katargir $
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

#include <gui/gui_export.h>

#include <gui/widgets/wx/wm_client.hpp>
#include <gui/opengl.h>

// Disable deprecated warning for AGLDrawable in wx code (MAC specific)
#if defined(__WXMAC__)  &&  NCBI_COMPILER_VERSION >= 421
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <wx/glcanvas.h>
#if defined(__WXMAC__)  &&  NCBI_COMPILER_VERSION >= 421
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif

#include <gui/opengl/glpane.hpp>

#include <gui/widgets/wx/wm_client.hpp>
#include <gui/widgets/wx/glcanvas.hpp>

#include <wx/textctrl.h>
#include <wx/treectrl.h>
#include <wx/timer.h>
#include "wx/html/htmlwin.h"

class wxHtmlWindow;


////@begin control identifiers
#define ID_CWXCLOCKPANEL 10010
#define ID_HTMLWINDOW2 10012
#define SYMBOL_CWXCLOCKPANEL_STYLE 0
#define SYMBOL_CWXCLOCKPANEL_TITLE wxT("Clock View")
#define SYMBOL_CWXCLOCKPANEL_IDNAME ID_CWXCLOCKPANEL
#define SYMBOL_CWXCLOCKPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CWXCLOCKPANEL_POSITION wxDefaultPosition
////@end control identifiers

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///  CClockPanelWMClient
class NCBI_GUIWIDGETS_WX_EXPORT CClockPanelWMClient :
    public wxPanel,
    public IWMClient
{
    DECLARE_DYNAMIC_CLASS( CClockPanelWMClient )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CClockPanelWMClient();
    CClockPanelWMClient(wxWindow* parent);

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXCLOCKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CWXCLOCKPANEL_POSITION, const wxSize& size = SYMBOL_CWXCLOCKPANEL_SIZE, long style = SYMBOL_CWXCLOCKPANEL_STYLE );

    /// Destructor
    ~CClockPanelWMClient();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void OnTimer(wxTimerEvent& event);

    /// @name IWMCleint interface implementation
    /// @{
    virtual wxWindow*      GetWindow();
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string  GetClientLabel(IWMClient::ELabel ltype) const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor(const CRgbaColor& color);
    virtual string GetIconAlias() const;
    virtual const wxMenu*  GetMenu();
    virtual void    UpdateMenu(wxMenu& root_menu);
    virtual CFingerprint  GetFingerprint() const;
    /// @}

    /// Should we show tooltips?
    static bool ShowToolTips();

    void    x_Update();
    virtual bool Show(bool show = true);

protected:
    wxHtmlWindow* m_HTMLWindow;
    wxTimer m_Timer;
    CRgbaColor  m_Color;
};


////////////////////////////////////////////////////////////////////////////////
/// CTextWMClient
class NCBI_GUIWIDGETS_WX_EXPORT CTextWMClient :
    public wxTextCtrl,
    public IWMClient
{
public:
    CTextWMClient(wxWindow* parent)
        : wxTextCtrl(parent, wxID_ANY, wxT("This is the text"),
                     wxDefaultPosition, wxSize(0,0), wxNO_BORDER |wxTE_MULTILINE | wxTE_RICH2)
    {
    }

    virtual wxWindow*      GetWindow()  {   return this;    }
    virtual string  GetClientLabel(IWMClient::ELabel ltype) const {   return "Text View";     }
    virtual const CRgbaColor* GetColor() const  {   return NULL;    }
    virtual void SetColor(const CRgbaColor& color)  {}
    virtual const wxMenu*  GetMenu() {  return NULL;    }
    virtual void    UpdateMenu(wxMenu& root_menu)   {}
    virtual CFingerprint  GetFingerprint() const
    {
        return CFingerprint(GetClientLabel(IWMClient::eDefault), false);
    }
};


////////////////////////////////////////////////////////////////////////////////
/// CHTMLWMClient
class NCBI_GUIWIDGETS_WX_EXPORT CHTMLWMClient :
    public wxHtmlWindow,
    public IWMClient
{
public:
    CHTMLWMClient(wxWindow* parent);

    virtual wxWindow*      GetWindow()  {   return this;    }
    virtual string  GetClientLabel(IWMClient::ELabel ltype) const {   return "HTML View";     }
    virtual const CRgbaColor* GetColor() const  {   return NULL;    }
    virtual void SetColor(const CRgbaColor& color)  {}
    virtual const wxMenu*  GetMenu() {  return NULL;    }
    virtual void    UpdateMenu(wxMenu& root_menu)   {}
    virtual CFingerprint  GetFingerprint() const
    {
        return CFingerprint(GetClientLabel(IWMClient::eDefault), false);
    }
protected:
    static int m_Count;
};


////////////////////////////////////////////////////////////////////////////////
/// CTreeWMClient
class NCBI_GUIWIDGETS_WX_EXPORT CTreeWMClient :
    public wxTreeCtrl,
    public IWMClient
{
public:
    CTreeWMClient(wxWindow* parent);
    virtual wxWindow*      GetWindow()  {   return this;    }
    virtual string  GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const {   return "Tree View";     }
    virtual const CRgbaColor* GetColor() const  {   return NULL;    }
    virtual void SetColor(const CRgbaColor& color)  {}
    virtual const wxMenu*  GetMenu() {  return NULL;    }
    virtual void    UpdateMenu(wxMenu& root_menu)   {}
    virtual CFingerprint  GetFingerprint() const
    {
        return CFingerprint(GetClientLabel(), false);
    }
};


////////////////////////////////////////////////////////////////////////////////
/// CTextWMClient
class NCBI_GUIWIDGETS_WX_EXPORT CGLTestWMClient :
    public CGLCanvas,
    public IWMClient
{
    DECLARE_EVENT_TABLE();

public:
    CGLTestWMClient(wxWindow* parent);

    void OnSize(wxSizeEvent& event);

    virtual wxWindow*      GetWindow()  {   return this;    }
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string  GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const {   return "Open GL view";     }
    virtual string  GetIconAlias()  const  {   return "";     }
    virtual const CRgbaColor* GetColor() const  {   return NULL;    }
    virtual void SetColor(const CRgbaColor& color)  {}
    virtual const wxMenu*  GetMenu() {  return NULL;    }
    virtual void    UpdateMenu(wxMenu& root_menu)   {}
    virtual CFingerprint  GetFingerprint() const
    {
        return CFingerprint(GetClientLabel(), false);
    }

protected:
    virtual void x_Render();

    CGlPane  m_Port;
};


END_NCBI_SCOPE;

#endif  // GUI_WIDGETS_WX___TEST_CLIENTS__HPP

