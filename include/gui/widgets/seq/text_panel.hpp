#ifndef GUI_WIDGETS_SEQ___TEXT_PANEL__HPP
#define GUI_WIDGETS_SEQ___TEXT_PANEL__HPP

/*  $Id: text_panel.hpp 43120 2019-05-16 16:34:13Z katargir $
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

#include <gui/gui_export.h>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>

#include <gui/objutils/objects.hpp>

#include <objtools/format/flat_file_config.hpp>

#include <gui/widgets/wx/gui_widget.hpp>
#include <gui/widgets/wx/ibioseq_editor.hpp>
#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>

#include <wx/panel.h>
#include <wx/aui/aui.h>


////@begin includes
////@end includes

class wxTextCtrl;
class wxCheckBox;
class wxChoice;
class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CRichTextCtrl;

////@begin control identifiers
#define SYMBOL_CTEXTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTEXTPANEL_TITLE _("Text Panel")
#define SYMBOL_CTEXTPANEL_IDNAME ID_CTEXTPANEL
#define SYMBOL_CTEXTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTEXTPANEL_POSITION wxDefaultPosition
////@end control identifiers

struct SConstScopedObject;
class CTextItemPanel;
class CTextPanelContext;
class CTextDataSource;
class ICommandProccessor;
class IMenuContributor;
class CRootTextItem;
class CUICommandRegistry;

class CFlatFileSeq
{
public:
    CFlatFileSeq() : m_Item() {}
    CFlatFileSeq(const string& name, objects::CBioseq_Handle& handle) :
        m_Name(name), m_Handle(handle), m_Item() {}

    ~CFlatFileSeq() { delete m_Item; }

    string m_Name;
    objects::CBioseq_Handle m_Handle;
    ITextItem* m_Item;
};

class NCBI_GUIWIDGETS_SEQ_EXPORT CTextPanel: public wxPanel
    , public IFlatFileCtrl
    , public IGuiWidget
{
    friend class CAsnViewContext;

    DECLARE_DYNAMIC_CLASS( CTextPanel )
    DECLARE_EVENT_TABLE()

public:
    static wxWindow* CreateForm(wxWindow* parent);

    enum TViewType { kFlatFile, kFastA, kASN, kXML, kUninitialized };

    /// @name IFlatFileCtrl interface implementation
    /// @{
    virtual bool SetPosition(const string& seq, const CObject* object);
    virtual bool SetPosition(const objects::CBioseq_Handle& h, const CObject* object);
    /// @}

    /// @name IGuiWidget implementation
    /// @{
    virtual void SetHost(IGuiWidgetHost* host);
    virtual bool InitWidget(TConstScopedObjects& objects);
    virtual const CObject* GetOrigObject() const;

    // Selection
    virtual void GetSelectedObjects (TConstObjects& objects) const;
    virtual void SetSelectedObjects (const TConstObjects& objects);

    // Data
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor);
    virtual void DataChanging();
    virtual void DataChanged();

    // Objects for action
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects);
    /// @}

    objects::CScope* GetScope() { return m_Scope.GetPointerOrNull(); }
    ICommandProccessor* GetCmdProccessor() { return m_CmdProccessor; }


    /// Constructors
    CTextPanel();
    CTextPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CTEXTPANEL_SIZE, long style = SYMBOL_CTEXTPANEL_STYLE );

    /// Destructor
    ~CTextPanel();

    /// Initialises member variables
    void Init();

    bool ObjectIsFeature() const;
    void UpdateRegistry();
    void RefreshView();
    void SaveState();
    void ClearState();
    void Load();

    void SetInitialViewType(const string& type);
    void SetInitialSequence(const string& sequence) { m_SequenceShown = sequence;  }

    void DataLoaded(CRootTextItem& root, CTextPanelContext& context);
    void SequenceListLoaded(const vector<CFlatFileSeq>& sequenceList);

    void AddExtensionToolbar(wxAuiToolBar& toolbar, const wxString& name);

    bool CanShowFlatFile() const;
    bool CanShowFastA() const;
    bool CanShowASN() const;
    bool CanShowXML() const;

    CIRef<IMenuContributor> GetMenuContributor();

    /// Creates the controls and sizers
    void CreateControls();

    void OnSearchTermCtrlEnter( wxCommandEvent& event );
    void OnFindNextClick( wxCommandEvent& event );
    void OnFindModeSelected( wxCommandEvent& event );
    void OnSequenceSelected( wxCommandEvent& event );
    void OnOptionsClick( wxCommandEvent& event );
    void OnContextMenu(wxContextMenuEvent& event);

////@begin CTextPanel member function declarations

    TViewType GetViewType() const { return m_ViewType ; }
    void SetViewType(TViewType value) { m_ViewType = value ; }

    int GetFontSize() const { return m_FontSize ; }
    void SetFontSize(int value);

    int GetFlatFileMode() const { return m_FlatFileMode ; }
    void SetFlatFileMode(int value) { m_FlatFileMode = value ; }

    bool GetShowSequence() const { return m_ShowSequence; }
    void SetShowSequence(bool value) { m_ShowSequence = value; }

    bool GetShowComponentFeats() const { return m_ShowComponentFeats; }
    void SetShowComponentFeats(bool value = true) { m_ShowComponentFeats = value; }

    int GetFeatMode() const { return m_FeatMode ; }
    void SetFeatMode(int value) { m_FeatMode = value ; }

    bool GetShowASNTypes() const { return m_ShowASNTypes ; }
    void SetShowASNTypes(bool value) { m_ShowASNTypes = value ; }

    bool GetShowASNPathToRoot() const { return m_ShowASNPathToRoot ; }
    void SetShowASNPathToRoot(bool value) { m_ShowASNPathToRoot = value ; }

    int GetFindMode() const { return m_FindMode ; }
    void SetFindMode(int value) { m_FindMode = value ; }

    bool GetHideVariations() const { return m_FFHideVariations; }
    void SetHideVariations(bool value) { m_FFHideVariations = value; }

    bool GetHideSTS() const { return m_FFHideSTS; }
    void SetHideSTS(bool value) { m_FFHideSTS = value; }

    bool GetOpenExpanded() const { return m_OpenExpanded; }
    void SetOpenExpanded(bool value) { m_OpenExpanded = value; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTextPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

	void GetMainObjects(TConstScopedObjects& objs);

    CTextItemPanel* GetWidget() const { return m_TextWindow; }

    static void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);
    static void SetTextBackground(const wxColor& color);
    static wxFont GetFont(int size);

    void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }

////@begin CTextPanel member variables
    CRichTextCtrl* m_FindTextCtrl;
    wxChoice* m_FindModeCtrl;
    wxChoice* m_SequenceList;
    CTextItemPanel* m_TextWindow;
private:
    TViewType m_ViewType;
    int m_FontSize;
    int m_FlatFileMode;
    bool m_ShowSequence;
    bool m_ShowComponentFeats;
    int m_FeatMode;
    bool m_ShowASNTypes;
    bool m_ShowASNPathToRoot;
    int m_FindMode;
    bool m_FFHideVariations;
    bool m_FFHideSTS;
    bool m_OpenExpanded;
    /// Control identifiers
    enum {
        ID_CTEXTPANEL = 10004,
        ID_WIDGET,

        ID_TB_SearchTerm,
        ID_TB_SearchButton,
        ID_TB_SearchModeLabel,
        ID_TB_SearchModeCtrl,
        ID_TB_SequenceLabel,
        ID_TB_SequenceListCtrl,
        ID_TB_Options
    };
////@end CTextPanel member variables

private:
    void x_LoadFlatFile();
    void x_LoadFastA();
    void x_LoadASN();
    void x_LoadXML();
    void x_UpdateViewType(const wxString& type);
    void x_SaveViewType();
    void x_FindText();
	bool x_SelectFlatFileSequence(const string& seqName);

    void x_FillSeqList();

    CTextPanelContext* x_CreateFlatFileContext(bool createEditor = true);
    CTextPanelContext* x_CreateAsnContext(const CSerialObject& so);
    CTextPanelContext* x_CreateXmlContext(const CSerialObject& so);
    CTextPanelContext* x_CreateFastaContext();
    void x_InitContext(CTextPanelContext& context);
    CIRef<IBioseqEditor> x_CreateEditor();

    void x_ReadRegistry();

    void x_CreateMainToolbar();

    CConstRef<CSerialObject> m_SO;
    CRef<objects::CScope> m_Scope;
    CRef<CTextDataSource> m_DataSource;

    string        m_SequenceShown;
    CConstRef<CObject> m_PositionOnLoad;

    IGuiWidgetHost* m_WidgetHost;

    wxAuiManager    m_AuiManager;

    vector<CFlatFileSeq> m_FlatFileSeqs;

    ICommandProccessor* m_CmdProccessor;

    map<string, bm::bvector<> > m_TreeState;
    int m_SavedPosX;
    int m_SavedPosY;

    bool m_initial = true;

    wxString m_WorkDir;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___TEXT_PANEL__HPP
