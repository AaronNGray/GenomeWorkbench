#ifndef GUI_WIDGETS_OBJECT_LIST_WIDGET___OBJECT_LIST_WIDGET_SEL__HPP
#define GUI_WIDGETS_OBJECT_LIST_WIDGET___OBJECT_LIST_WIDGET_SEL__HPP

/*  $Id: object_list_widget_sel.hpp 43789 2019-08-30 16:18:52Z katargir $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/tglbtn.h"
#include "wx/listctrl.h"
////@end includes

#include <wx/panel.h>
#include <wx/stattext.h>
#include <objmgr/scope.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/object_list.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_COBJECTLISTWIDGETSEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_COBJECTLISTWIDGETSEL_TITLE _("Object list widget with selector")
#define SYMBOL_COBJECTLISTWIDGETSEL_IDNAME ID_COBJECTLISTWIDGETSEL
#define SYMBOL_COBJECTLISTWIDGETSEL_SIZE wxSize(350, 200)
#define SYMBOL_COBJECTLISTWIDGETSEL_POSITION wxDefaultPosition
////@end control identifiers

class wxChoice;
class wxButton;
class wxRadioButton;

/*!
 * CObjectListWidgetSel class declaration
 */

BEGIN_NCBI_SCOPE

class CObjectListWidget;
class CwxTableListCtrl;
class CGroupTableModel;

typedef map<string, TConstScopedObjects> TObjectsMap;

typedef CRef<CObjectList> TObjectList;
typedef map<string, TObjectList> TObjectListMap;

class IObjectCombinator 
{
    /****** PUBLIC DEFINITION SECTION ******/
public:
    virtual ~IObjectCombinator() {}

    virtual void operator()( const TConstScopedObjects& objects, TObjectsMap& objects_map ) const = 0;
};



class IObjectListAccValidator
{
public:
    virtual ~IObjectListAccValidator() {}
    virtual CConstRef<CObject> Validate(const objects::CSeq_id& id, objects::CScope& scope, wxString& errMessage) = 0;
};

class NCBI_GUIWIDGETS_OBJECT_LIST_EXPORT CObjectListWidgetSel: public wxPanel
    , public IRegSettings
{
    DECLARE_DYNAMIC_CLASS( CObjectListWidgetSel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CObjectListWidgetSel();
    CObjectListWidgetSel( wxWindow* parent, wxWindowID id = SYMBOL_COBJECTLISTWIDGETSEL_IDNAME, const wxPoint& pos = SYMBOL_COBJECTLISTWIDGETSEL_POSITION, const wxSize& size = SYMBOL_COBJECTLISTWIDGETSEL_SIZE, long style = SYMBOL_COBJECTLISTWIDGETSEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COBJECTLISTWIDGETSEL_IDNAME, const wxPoint& pos = SYMBOL_COBJECTLISTWIDGETSEL_POSITION, const wxSize& size = SYMBOL_COBJECTLISTWIDGETSEL_SIZE, long style = SYMBOL_COBJECTLISTWIDGETSEL_STYLE );

    /// Destructor
    ~CObjectListWidgetSel();

    /// Initialises member variables
    void Init();

    /// IRegSettings
    virtual void SetRegistryPath(const string& path) { m_RegPath = path; }
    virtual void SaveSettings() const;
    virtual void LoadSettings();

    void SetTitle(const wxString& value);

    void SetFilterDuplicates(bool filter) { m_FilterDuplicates = filter; }

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

    void SetAccessionValidator(IObjectListAccValidator* validator);

    void ShowNuclProtButtons(void);

    void EnableGroups(bool enable = true);

    void SetDefaultSelection(const wxString& selection) {
        m_DefaultSelection = selection;
    }


////@begin CObjectListWidgetSel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ADDACC_CTRL
    void OnAccessionUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ADDACC_BTN
    void OnAddAccessionClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_FILTER
    void OnFilterUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_NUC_RDBTN
    void OnNucleotidesSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PROT_RDBTN
    void OnProteinsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_SELECT_ALL
    void OnSelectAll( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TOGGLEGRP
    void OnGroupClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CATCHOICE
    void OnCategorySelected( wxCommandEvent& event );

    void OnSelectionUpdate( wxListEvent& event );

////@end CObjectListWidgetSel event handler declarations

////@begin CObjectListWidgetSel member function declarations

    CIRef<IObjectCombinator> GetCombinator() const { return m_Combinator ; }
    void SetCombinator(CIRef<IObjectCombinator> value) { m_Combinator = value ; }

    bool GetDoSelectAll() const { return m_DoSelectAll ; }
    void SetDoSelectAll(bool value) { m_DoSelectAll = value ; }

    TObjectsMap GetGroupsMap() const { return m_GroupsMap ; }
    void SetGroupsMap(TObjectsMap value) { m_GroupsMap = value ; }

    TConstScopedObjects GetSelection() const { return m_Selection ; }
    void SetSelection(TConstScopedObjects value) { m_Selection = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CObjectListWidgetSel member function declarations

    const TObjectListMap& GetObjects() const { return m_ObjectListMap ; }
    void SetObjects( const TObjectListMap& value ){ m_ObjectListMap = value; };
    void SetObjects( const TObjectsMap* value );


    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CObjectListWidgetSel member variables
    wxTextCtrl* m_Accession;
    wxButton* m_AddBtn;
    wxTextCtrl* m_FilterText;
    wxRadioButton* m_NucRadio;
    wxRadioButton* m_ProtRadio;
    wxToggleButton* m_ToggleGroups;
    wxChoice* m_CatChoice;
    CObjectListWidget* m_ObjTable;
    CwxTableListCtrl* m_GrpTable;
    wxStaticText* m_TotalRows;
private:
    CIRef<IObjectCombinator> m_Combinator;
    bool m_DoSelectAll;
    TObjectsMap m_GroupsMap;
    CGroupTableModel* m_GrpTableModel;
    TConstScopedObjects m_Selection;
    /// Control identifiers
    enum {
        ID_COBJECTLISTWIDGETSEL = 10000,
        ID_STATICBOX1 = 10001,
        ID_ADDACC_CTRL = 10002,
        ID_ADDACC_BTN = 10003,
        ID_FILTER = 10004,
        ID_NUC_RDBTN = 10005,
        ID_PROT_RDBTN = 10006,
        ID_BUTTON_SELECT_ALL = 10011,
        ID_TOGGLEGRP = 10007,
        ID_CATCHOICE = 10008,
        ID_OBJTABLE = 10009,
        ID_GRPTABLE = 10010
    };
////@end CObjectListWidgetSel member variables

    typedef map<string, SConstScopedObject> TObjectLabelsMap;

    TObjectListMap m_ObjectListMap;

    TObjectListMap m_NucObjects;
    TObjectListMap m_ProtObjects;

    wxString m_DefaultSelection;
    
private:
    void x_SetUpObjectList( CObjectList* obj_list );
    CRef<CObjectList> x_FilterObjectList( CObjectList* obj_list, string pattern );
    CRef<CObjectList> x_GetObjectList(const wxString& category, TObjectListMap& obj_listmap);
    void x_CopyUniqueObjects(const CObjectList &src_list, CObjectList &dst_list, TObjectLabelsMap &object_labels);
    CRef<CObjectList> x_FilterDuplicates(TObjectListMap& obj_listmap);

    void x_FillWithObjects(const wxString& conv);
    void x_PopulateObjects( CObjectList* obj_list );
    void x_PopulateGroups();
    
    void x_UpdateUI();
    void x_UpdateAddBtn();
    void x_UpdateSequences();
    void x_UpdateTotalRows();

    bool m_ObjectsPacked;
    bool m_GroupsPacked;

    string  m_RegPath;
    bool    m_SingleSel;
    bool    m_FilterDuplicates;

    string m_Pattern;

    IObjectListAccValidator* m_AccValidator;

    CRef<objects::CScope> m_Scope;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_OBJECT_LIST_WIDGET___OBJECT_LIST_WIDGET_SEL__HPP
