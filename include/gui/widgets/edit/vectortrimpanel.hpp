/* $Id: vectortrimpanel.hpp 38143 2017-04-03 16:11:47Z filippov $
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
 * Authors:  Yoon Choi
 */
#ifndef _VECTORTRIMPANEL_H_
#define _VECTORTRIMPANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/graph_ci.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/gauge.h>
#include <wx/listctrl.h>
#include <wx/html/htmlwin.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>

#include <algo/blast/format/vecscreen_run.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/wx/checkedlistctrl.hpp>
#include <gui/widgets/edit/vectorscreen.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxTextCtrl;
class wxStaticText;
class wxRadioBox;
////@end forward declarations

class wxButton;

BEGIN_NCBI_SCOPE

class CMatchesListCtrl;
class CLocationsListCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CVECTORTRIMPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CVECTORTRIMPANEL_TITLE _("Vector Trim Panel")
#define SYMBOL_CVECTORTRIMPANEL_IDNAME ID_CVECTORTRIMPANEL
#define SYMBOL_CVECTORTRIMPANEL_SIZE wxDefaultSize
#define SYMBOL_CVECTORTRIMPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CVectorTrimPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CVectorTrimPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CVectorTrimPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVectorTrimPanel();
    CVectorTrimPanel( wxWindow* parent, 
                      objects::CSeq_entry_Handle seh,
                      ICommandProccessor* cmdProcessor,
                      wxWindowID id = SYMBOL_CVECTORTRIMPANEL_IDNAME, 
                      const wxPoint& pos = SYMBOL_CVECTORTRIMPANEL_POSITION, 
                      const wxSize& size = SYMBOL_CVECTORTRIMPANEL_SIZE, 
                      long style = SYMBOL_CVECTORTRIMPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CVECTORTRIMPANEL_IDNAME, 
                 const wxPoint& pos = SYMBOL_CVECTORTRIMPANEL_POSITION, 
                 const wxSize& size = SYMBOL_CVECTORTRIMPANEL_SIZE, 
                 long style = SYMBOL_CVECTORTRIMPANEL_STYLE );

    /// Destructor
    ~CVectorTrimPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CVectorTrimPanel event handler declarations

////@end CVectorTrimPanel event handler declarations
    void OnRunVecscreen( wxCommandEvent& event );
    void OnSort( wxCommandEvent& event );
    void OnSelectAll( wxCommandEvent& event );
    void OnSelectStrongModerate( wxCommandEvent& event );
    void OnUnselectAll( wxCommandEvent& event );
    void OnUnselectInternal( wxCommandEvent& event );
    //void OnViewAlignmentGraphic( wxCommandEvent& event );
    void OnMakeReport( wxCommandEvent& event );
    void OnTrimSelected( wxCommandEvent& event );
    void OnDismiss( wxCommandEvent& event );
    void OnFindForward( wxCommandEvent& event );
    void OnFindReverse( wxCommandEvent& event );

////@begin CVectorTrimPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CVectorTrimPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void DisplaySummaryBySeqid(const TVecscreenSummaryBySeqid& 
                               vecscreen_summary);


////@begin CVectorTrimPanel member variables
    /// Control identifiers
    enum {
        ID_CVECTORTRIMPANEL = wxID_ANY
    };

    enum EInternalTrimType {
        eTrimToClosestEnd = 0,
        eTrimTo5PrimeEnd,
        eTrimTo3PrimeEnd,
    };
////@end CVectorTrimPanel member variables

private:
    typedef vector<SRangeMatch> TCuts;

    bool x_TrimSelected();
    void x_TrimSelected(CRef<CCmdComposite> command,
                        const CSeq_id* seqid,
                        const SVecscreenResult& vecres);
    void x_SetSeqData(const CBioseq_Handle& bsh,
                      CRef<objects::CSeq_inst> inst,
                      const TCuts& sorted_cuts);
    void x_TrimFeatureLocations(CRef<CSeq_feat> feat,
                                const TCuts& sorted_cuts,
                                const CSeq_id* seqid,
                                bool& bFeatureDeleted,
                                bool& bFeatureTrimmed,
                                bool& bProdDeleted,
                                bool& bProdTrimmed);
    void x_AdjustCdregionFrame(CScope& scope, 
                               CRef<CSeq_feat> feat,
                               const TCuts& sorted_cuts,
                               const CSeq_id* seqid,
                               bool bFeatureTrimmed,
                               bool bProdTrimmed);
    void x_SeqLocDelete(CRef<CSeq_loc> loc, 
                        TSeqPos from, TSeqPos to,
                        const CSeq_id* seqid,
                        bool& bCompleteCut,
                        bool& bTrimmed);
    void x_SeqIntervalDelete(CRef<CSeq_interval> interval, 
                             TSeqPos cut_from, TSeqPos cut_to,
                             const CSeq_id* seqid,
                             bool& bCompleteCut,
                             bool& bTrimmed);
    void x_AdjustInternalCutLocations(TCuts& cuts, TSeqPos seq_length);
    void x_FindCompleteDeletions(const TVecscreenSummaryBySeqid& seqidSummary,
                                 TVecscreenSummaryBySeqid& completeDeletions,
                                 TVecscreenSummaryBySeqid& trimOnly);
    bool x_IsCompleteCut(const TCuts& sorted_cuts, TSeqPos seq_length);
    void x_MergeCuts(TCuts& sorted_cuts);
    void x_DeleteSelected(CRef<CCmdComposite> command,
                          const CSeq_id* seqid);
    void x_RetranslateCDS(CScope& scope,
                          CRef<CCmdComposite> command,
                          CRef<CSeq_feat> cds);
    void x_AdjustDensegAlignment(CRef<CSeq_align> align,
                                 CDense_seg::TDim row,
                                 const TCuts& sorted_cuts);
    void x_CutDensegSegment(CRef<CSeq_align> align,
                            CDense_seg::TDim row,
                            TSeqPos pos);
    bool x_FindSegment(const CDense_seg& denseg,
                       CDense_seg::TDim row,
                       TSeqPos pos,
                       CDense_seg::TNumseg& seg,
                       TSeqPos& seg_start) const;
    void x_TrimAlignments(CRef<CCmdComposite> command,
                          const TVecscreenSummaryBySeqid& trimOnly);
    void x_GetSortedCuts(TCuts& sorted_cuts,
                         const CSeq_id* seqid);
    void x_TrimSeqGraphs(CRef<CCmdComposite> command,
                         const TVecscreenSummaryBySeqid& trimOnly);
    void x_TrimSeqGraphData(CRef<CSeq_graph> new_graph, 
                            const CMappedGraph& orig_graph,
                            const TCuts& sorted_cuts,
                            const CSeq_id* seqid);
    void x_UpdateSeqGraphLoc(CRef<CSeq_graph> new_graph, 
                             const TCuts& sorted_cuts,
                             const CSeq_id* seqid);
    void x_GetTrimCoordinates(const TCuts& sorted_cuts, 
                              const CSeq_id* seqid,
                              TSeqPos& trim_start,
                              TSeqPos& trim_stop);

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    ICommandProccessor* m_CmdProcessor;
    CVectorScreen m_vectorscreen;

    //
    // Layout of elements in this panel
    //

    // Row 1 - display results
    wxBoxSizer*         m_sizerVectorResults;
    CMatchesListCtrl*   m_listMatches;
    // The following 2 elements are hidden/shown in the same location
    // depending on a toggle.
    wxHtmlWindow*       m_viewAlignGraphic;
    CLocationsListCtrl* m_listLocations;

    // Row 2 - run blast vecscreen against UniVec db
    wxButton* m_buttonRunVecscreen;
    wxGauge*  m_gaugeProgressBar;

    // Row 3 - Find text
    wxTextCtrl* m_textFind;
    wxButton* m_buttonFindForward;
    wxButton* m_buttonFindReverse;

    // Row 4 - Sort results
    wxRadioBox* m_radioBoxSort;

    // Row 5 - Internal Trim Options
    wxRadioBox* m_radioBoxInternalTrimOptions;

    // Row 6 - select results
    wxButton* m_buttonSelectAll;
    wxButton* m_buttonUnselectAll;
    wxButton* m_buttonUnselectInternal;
    wxButton* m_buttonSelectStrongModerate;
    wxCheckBox* m_checkboxViewGraphic;

    wxCheckBox*  m_CitSub;

    // Row 7 - Trim selected sequences
    wxButton* m_buttonMakeReport;
    wxButton* m_buttonTrimSelected;
    wxButton* m_buttonDismiss;

    // Parent window
    wxWindow* m_pParentWindow;
};


class CMatchesListCtrl: public wxCheckedListCtrl
{
public:
    enum ESortOption {
        eSortInternal5Prime3Prime,
        eSortStrength,
        eSortMarked,
        eSortAccession
    };

    CMatchesListCtrl();
    CMatchesListCtrl(wxWindow *parent,
                     const wxWindowID id,
                     const wxPoint& pos=wxDefaultPosition,
                     const wxSize& size=wxDefaultSize,
                     long style=wxLC_ICON);
    ~CMatchesListCtrl();
 
    bool Create(wxWindow *parent, 
                wxWindowID id,
                const wxPoint &pos=wxDefaultPosition, 
                const wxSize &size=wxDefaultSize, 
                long style=wxLC_ICON);

    void SetAlignGraphicView( wxHtmlWindow* view );
    void SetRangeView( CLocationsListCtrl* view );

    const TVecscreenSummaryBySeqid& GetSelectedVecscreenSummaryBySeqid();

    bool UpdateData(const TVecscreenSummaryBySeqid& 
                    vecscreen_summary);

    void SelectAll();
    void SelectStrongModerate();
    void UnselectAll();
    void UnselectInternal();

    void OnColClick(wxListEvent& event);
    void OnSelected(wxListEvent& event);
    void OnChecked(wxListEvent& event);

    void Sort(ESortOption eSortOption, bool bAscending);

    void GetReport(wxString& report);
    void GetSelectedLocations(wxString& report, CVectorTrimPanel::EInternalTrimType eInternalTrimType);
    void FindForward(wxString searchstr);
    void FindReverse(wxString searchstr);

    bool AllSelected();

protected:
    class CUVHitLoc 
    {
    public:
        // Also used for sort order
        enum EHitLoc {
            // Ascending order of display, e.g., internal before 5' before 3'
            eInternal,
            e5Prime,
            e3Prime, 
            eUnknown
        };

        CUVHitLoc(bool selected, EHitLoc location, const SVecscreenResult& vecres,
                  const CSeq_id* seqid, const vector<SRangeMatch>& matches)
          : m_Selected(selected),
            m_HitLocation(location),
            m_FastaSeqid(vecres.m_FastaSeqid),
            m_Seqid(seqid),
            m_SeqLen(vecres.m_SeqLen),
            m_matches(matches)
        {}; 

        // The following members map to the 4-column display :
        // 1) Selection
        // 2) Hit location (5', 3', Internal)
        // 3) Seqid
        // 4) Match strength(s) - abutting hits are displayed together
        //    e.g., "strong" (single hit) or "strong;suspect" (2 abutting hits)

        // 1) Selected in the gui display or not
        bool m_Selected;

        // 2) Hit location
        EHitLoc m_HitLocation;

        // 3) Fasta format seqid string
        string m_FastaSeqid;
        const CSeq_id* m_Seqid;  
        TSeqPos m_SeqLen;

        // 4) Keeps track of individual hit location and type.
        //    Abutting hits are kept together in the same container.
        vector<SRangeMatch> m_matches;
    };
    typedef vector<CUVHitLoc> TVecscreenSummaryByLocation;

    class CUVHitCmp : public binary_function<CUVHitLoc, CUVHitLoc, bool>
    {
    public:
        CUVHitCmp(ESortOption eSortOption, bool bAscending) 
          : m_SortOption(eSortOption),
            m_SortAscending(bAscending)
        {}

        bool operator()(const CUVHitLoc& x, const CUVHitLoc& y) const;

    protected:
        int x_Compare(const CUVHitLoc& x, const CUVHitLoc& y) const;
        int x_CompareMatches(const vector<SRangeMatch>& x, const vector<SRangeMatch>& y) const;
        void x_FindStrongestMatchType(const vector<SRangeMatch>& matches, 
                                      SRangeMatch::EMatchType& matchtype) const;

    private:
        ESortOption m_SortOption;
        bool m_SortAscending;
    };

    bool x_GetSelected(const CUVHitLoc& hitloc);
    wxString x_GetHitLocation(const CUVHitLoc& hitloc);
    wxString x_GetSeqid(const CUVHitLoc& hitloc);
    wxString x_GetMatchType(const CUVHitLoc& hitloc);

    void x_Redraw();

    // Methods to transform data from external data type to 
    // internal data type and vice versa
    void x_ConvertSelectedToBySeqid(const TVecscreenSummaryByLocation& seqidSummaryByLocation, 
                                    TVecscreenSummaryBySeqid& seqidSummaryBySeqid) const;
    void x_ConvertAllToByLocation(const TVecscreenSummaryBySeqid& vecscreen_summary, 
                                  TVecscreenSummaryByLocation& seqidSummaryByLocation) const;
    void x_InsertMatches(TVecscreenSummaryByLocation& seqidSummaryByLocation,
                         const vector<SRangeMatch>& sorted_matches,
                         const CSeq_id* seqid,
                         const SVecscreenResult& vecres) const;

    bool x_Find(const CUVHitLoc& hitloc, const wxString& searchstr);
    bool x_IsValidRow(int row);
    void x_DisplaySearchResult(bool bFound, int row);

private:
    wxListItemAttr m_listItemAttr;

    // Uses data type known by external callers.
    // Vecscreen summary reorganized around seqid.
    TVecscreenSummaryBySeqid m_seqidSummaryBySeqid;

    // Uses data type known by this class only, for data display.
    // Vecscreen summary organized around hit location.
    TVecscreenSummaryByLocation m_seqidSummaryByLocation;

    // Show alignment graphic
    wxHtmlWindow* m_AlignView;
    CLocationsListCtrl* m_RangeView;

    // Sort options
    ESortOption m_SortOption;
    bool m_SortAscending;

    // "Find Text" functionality - keep track of selected row
    int m_SelectedRow;

    // Do not allow copy/assignment
    wxDECLARE_NO_COPY_CLASS(CMatchesListCtrl);

    DECLARE_DYNAMIC_CLASS( CMatchesListCtrl )
    DECLARE_EVENT_TABLE()
};


class CLocationsListCtrl: public wxListCtrl
{
public:
    CLocationsListCtrl();
    CLocationsListCtrl(wxWindow *parent,
                       const wxWindowID id,
                       const wxPoint& pos=wxDefaultPosition,
                       const wxSize& size=wxDefaultSize,
                       long style=wxLC_ICON);
    ~CLocationsListCtrl();

    bool Create(wxWindow *parent, 
                wxWindowID id,
                const wxPoint &pos=wxDefaultPosition, 
                const wxSize &size=wxDefaultSize, 
                long style=wxLC_ICON);

    void OnSelected(wxListEvent& event);

    bool UpdateData(const string& fastaseqid,
                    const vector<SRangeMatch>& matches);

private:
    wxListItemAttr m_listItemAttr;

    // Do not allow copy/assignment
    wxDECLARE_NO_COPY_CLASS(CLocationsListCtrl);

    DECLARE_DYNAMIC_CLASS( CLocationsListCtrl )
    DECLARE_EVENT_TABLE()
};


#define SYMBOL_CLISTREPORTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CLISTREPORTDLG_TITLE _("Report Dialog")
#define SYMBOL_CLISTREPORTDLG_SIZE wxDefaultSize

class CListReportDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CListReportDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CListReportDlg();
    CListReportDlg( wxWindow* parent, 
                    wxWindowID id = wxID_ANY, 
                    const wxString& caption = SYMBOL_CLISTREPORTDLG_TITLE, 
                    const wxPoint& pos = wxDefaultPosition, 
                    const wxSize& size = SYMBOL_CLISTREPORTDLG_SIZE, 
                    long style = SYMBOL_CLISTREPORTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxString& caption = SYMBOL_CLISTREPORTDLG_TITLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = SYMBOL_CLISTREPORTDLG_SIZE,
                 long style = SYMBOL_CLISTREPORTDLG_STYLE );

    /// Destructor
    ~CListReportDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetHeader(const wxString& header);
    void SetList(const wxString& list);
    void SetFooter(const wxString& footer);

private:
    wxStaticText* m_textHeader;
    wxTextCtrl*   m_textList;
    wxStaticText* m_textFooter;
};


#define SYMBOL_CVECSCREENREPORTPANEL_STYLE wxBORDER_SIMPLE
#define SYMBOL_CVECSCREENREPORTPANEL_SIZE wxDefaultSize

class CVecscreenReportPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CVecscreenReportPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVecscreenReportPanel();
    CVecscreenReportPanel( wxWindow* parent, 
                      wxWindowID id = wxID_ANY, 
                      const wxPoint& pos = wxDefaultPosition, 
                      const wxSize& size = SYMBOL_CVECSCREENREPORTPANEL_SIZE, 
                      long style = SYMBOL_CVECSCREENREPORTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = SYMBOL_CVECSCREENREPORTPANEL_SIZE,
                 long style = SYMBOL_CVECSCREENREPORTPANEL_STYLE );

    /// Destructor
    ~CVecscreenReportPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetReport(const wxString& report);

private:
    wxTextCtrl* m_textReport;
};


#define SYMBOL_CVECSCREENREPORT_STYLE wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER
#define SYMBOL_CVECSCREENREPORT_TITLE _("VecScreen Report")
#define SYMBOL_CVECSCREENREPORT_SIZE wxDefaultSize

class CVecscreenReport: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CVecscreenReport )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CVecscreenReport();
    CVecscreenReport( wxWindow* parent, 
                      wxWindowID id = wxID_ANY, 
                      const wxString& title = SYMBOL_CVECSCREENREPORT_TITLE, 
                      const wxPoint& pos = wxDefaultPosition, 
                      const wxSize& size = SYMBOL_CVECSCREENREPORT_SIZE, 
                      long style = SYMBOL_CVECSCREENREPORT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
                 wxWindowID id = wxID_ANY, 
                 const wxString& title = SYMBOL_CVECSCREENREPORT_TITLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = SYMBOL_CVECSCREENREPORT_SIZE,
                 long style = SYMBOL_CVECSCREENREPORT_STYLE );

    /// Destructor
    ~CVecscreenReport();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetReport(const wxString& report);

private:
    CVecscreenReportPanel* m_panel;
};


END_NCBI_SCOPE

#endif
    // _VECTORTRIMPANEL_H_
