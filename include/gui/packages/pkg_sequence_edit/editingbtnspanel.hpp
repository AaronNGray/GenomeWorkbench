/*  $Id: editingbtnspanel.hpp 44311 2019-11-26 22:49:57Z filippov $
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
 * Authors:  Colleen Bollin
 */
#ifndef _EDITINGBTNSPANEL_H_
#define _EDITINGBTNSPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <gui/utils/event_handler.hpp>

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/framework/pkg_wb_connect.hpp>


#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>

#include <gui/packages/pkg_sequence_edit/remove_sequences.hpp>


/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/collpane.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/frame.h>
#include <wx/srchctrl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE
using namespace objects;

class ICommandProccessor;
class IMovableButton;
class CButtonPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CEDITINGBTNSPANEL 10020
#define ID_CHECKBOX_COLLAPSE 10021
#define ID_TAB1 10022
#define ID_TAB2 10023
#define ID_TAB3 10024
#define ID_CHECKBOX_AUTOSTART 10025
#define ID_SET_BG_COLOR 10026
#define ID_RESET_BG_COLOR 10027

#define ID_NOTEBOOK_PANEL 10030
#define ID_TAB_REMOVED 10031
#define ID_RESET_TABS 10032
#define ID_FIND_BUTTON 10033
#define ID_IMPORT_TABS 10034
#define ID_EXPORT_TABS 10035
#define ID_ADD_MACRO 10036

#define ID_POPUP_A 10037
#define ID_POPUP_B 10038
#define ID_POPUP_C 10039
#define ID_POPUP_REMOVE 10040
#define ID_POPUP_EDIT 10041

#define ID_BUTTON1 10043
#define ID_BUTTON2 10044
#define ID_BUTTON3 10045
#define ID_BUTTON4 10046
#define ID_BUTTON5 10047
#define ID_BUTTON6 10048
#define ID_BUTTON7 10049
#define ID_BUTTON8 10050
#define ID_BUTTON9 10051
#define ID_BUTTON10 10052
#define ID_BUTTON11 10053
#define ID_BUTTON12 10054
#define ID_BUTTON13 10055
#define ID_BUTTON14 10056
#define ID_BUTTON15 10057
#define ID_BUTTON16 10058
#define ID_BUTTON17 10059
#define ID_BUTTON18 10060
#define ID_BUTTON19 10061
#define ID_BUTTON20 10062
#define ID_BUTTON21 10063
#define ID_BUTTON22 10064
#define ID_BUTTON23 10065
#define ID_BUTTON24 10066
#define ID_BUTTON25 10067
#define ID_BUTTON26 10068
#define ID_BUTTON27 10069
#define ID_BUTTON28 10070
#define ID_BUTTON29 10071
#define ID_BUTTON30 10072
#define ID_BUTTON31 10073
#define ID_BUTTON32 10074
#define ID_BUTTON33 10075
#define ID_BUTTON34 10076
#define ID_BUTTON35 10077
#define ID_BUTTON36 10078
#define ID_BUTTON37 10079
#define ID_BUTTON38 10080
#define ID_BUTTON39 10081
#define ID_BUTTON40 10082
#define ID_BUTTON41 10083
#define ID_BUTTON42 10084
#define ID_BUTTON43 10085
#define ID_BUTTON44 10086
#define ID_BUTTON45 10087
#define ID_BUTTON46 10088
#define ID_BUTTON47 10089
#define ID_BUTTON48 10090
#define ID_BUTTON49 10091
#define ID_BUTTON50 10092
#define ID_BUTTON51 10093
#define ID_BUTTON52 10094
#define ID_BUTTON53 10095
#define ID_BUTTON54 10096
#define ID_BUTTON55 10097
#define ID_BUTTON56 10098
#define ID_BUTTON57 10099
#define ID_BUTTON58 10100
#define ID_BUTTON59 10101
#define ID_BUTTON60 10102
#define ID_BUTTON61 10103
#define ID_BUTTON62 10104
#define ID_BUTTON63 10105
#define ID_BUTTON64 10106
#define ID_BUTTON65 10107
#define ID_BUTTON66 10108
#define ID_BUTTON67 10109
#define ID_BUTTON68 10110
#define ID_BUTTON69 10111
#define ID_BUTTON70 10112
#define ID_BUTTON71 10113
#define ID_BUTTON72 10114
#define ID_BUTTON73 10115
#define ID_BUTTON74 10116
#define ID_BUTTON75 10117
#define ID_BUTTON76 10118
#define ID_BUTTON77 10119
#define ID_BUTTON78 10120
#define ID_BUTTON79 10121
#define ID_BUTTON80 10122
#define ID_BUTTON81 10123
#define ID_BUTTON82 10124
#define ID_BUTTON83 10125
#define ID_BUTTON84 10126
#define ID_BUTTON85 10127
#define ID_BUTTON86 10128
#define ID_BUTTON87 10129
#define ID_BUTTON88 10130
#define ID_BUTTON89 10131
#define ID_BUTTON90 10132
#define ID_BUTTON91 10133
#define ID_BUTTON92 10134
#define ID_BUTTON93 10135
#define ID_BUTTON94 10136
#define ID_BUTTON95 10137
#define ID_BUTTON96 10138
#define ID_BUTTON97 10139
#define ID_BUTTON98 10140
#define ID_BUTTON99 10141
#define ID_BUTTON100 10142
#define ID_BUTTON101 10143
#define ID_BUTTON102 10144
#define ID_BUTTON103 10145
#define ID_BUTTON104 10146
#define ID_BUTTON105 10147
#define ID_BUTTON106 10148
#define ID_BUTTON107 10149
#define ID_BUTTON108 10150
#define ID_BUTTON109 10151
#define ID_BUTTON110 10152
#define ID_BUTTON111 10153
#define ID_BUTTON112 10154
#define ID_BUTTON113 10155
#define ID_BUTTON114 10156
#define ID_BUTTON116 10158
#define ID_BUTTON117 10159
#define ID_BUTTON118 10160
#define ID_BUTTON119 10161
#define ID_BUTTON120 10162
#define ID_BUTTON121 10163
#define ID_BUTTON122 10164
#define ID_BUTTON123 10165
#define ID_BUTTON124 10166
#define ID_BUTTON125 10167
#define ID_BUTTON126 10168
#define ID_BUTTON127 10169
#define ID_BUTTON128 10170
#define ID_BUTTON129 10171
#define ID_BUTTON130 10172
#define ID_BUTTON131 10173
#define ID_BUTTON132 10174
#define ID_BUTTON133 10175
#define ID_BUTTON134 10176
#define ID_BUTTON135 10177
#define ID_BUTTON136 10178
#define ID_BUTTON137 10179
#define ID_BUTTON138 10180
#define ID_BUTTON139 10181
#define ID_BUTTON140 10182
#define ID_BUTTON141 10183
#define ID_BUTTON142 10184
#define ID_BUTTON143 10185
#define ID_BUTTON144 10186
#define ID_BUTTON145 10187
#define ID_BUTTON146 10188
#define ID_BUTTON147 10189
#define ID_BUTTON148 10190
#define ID_BUTTON149 10191
#define ID_BUTTON150 10192
#define ID_BUTTON151 10193
#define ID_BUTTON152 10194
#define ID_BUTTON153 10195
#define ID_BUTTON154 10196
#define ID_BUTTON155 10197
#define ID_BUTTON156 10198
#define ID_BUTTON157 10199
#define ID_BUTTON158 10200
#define ID_BUTTON159 10201
#define ID_BUTTON160 10202
#define ID_BUTTON161 10203
#define ID_BUTTON162 10204
#define ID_BUTTON163 10205
#define ID_BUTTON164 10206
#define ID_BUTTON165 10207
#define ID_BUTTON166 10208
#define ID_BUTTON167 10209
#define ID_BUTTON168 10210
#define ID_BUTTON169 10211
#define ID_BUTTON170 10212
#define ID_BUTTON171 10213
#define ID_BUTTON172 10214
#define ID_BUTTON173 10215

#define ID_BUTTON_DYNAMIC 10300

#define SYMBOL_CEDITINGBTNSPANEL_STYLE wxDEFAULT_FRAME_STYLE
#define SYMBOL_CEDITINGBTNSPANEL_TITLE _("EditingBtns")
#define SYMBOL_CEDITINGBTNSPANEL_IDNAME ID_CEDITINGBTNSPANEL
#define SYMBOL_CEDITINGBTNSPANEL_SIZE wxSize(600,400)
#define SYMBOL_CEDITINGBTNSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CEditingBtnsPanel class declaration
 */

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CEditingBtnsPanel: public wxFrame, public CEventHandler,  public IRegSettings //, public IPackageWorkbenchConnector
{    
    DECLARE_DYNAMIC_CLASS( CEditingBtnsPanel )
    DECLARE_EVENT_TABLE()

    /// Constructors
    CEditingBtnsPanel();
    CEditingBtnsPanel( wxWindow* parent, IWorkbench* workbench, wxWindowID id = SYMBOL_CEDITINGBTNSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEDITINGBTNSPANEL_POSITION, const wxSize& size = SYMBOL_CEDITINGBTNSPANEL_SIZE, long style = SYMBOL_CEDITINGBTNSPANEL_STYLE );
    /// Destructor
    ~CEditingBtnsPanel();

    CEditingBtnsPanel(CEditingBtnsPanel const&);              // Don't Implement
    CEditingBtnsPanel& operator=(CEditingBtnsPanel const&); // Don't implement
    static CEditingBtnsPanel *m_Instance;
    bool ShouldAutoStart(void) {return m_AutoStart->GetValue();}
public:

    static void GetInstance( wxWindow* parent, IWorkbench* workbench, bool autostart = false, wxWindowID id = SYMBOL_CEDITINGBTNSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEDITINGBTNSPANEL_POSITION, const wxSize& size = SYMBOL_CEDITINGBTNSPANEL_SIZE, long style = SYMBOL_CEDITINGBTNSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDITINGBTNSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEDITINGBTNSPANEL_POSITION, const wxSize& size = SYMBOL_CEDITINGBTNSPANEL_SIZE, long style = SYMBOL_CEDITINGBTNSPANEL_STYLE );

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();
    

    void OnClickLock( wxCommandEvent& event );
    void OnNotebookTabChanged(wxBookCtrlEvent &event);
    void ShowAfter(wxWindow* dlg);

////@begin CEditingBtnsPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnReverseAuthorNamesClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void EditPubs( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
    void ParseLocalIdToSrc( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
    void OnFixCapitalizationAll( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
    void OnFixCapitalizationAuthors( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON6
    void OnFixCapitalizationTitles( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON7
    void OnFixCapitalizationAffiliation( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON8
    void SortUniqueCount( wxCommandEvent& event );
    void ClickableSortUniqueCount( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON9
    void OnRemoveUnpublishedPublications( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON10
    void TableReaderFromClipboard( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
    void TaxFixCleanup( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
    void RemoveAllFeatures( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13
    void OnStripAuthorSuffixes( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON14
    void OnRemoveAuthorConsortiums( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON16
    void TrimNsRich( wxCommandEvent& event );

    void TrimNsTerminal( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON17
    void AnnotateDescriptorComment( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON18
    void MrnaCds( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON19
    void OnAddCitSubForUpdate( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON20
    void OnRemoveSequences( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON21
    void OnRemoveGenomeProjectsDB( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON22
    void RemProtTitles( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON23
    void RmCultureNotes( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON24
    void RevComp16S(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON25
    void OnRemoveDescriptors( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON26
    void StrainToSp( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON27
    void OnRemoveAllStructuredComments( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON28
    void BulkEditCDS( wxCommandEvent& event );

    void AddTranslExcept( wxCommandEvent& event );

    void OnRemoveFeatures( wxCommandEvent& event );

    void BulkGeneEdit( wxCommandEvent& event );

    void OnRemoveBankitComment( wxCommandEvent& event );

    void OnRemoveStructuredCommentKeyword( wxCommandEvent& event );

    void OnAddStructuredCommentKeyword( wxCommandEvent& event );

    void OnReorderStructuredComment( wxCommandEvent& event );

    void OnReorderSequences( wxCommandEvent& event );

    void OnConvertComment( wxCommandEvent& event );

    void OnParseComment( wxCommandEvent& event );

    void OnAssignFeatureIds( wxCommandEvent& event );

    void LabelrRNAs( wxCommandEvent& event );

    void RetranslateCDS( wxCommandEvent& event );

    void RetranslateCDS_NoStp( wxCommandEvent& event );

    void BulkEditRNA( wxCommandEvent& event );

    void OnFixiInPrimerSeqs( wxCommandEvent& event );

    void OnFixOrgModInstitution( wxCommandEvent& event );

    void OnSwapPrimerSeqName( wxCommandEvent& event );

    void OnMergePrimerSets( wxCommandEvent& event );

    void OnSplitPrimerSets( wxCommandEvent& event );

    void OnAppendModToOrg( wxCommandEvent& event );

    void OnParseCollectionDateMonthFirst( wxCommandEvent& event );

    void OnParseCollectionDateDayFirst( wxCommandEvent& event );

    void OnCountryFixupCap ( wxCommandEvent& event );

    void OnCountryFixupNoCap ( wxCommandEvent& event );
    
    void OnAddrRNA16S ( wxCommandEvent& event );
    
    void OnAddrRNA18S ( wxCommandEvent& event );
    
    void OnAddrRNA23S ( wxCommandEvent& event );
    
    void OnRemoveDefLines (wxCommandEvent& event);

    void VectorTrim( wxCommandEvent& event );

    void AddCDS( wxCommandEvent& event );

    void AddRNA( wxCommandEvent& event );

    void AddOtherFeature( wxCommandEvent& event );

    void ValidateSeq( wxCommandEvent& event );

    void DiscrepancyReport ( wxCommandEvent& event );

    void AddSequences( wxCommandEvent& event );

    void ApplySourceQual( wxCommandEvent& event );
    void EditSourceQual( wxCommandEvent& evt );
    void ConvertSourceQual( wxCommandEvent& evt );
    void SwapSourceQual( wxCommandEvent& evt );
    void RemoveSourceQual( wxCommandEvent& evt );
    void ApplyRNAQual( wxCommandEvent& evt );
    void EditRNAQual( wxCommandEvent& evt );
    void ConvertRNAQual( wxCommandEvent& evt );
    void SwapRNAQual( wxCommandEvent& evt );
    void RemoveRNAQual( wxCommandEvent& evt );
    void ApplyCGPQual( wxCommandEvent& evt );
    void EditCGPQual( wxCommandEvent& evt );
    void ConvertCGPQual( wxCommandEvent& evt );
    void SwapCGPQual( wxCommandEvent& evt );
    void RemoveCGPQual( wxCommandEvent& evt );
    void ApplyFeatQual( wxCommandEvent& evt );
    void EditFeatQual( wxCommandEvent& evt );
    void ConvertFeatQual( wxCommandEvent& evt );
    void SwapFeatQual( wxCommandEvent& evt );
    void RemoveFeatQual( wxCommandEvent& evt );
    void EditStructField( wxCommandEvent& evt );
    void EditFeatureEvidence( wxCommandEvent& evt );
    void EditFeatureLocation( wxCommandEvent& evt );
    void ParseText( wxCommandEvent& evt );
    void ParseTextFromDefline( wxCommandEvent& evt );
    void ConvertFeatures( wxCommandEvent& evt );
    void TableReader( wxCommandEvent& event );
    void BulkEdit( wxCommandEvent& event );
    void MolInfoEdit(wxCommandEvent& evt);
    void AutodefDefaultOptions(wxCommandEvent& evt);
    void AutodefOptions(wxCommandEvent& evt);
    void AutodefMisc(wxCommandEvent& evt);
    void AutodefId(wxCommandEvent& evt);
    void AutodefNoMods(wxCommandEvent& evt);
    void AutodefPopset(wxCommandEvent& evt);
    void AutodefRefresh(wxCommandEvent& evt);
    void ParseStrainSerotypeFromNames (wxCommandEvent& event);
    void AddStrainSerotypeToNames (wxCommandEvent& event);
    void FixupOrganismNames (wxCommandEvent& event);
    void SplitQualifiersAtCommas (wxCommandEvent& event);
    void SplitStructuredCollections (wxCommandEvent& event);
    void TrimOrganismNames (wxCommandEvent& event);
    void AddNamedrRNA12S(wxCommandEvent& event);
    void AddNamedrRNA28S(wxCommandEvent& event);
    void AddNamedrRNA26S(wxCommandEvent& event);
    void AddNamedrRNASmall(wxCommandEvent& event);
    void AddNamedrRNALarge(wxCommandEvent& event);
    void AddControlRegion ( wxCommandEvent& event);
    void AddMicrosatellite ( wxCommandEvent& event);
    void CreateFeature ( wxCommandEvent& evt);
    void RemoveUnverified( wxCommandEvent& event );
    void AddDefLine ( wxCommandEvent& event);

    void PrefixOrgToDefLine ( wxCommandEvent& event);
    void PrefixStrainToDefLine ( wxCommandEvent& event);
    void PrefixCloneToDefLine ( wxCommandEvent& event);
    void PrefixIsolateToDefLine ( wxCommandEvent& event);
    void PrefixHaplotToDefLine ( wxCommandEvent& event);
    void PrefixCultivarToDefLine ( wxCommandEvent& event);
    void PrefixListToDefLine ( wxCommandEvent& event);

    void SegregateSets( wxCommandEvent& event );
    void PT_Cleanup (wxCommandEvent& event);
    void OnCallerTool(wxCommandEvent& event);
    void SelectTarget(wxCommandEvent& event);
    // macros
    void RunMacro( wxCommandEvent& event );
    void RunMacroWithReport( wxCommandEvent& event );

    void AddSet( wxCommandEvent& event ); 
    void AddGSet( wxCommandEvent& event );
    void UnculTaxTool( wxCommandEvent& event );
    void RemoveSrcNotes( wxCommandEvent& evt );
    void AddIsolSource( wxCommandEvent& evt );
    void ExtendPartialsConstr( wxCommandEvent& evt );
    void ExtendPartialsAll( wxCommandEvent& evt );
    void ConvertCdsToMiscFeat( wxCommandEvent& evt );
    void LowercaseQuals( wxCommandEvent& evt );
    void RemoveTextOutsideString( wxCommandEvent& evt );
    void CorrectGenes( wxCommandEvent& event );
    void CreateDescGenomeAssemblyComment ( wxCommandEvent& evt);
    void ECNumberCleanup( wxCommandEvent& evt );
    void Add_rRNA( wxCommandEvent& evt );
    void Add_misc_feat( wxCommandEvent& evt );
    void Add_gene( wxCommandEvent& evt );
    void Save( wxCommandEvent& evt );
    void ExportTable(wxCommandEvent& evt);
    void SequesterSets( wxCommandEvent& event );
    void EditSubmitBlock( wxCommandEvent& event);
    void rRNAtoDNA( wxCommandEvent& event);
    void WrongQualsToNote( wxCommandEvent& event);
    void IllegalQualsToNote( wxCommandEvent& event);
    void RmIllegalQuals( wxCommandEvent& event);
    void RmWrongQuals( wxCommandEvent& event);
    void GroupExplode( wxCommandEvent& event);
    void RemoveTextInsideStr( wxCommandEvent& evt );
    void FindASN1( wxCommandEvent& evt );
    void RemoveDupFeats( wxCommandEvent& evt );
    void RmWrongOrIllegalQuals( wxCommandEvent& event);
    void UpdateSingleSeqClipboard( wxCommandEvent& event );
    void UpdateMultSeqClipboard(wxCommandEvent& event);
    void ShowFeatureTable( wxCommandEvent& event );
    void OnMegaReport(wxCommandEvent& event);
    void ImportFeatureTable(wxCommandEvent& event);
    void ImportFeatureTableClipboard(wxCommandEvent& event);
    void LaunchDesktop(wxCommandEvent& event);
    void BarcodeTool( wxCommandEvent& event );
    void ApplyDbxrefs(wxCommandEvent& event);
    void EditSequence( wxCommandEvent& event );
    void MacroEditor( wxCommandEvent& event );
    void SaveASN1File(wxCommandEvent& event);

////@end CEditingBtnsPanel event handler declarations

////@begin CEditingBtnsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditingBtnsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditingBtnsPanel member variables
////@end CEditingBtnsPanel member variables

    void SetFrameAndPosition();
    bool OnDataChange ();
    bool IsLocationSelected();
    bool IsLocationSelectedAa();
    bool IsLocationSelectedNa();
    void PlaceButtons();
    void x_LockButtons(wxSizer *sizer, bool locked);
    void OnResetTabs ( wxCommandEvent& event);
    void OnFindButton ( wxCommandEvent& event);
    void OnCancelFindButton ( wxCommandEvent& event);
    void OnExportBtnClick( wxCommandEvent& event );
    void OnImportBtnClick( wxCommandEvent& event );
    void OnAddMacroClick( wxCommandEvent& event );
    void OnSetBackground(wxHyperlinkEvent& event);
    void OnResetBackground(wxHyperlinkEvent& event);
    void ResetTabs();
    void RemoveButton(IMovableButton *button);
    void EditButton(IMovableButton *button);
    void OnChar(wxKeyEvent& evt);
    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    void LoadSettings(wxString path);
    void SaveSettings(wxString path) const;
    /// @}

private:
    vector<CRef<CSeq_loc> > x_GetSelectedLocations();
    void x_ResetRemovedTab();
    void x_InterpretMacro(const string& macro_name, CNcbiOstrstream& log);
    CSeq_entry_Handle x_GetSelectedSeq();
    void SetPanelsBackgroundColour();
    void SetTextViewBackgroundColour(const wxColour &color);
    void SaveSettings(const string &regpath, wxWindow *dlg) const;
    void LoadSettings(const string &regpath, wxWindow *dlg);
    void AddDefaultMacroButtons();
    void RemoveStaticButtons( vector<int> &ids,  vector<string> &filenames,  vector<string> &button_names,  vector<string> &titles);

    TConstScopedObjects    m_InputObjects;
    string m_RegPath;
    ICommandProccessor* m_CmdProccessor;
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxString m_WorkDir;
    CConstRef<CSeq_submit> m_SeqSubmit;
    IWorkbench* m_Workbench;
    vector<CRef<CSeq_loc> > m_Locations;
    wxCheckBox* m_CheckBox;
    wxCheckBox*  m_AutoStart;
    wxBoxSizer *m_Sizer1,*m_Sizer2, *m_Sizer3, *m_SizerRemoved;
    wxButton *m_resetTabsButton, *m_importTabsButton, *m_exportTabsButton, *m_AddMacroButton;
    wxSearchCtrl *m_findButton;
    wxSize m_FrameSize;
    wxPoint m_FramePosition;
    wxNotebook *m_Notebook;
    map< int,vector<pair<int,wxPoint> > > m_MovableButtonPosition;
    vector<IMovableButton*> m_all_buttons;
    mutable map<int, string> m_dynamic_buttons_filename, m_dynamic_buttons_title, m_dynamic_buttons_name;
    wxColour m_bg_color;
    bool m_bg_color_set;
    CButtonPanel *m_panel1;
    CButtonPanel *m_panel2;
    CButtonPanel *m_panel3;
    CButtonPanel *m_panel_removed;
};


class IMovableButton : public wxButton  // Taken from: http://wiki.wxwidgets.org/Dragging_a_wxWindow_around
{
private:
    void RemoveButton();
    int m_OrigId;
    wxPoint m_Position;
    bool m_dragging;
    bool m_locked;
    bool m_removed;
    wxSizer *m_Sizer, *m_TopSizer1, *m_TopSizer2, *m_TopSizer3, *m_TopSizerRemoved;
    wxWindow *m_win_top;
    wxPoint m_ScrollPos;
    wxMenu m_menu;
public:
    IMovableButton()  : wxButton() {}   
    IMovableButton(wxPanel* parent, wxWindowID id, const wxString &label=wxEmptyString, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0);
    virtual ~IMovableButton() {}
    virtual bool VerifySelection(CEditingBtnsPanel* e) = 0;
    virtual void SetLocked(bool locked);
    virtual void SetRemoved(bool removed);
    virtual void SetDragging(bool dragging);
    virtual void SetSizers(wxWindow *win_top=NULL, wxSizer *sizer1=NULL, wxSizer *sizer2=NULL, wxSizer *sizer3=NULL, wxSizer *sizer_removed_actual=NULL);
    virtual  wxPoint FindUnoccupiedPosition(wxSizer *sizer = NULL);
    virtual bool IsOccupiedPosition();
    virtual void OnRightClick(wxContextMenuEvent &evt);
    virtual void onMouseClick(wxCommandEvent& evt);
    virtual void MoveButton(wxPoint mousePos);
    virtual void OnKeyDown(wxKeyEvent& evt);
    virtual bool OnChar(wxKeyEvent& event);
    void SetScrollPos(wxPoint scrollpos);
    void OnPopupClick(wxCommandEvent &evt);
    void OnPopupClickUpdate(wxUpdateUIEvent& event);
    int GetOrigId() {return m_OrigId;}
    wxPoint GetOrigPosition() {return m_Position;}
    void SetOriginalPosition(wxPoint p) {m_Position = p;}
    void CopyButton(wxSizer *sizer, wxPoint p);
    void DeleteButton();
    void AddDynamicMenu();


  DECLARE_ABSTRACT_CLASS( IMovableButton )
};

class CButtonPanel : public wxScrolledWindow
{
private:
    IMovableButton *m_MovingButton;
public:
    CButtonPanel() : wxScrolledWindow() {}
    CButtonPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
    virtual ~CButtonPanel();
    virtual bool Layout();
    virtual void onMouseClick(wxMouseEvent& evt);
    virtual void OnKeyDown(wxKeyEvent& event);
    virtual bool OnChar(wxKeyEvent& event);
    void SetMovingButton(IMovableButton *btn, bool dragging);
    wxPoint GetScrollPos();
    IMovableButton *CreateButton(wxWindow *w, int x, int y);
    DECLARE_DYNAMIC_CLASS( CButtonPanel )
};

END_NCBI_SCOPE

#endif
    // _EDITINGBTNSPANEL_H_
