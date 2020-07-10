#ifndef GUI_WIDGETS_WX___MAIN_FRAME__HPP
#define GUI_WIDGETS_WX___MAIN_FRAME__HPP

/*  $Id: main_frame.hpp 35937 2016-07-13 18:35:20Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/reg_settings.hpp>

#include <wx/frame.h>

BEGIN_NCBI_SCOPE

class IWorkbench;

///////////////////////////////////////////////////////////////////////////////
/// CMainFrame
/// Base class for Application Main Frame, derive your frames from this one.
class NCBI_GUIWIDGETS_WX_EXPORT  CMainFrame :
    public wxFrame,
    public IRegSettings
{
    DECLARE_EVENT_TABLE()
public:
    /// IHintListener - an interface for a component that shows command help
    /// hints
    class IHintListener {
    public:
        virtual ~IHintListener() {}
        virtual void    ShowCommandHint(const string& text) = 0;
        virtual void    HideCommandHint() = 0;
    };

public:
    typedef wxFrame TParent;

    /// Constructors
    CMainFrame();
    CMainFrame(wxWindow* parent, wxWindowID id, const wxString& caption,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxDEFAULT_FRAME_STYLE );
    ~CMainFrame();

    virtual void    SetHintListener(IHintListener* listener);

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    void    OnExitClick(wxCommandEvent& event);

    /// overriding these function to make it less intrusive
    virtual void DoGiveHelp(const wxString& text, bool show);

protected:

#ifdef NCBI_OS_MSWIN
    /// Override wxFrame::MSWWindowProc() in order to handle WM_ENABLE event
    virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
    void OnActivate(wxActivateEvent& event);
    void x_SaveMenuFocus();
    void x_RestoreMenuFocus();
#endif

    string  m_RegPath; /// path in registry

    IHintListener* m_HintListener;

#ifdef NCBI_OS_MSWIN
    int m_ModalCounter; // incremented for every entrance into modal state
    wxWindow* m_MenuFocusWnd;
#endif
};


END_NCBI_SCOPE


#endif // GUI_WIDGETS_WX___MAIN_FRAME__HPP
