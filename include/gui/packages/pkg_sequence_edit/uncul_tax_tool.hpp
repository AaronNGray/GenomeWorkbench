
/*  $Id: uncul_tax_tool.hpp 42425 2019-02-21 15:31:49Z asztalos $
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

#ifndef _UNCUL_TAX_TOOL_H_
#define _UNCUL_TAX_TOOL_H_

#include <corelib/ncbistd.hpp>

#include <gui/utils/command_processor.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/taxon3/Taxon3_reply.hpp>

#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_UNCUL_TAX_TOOL 11300
#define SYMBOL_UNCUL_TAX_TOOL_STYLE wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_UNCUL_TAX_TOOL_TITLE _("Uncultured Taxonomy Tool")
#define SYMBOL_UNCUL_TAX_TOOL_IDNAME ID_UNCUL_TAX_TOOL
#define SYMBOL_UNCUL_TAX_TOOL_SIZE wxSize(1400, 840)
#define SYMBOL_UNCUL_TAX_TOOL_POSITION wxDefaultPosition
////@end control identifiers
#define ID_ADD_SP_UNCUL_TAXTOOL 11301
#define ID_ADD_BACTERIUM_UNCUL_TAXTOOL 11302
#define ID_APPLY_BTN 11303
#define ID_CANCEL_BTN 11304
#define ID_REFRESH_BTN 11305
#define ID_TRIM_BTN 11306

class CUnculTaxTool : public wxFrame
{    
    DECLARE_DYNAMIC_CLASS( CUnculTaxTool )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CUnculTaxTool();
    CUnculTaxTool( wxWindow* parent, objects::CSeq_entry_Handle seh, IWorkbench* workbench, wxWindowID id = SYMBOL_UNCUL_TAX_TOOL_IDNAME, const wxString& caption = SYMBOL_UNCUL_TAX_TOOL_TITLE, const wxPoint& pos = SYMBOL_UNCUL_TAX_TOOL_POSITION, const wxSize& size = SYMBOL_UNCUL_TAX_TOOL_SIZE, long style = SYMBOL_UNCUL_TAX_TOOL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_UNCUL_TAX_TOOL_IDNAME, const wxString& caption = SYMBOL_UNCUL_TAX_TOOL_TITLE, const wxPoint& pos = SYMBOL_UNCUL_TAX_TOOL_POSITION, const wxSize& size = SYMBOL_UNCUL_TAX_TOOL_SIZE, long style = SYMBOL_UNCUL_TAX_TOOL_STYLE );

    /// Destructor
    ~CUnculTaxTool();

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

    CRef<CCmdComposite> GetCommand();

    CRef<objects::CSeq_table> GetValuesTableFromSeqEntry();
    CRef<CCmdComposite> GetCommandFromValuesTable(CRef<CSeq_table> values_table);
    CRef<objects::CSeq_table> GetChoices(CRef<objects::CSeq_table> values_table) { CRef<objects::CSeq_table> choices; return choices; } ;
    int GetCollapsible() {return 0;}
    bool IsReadOnlyColumn(string column_name) { if (column_name == "Taxname" || column_name.empty() || column_name == "expand") return true; return false; };

    void OnAddSp( wxCommandEvent& event );
    void OnAddBacterium( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );
    void OnApply( wxCommandEvent& event );
    void OnRefreshBtn( wxCommandEvent& event );
    void TrimSuggestions(wxCommandEvent& event );
private:

    void AddBioSource(const objects::CSeq_entry& seq, const objects::CSeqdesc& desc);
    void GetDesc(const CSeq_entry& se);
    void FindBioSource(objects::CSeq_entry_Handle tse);
    
    bool OkToTaxFix(const string& taxname);
    void LookupSuggestions(vector<string> &suggestions, vector<bool> &verified);
    string StandardFixes(const CBioSource &biosource);
    bool IsSpeciesSpecific(const CBioSource &biosource);
    string MakeUnculturedName(const string &taxname, const string suffix = "");
    bool CompareOrgnameLineage(CRef<CT3Reply> reply, const string &lineage);
    string GetSuggestion(CRef<CT3Reply> reply);
    string GetRank(CRef<CT3Reply> reply);
    CRef<CT3Reply> GetReply(const CBioSource &biosource, const string &standard_taxname);
    bool CheckSuggestedFix(const CBioSource &biosource, string &suggestion);
    bool IsAmbiguous(CRef<CT3Reply> reply);
    string TryRankFix(CRef<CT3Reply>  reply, unsigned int i, string &name);
    int GetColumn();
    void ConvertSpeciesSpecificNote(CBioSource &biosource, const string &new_note);
    void PreloadCache();

    objects::CSeq_entry_Handle m_TopSeqEntry;

    vector<pair<CConstRef<objects::CSeq_entry>, CConstRef<objects::CSeqdesc>>> m_BioSource;
    
    map<string, CRef<CT3Reply> > m_ReplyCache;
    wxGrid* m_Grid;
    CSeqTableGridPanel* m_GridPanel;
    IWorkbench*     m_Workbench;
    set<unsigned int> m_ConvertNote, m_RemoveNote;


    struct SUnindexedObject {
        CRef<objects::CBioSource> actual_obj;
        CConstRef<objects::CBioSource> orig_obj;
        CConstRef<objects::CBioSource> new_obj;
    };

    typedef vector<SUnindexedObject> TUnindexedObjects;
    TUnindexedObjects m_UnindexedObjects;
};


END_NCBI_SCOPE

#endif
    // _UNCUL_TAX_TOOL_H_
