#ifndef GUI_FRAMEWORK___ITEM_SELECTION_PANEL__HPP
#define GUI_FRAMEWORK___ITEM_SELECTION_PANEL__HPP

/*  $Id: item_selection_panel.hpp 24477 2011-09-29 17:12:25Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/group_map_widget.hpp>

#include <gui/utils/ui_object.hpp>

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>

////@begin includes
////@end includes

#include <wx/panel.h>

////@begin control identifiers
#define SYMBOL_CITEMSELECTIONPANEL_STYLE wxNO_BORDER|wxCLIP_CHILDREN|wxTAB_TRAVERSAL
#define SYMBOL_CITEMSELECTIONPANEL_TITLE _("ItemSelectionPanel")
#define SYMBOL_CITEMSELECTIONPANEL_IDNAME ID_CITEMSELECTIONPANEL
#define SYMBOL_CITEMSELECTIONPANEL_SIZE wxDefaultSize
#define SYMBOL_CITEMSELECTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


class wxTextCtrl;
class wxStaticText;
class wxButton;

BEGIN_NCBI_SCOPE

class CSplitter;
class CGroupMapWidget;


// an abstract item in CItemSelectionPanel
class IItemWithDescription
{
public:
    virtual const IUIObject&  GetDescriptor() const = 0;
    virtual string    GetCategory() = 0;
    virtual bool      ShowDefault() const = 0;

    virtual ~IItemWithDescription() {}
};

///////////////////////////////////////////////////////////////////////////////
/// CItemSelectionPanel

class CItemSelectionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CItemSelectionPanel )
    DECLARE_EVENT_TABLE()

public:
    CItemSelectionPanel();
    CItemSelectionPanel( wxWindow* parent,
                         wxWindowID id = SYMBOL_CITEMSELECTIONPANEL_IDNAME,
                         const wxPoint& pos = SYMBOL_CITEMSELECTIONPANEL_POSITION,
                         const wxSize& size = SYMBOL_CITEMSELECTIONPANEL_SIZE,
                         long style = SYMBOL_CITEMSELECTIONPANEL_STYLE );

    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_CITEMSELECTIONPANEL_IDNAME,
                 const wxPoint& pos = SYMBOL_CITEMSELECTIONPANEL_POSITION,
                 const wxSize& size = SYMBOL_CITEMSELECTIONPANEL_SIZE,
                 long style = SYMBOL_CITEMSELECTIONPANEL_STYLE );

    ~CItemSelectionPanel();

    void Init();

    void CreateControls();

////@begin CItemSelectionPanel event handler declarations

    void OnKeyDown( wxKeyEvent& event );

    void OnSearchUpdated( wxCommandEvent& event );

    void OnResetClick( wxCommandEvent& event );

////@end CItemSelectionPanel event handler declarations

////@begin CItemSelectionPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CItemSelectionPanel member function declarations

    static bool ShowToolTips();

////@begin CItemSelectionPanel member variables
    wxTextCtrl* m_SearchCtrl;
    wxButton* m_ResetBtn;
    wxStaticText* m_StatusText;
    CSplitter* m_Splitter;
    enum {
        ID_CITEMSELECTIONPANEL = 10026,
        ID_SEARCH = 10028,
        ID_RESET = 10029,
        ID_STATUS = 10032,
        ID_SPLITTER = 10027
    };
////@end CItemSelectionPanel member variables
    enum {
        ID_MAP_WIDGET = 10030,
        IDDESCR = 10031
    };

    wxTextCtrl* m_DescrCtrl;
    CGroupMapWidget* m_MapWidget;

public:
    typedef CIRef<IItemWithDescription>   TItemRef;

    void    SetItemTypeLabel(const string& label);
    void    SetItems(vector<TItemRef>& items);

    void    OnItemSelected(wxCommandEvent& event);

    TItemRef GetSelectedItemRef();

    /// @name IRegSettings interface
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    void    x_UpdateMapWidget();
    void    x_FilterItems(const string& query);

    size_t  x_FindItemByLabel(const string& label);
    void    x_AddItem(CGroupMapWidget::TGroupDescrVector& descrs,
                      IItemWithDescription& manager);

protected:
    vector<TItemRef>    m_AllItems;
    vector<TItemRef>    m_Items;

    vector<string>      m_CollapsedGroups;

    string  m_ItemTypeLabel;
    string  m_RegPath;
};

END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___ITEM_SELECTION_PANEL__HPP
