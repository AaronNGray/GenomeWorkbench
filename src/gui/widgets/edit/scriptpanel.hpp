/*  $Id:  scriptpanel.h 38070 06/07/2017 15:12:46 17:48:35Z Igor Filippov$
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
 * Authors:  Igor Filippov
 */

#ifndef _SCRIPTPANEL_H_
#define _SCRIPTPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/macro_rep.hpp>
#include <gui/objutils/macro_engine.hpp>

/*!
 * Includes
 */

////@begin includes
#include <wx/nonownedwnd.h>
#include <wx/propgrid/propgrid.h>
#include <wx/generic/statbmpg.h>
#include <wx/cursor.h>
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSCRIPTPANEL 10001
#define ID_SCROLLEDWINDOW 10003
#define ID_SCROLLEDWINDOW1 10004
#define SYMBOL_CSCRIPTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSCRIPTPANEL_TITLE _("ScriptPanel")
#define SYMBOL_CSCRIPTPANEL_IDNAME ID_CSCRIPTPANEL
#define SYMBOL_CSCRIPTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSCRIPTPANEL_POSITION wxDefaultPosition
////@end control identifiers

class CMacroLabel;
class CMacroParamsPanel;
class CMacroLabelHandler;
class CMacroFlowEditor;

/*!
 * CScriptPanel class declaration
 */

class CScriptPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CScriptPanel )
    DECLARE_EVENT_TABLE()
        
 public:
    /// Constructors
    CScriptPanel();
    CScriptPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSCRIPTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSCRIPTPANEL_POSITION, const wxSize& size = SYMBOL_CSCRIPTPANEL_SIZE, long style = SYMBOL_CSCRIPTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSCRIPTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSCRIPTPANEL_POSITION, const wxSize& size = SYMBOL_CSCRIPTPANEL_SIZE, long style = SYMBOL_CSCRIPTPANEL_STYLE );

    /// Destructor
    ~CScriptPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CScriptPanel event handler declarations

////@end CScriptPanel event handler declarations

////@begin CScriptPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CScriptPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    CMacroLabel* AddMacro(CRef<macro::CMacroRep> macro);
    CMacroLabel* InsertMacro(CRef<macro::CMacroRep> macro, size_t index);
    void FitPage();
    vector<pair<CRef<macro::CMacroRep>, bool> > GetScript();
    void SelectMacro(CMacroLabel *label);
    void SelectMacros(CMacroLabel *label) ;
    void UnselectMacro();
    void SelectItem(size_t item);
    bool IsMacroSelected() { return !m_selected_macros.empty();}
    void ToggleSkip();
    void ToggleSkip(CMacroLabel* label);
    void DragMacro();
    void DropMacro();
    void OnMouseUp();
    void OnMouseDown();
    const map<size_t, CMacroLabel*>& GetSelectedMacros() {return m_selected_macros;}
    void IncreaseFont();
    void DecreaseFont();
    void DeleteSelected();
    size_t GetIndex(CMacroLabel* label);
    wxPoint GetMousePosition() {return m_Position;}
    void SetMousePosition(wxPoint p) {m_Position = p;}
    wxPoint GetCurrentMousePosition() {return  m_CurrentMousePos;}
    bool IsDragging() {return m_drag;}
    void UpdateCounter(size_t i, size_t counter);
    void ResetCounters();
    wxString GetPath() {return m_path;}
    void SetPath(const wxString& path) {m_path = path;}
    bool IsModified() {return m_modified;}
    void SetModified(bool modified) {m_modified = modified;}
    void UpdateParamsPanel();
    size_t GetCount();
    void OnRightClick(wxContextMenuEvent &evt);
    void CommitChanges();
    void SaveScrollPos();
    void LoadScrollPos();
 private:
    void ShowSplashWindow(CMacroFlowEditor* frame);
    size_t GetInsertIndex(int y, wxSizerItemList& children);
    void ShiftBottomLabels();
   

////@begin CScriptPanel member variables
    wxScrolledWindow *m_ScriptWindow;
    CMacroParamsPanel *m_Parameters;
////@end CScriptPanel member variables
    map<size_t, CMacroLabel*> m_selected_macros;
    size_t m_displaced;
    CMacroLabelHandler *m_handler;
    bool m_drag;
    bool m_once_per_drag;
    wxFont m_font;
    wxPoint m_Position;
    wxPoint  m_CurrentMousePos;

    class CDragTimer : public wxTimer
    {
        CScriptPanel* m_panel;
    public:
        CDragTimer(CScriptPanel* panel) : wxTimer(), m_panel(panel) {}
        void Notify() 
            {
                if (m_panel->IsDragging())
                    m_panel->DragMacro();
            }
    };

    CDragTimer m_timer;
    wxString m_path;
    bool m_modified;
    wxCursor m_copy_cursor;
    int m_scroll_pos;
};

#ifdef __WXMAC__
typedef wxPanel TMacroLabelParent;
#else
typedef wxNonOwnedWindow TMacroLabelParent;
#endif


class CMacroLabel: public  TMacroLabelParent
{    
    DECLARE_CLASS( CMacroLabel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroLabel();
    CMacroLabel(wxWindow* parent, CRef<macro::CMacroRep> macro, const wxFont &font, wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxFRAME_SHAPED|wxBORDER_NONE);

    bool Create( wxWindow* parent,  wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxFRAME_SHAPED|wxBORDER_NONE);

    /// Destructor
    ~CMacroLabel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

   /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    //virtual bool Layout();
    
    CRef<macro::CMacroRep> GetMacro() {return m_macro;}
    void ToggleSkip();
    void OnMouseDrag();
    void OnMouseUp();
    void OnMouseDown();
    void OnMouseAndShiftDown();
    void DragMacro();
    void IncreaseFont();
    void DecreaseFont();
    wxFont GetFont();
    void Select();
    void Unselect();
    bool IsSkipped() {return m_skipped;}
    void SetIndex(size_t index);
    size_t GetIndex() {return m_index_val;}
    void UpdateCounter(size_t counter);
    static wxColour GetLabelColour(const string &str);
    void UpdateText();
    void UpdateMacro(CRef<macro::CMacroRep> macro);
    bool IsSelected() {return m_selected;}
    void CommitChanges();
private:
    void RoundedShape();
    int GetTextWidth();
    wxString ConstructTitle();

#ifdef __WXMAC__
    void SetShape(wxGraphicsPath&) {}
#endif
    CRef<macro::CMacroRep> m_macro;
    wxPanel* m_panel;
    wxStaticText *m_text;
    CMacroLabelHandler *m_handler1;
    CMacroLabelHandler *m_handler2;
    CMacroLabelHandler *m_handler3;
    CMacroLabelHandler *m_handler4;
    CMacroLabelHandler *m_handler5;
    wxGenericStaticBitmap  *m_bitmap;
    wxBitmap m_static_bitmap;
    bool m_skipped;
    wxStaticText* m_index;
    size_t m_index_val;
    wxStaticText* m_counter;
    wxFont m_font;
    bool m_selected;
    macro::CMacroEngine m_MEngine;
};

class CMacroParamsPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CMacroParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroParamsPanel();
    CMacroParamsPanel( wxWindow* parent, 
                       wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );

    /// Destructor
    ~CMacroParamsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
  /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocationListCtrl member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void SetMacro(CRef<macro::CMacroRep> macro, CMacroLabel *label = NULL);
    void OnPropertyChanged( wxPropertyGridEvent& event );
    void UpdateMacro();
    void CommitChanges();
private:

    CRef<macro::CMacroRep> m_macro;
    wxPropertyGrid *m_PropGridCtrl;
    vector<pair<macro::IMacroVar*, wxPGProperty*> > m_props;
    CMacroLabel *m_label;
};

class CMacroLabelHandler : public wxEvtHandler
{
public:
    CMacroLabelHandler(CMacroLabel *win) : m_win(win), m_panel(NULL) {}
    CMacroLabelHandler(CScriptPanel *panel) : m_win(NULL), m_panel(panel) {}
protected:
    // event handlers
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseDrag(wxMouseEvent& event);
    void OnMouseDoubleClick(wxMouseEvent& event);
private:
    void RunSimpleEditor();

    CMacroLabel *m_win;
    CScriptPanel *m_panel;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CMacroLabelHandler);
};

END_NCBI_SCOPE
#endif
    // _SCRIPTPANEL_H_
