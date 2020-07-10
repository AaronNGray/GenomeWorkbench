/*  $Id: barcode_tool.hpp 44314 2019-11-27 14:31:38Z filippov $
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

#ifndef _BARCODE_TOOL_H_
#define _BARCODE_TOOL_H_

#include <corelib/ncbistd.hpp>

#include <gui/utils/command_processor.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objtools/validator/validator_barcode.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <wx/imaglist.h>

#include <wx/scrolwin.h>
#include <wx/vscroll.h>
#include <wx/textwrapper.h>
#include <wx/dcbuffer.h>
#include <wx/platform.h>

BEGIN_NCBI_SCOPE

class CBarcodePanel;

#define ID_BARCODE_TOOL_SELECT 12000
#define ID_BARCODE_TOOL_SELECT_ALL 12001
#define ID_BARCODE_TOOL_UNSELECT_ALL 12002
#define ID_BARCODE_TOOL_DISMISS 12003
#define ID_BARCODE_TOOL_APPLY_DBXREFS 12004
#define ID_BARCODE_TOOL_REMOVE_KEYWORD 12005
#define ID_BARCODE_TOOL_ADD_KEYWORD 12006

class CBarcodeTool: public wxFrame
{
 DECLARE_DYNAMIC_CLASS( CBarcodeTool )
 DECLARE_EVENT_TABLE()

public:
    CBarcodeTool() {  Init(); }
    CBarcodeTool( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor, IWorkbench* wb, 
                  wxWindowID id = wxID_ANY, const wxString& caption = _("BARCODE Discrepancy Tool"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(1350,470), long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL|wxBG_STYLE_PAINT);
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = _("BARCODE Discrepancy Tool"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(1350,470), long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL|wxBG_STYLE_PAINT );       
    virtual ~CBarcodeTool() {}
    bool ShowToolTips()  { return true; }
    wxBitmap GetBitmapResource( const wxString& name ) {   wxUnusedVar(name);   return wxNullBitmap; }
    wxIcon GetIconResource( const wxString& name ) {  wxUnusedVar(name);   return wxNullIcon; }
    virtual void Init();
    void CreateControls();    
    void OnSelect( wxCommandEvent& event );
    void OnSelectAll( wxCommandEvent& event );
    void OnUnselectAll( wxCommandEvent& event );
    void OnDismiss( wxCommandEvent& event );
    void CheckRow(int i);
    void OnAddKeyword( wxCommandEvent& event );
    void OnRemoveKeyword( wxCommandEvent& event );
    void OnApplyDbxrefs( wxCommandEvent& event );
    void ClickLink( int i );

    static CRef<CCmdComposite> ApplyDbxrefs(CSeq_entry_Handle seh);

private:
   

    void FindBarcodeFeats(CBioseq_Handle bsh);
    bool GetHasKeyword(CBioseq_Handle bsh);

    void GetValues(void);
    void ShowReport(void);

    wxRadioBox *m_Radiobox;
    wxChoice *m_Choice;
    CStringConstraintPanel* m_StringConstraintPanel;
    validator::TBarcodeResults m_barcode;
    set<int> m_selected;
    ICommandProccessor* m_CmdProcessor;
    CSeq_entry_Handle m_TopSeqEntry;
    IWorkbench* m_Workbench;
    CBarcodePanel *m_scrolled;
    wxGridSizer *m_grid;
};

class CBarcodePanel : public wxVScrolledWindow
{
    DECLARE_DYNAMIC_CLASS( CBarcodePanel )
    DECLARE_EVENT_TABLE()
public:
    CBarcodePanel() : wxVScrolledWindow(), m_barcode(nullptr), m_selected(nullptr), m_RowHeight(20) {}
    CBarcodePanel(wxWindow *parent, validator::TBarcodeResults *barcode, set<int> *selected,
                   wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
    virtual ~CBarcodePanel();
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnEraseBackground(wxEraseEvent& event);
    virtual wxCoord OnGetRowHeight( size_t row ) const;

    virtual void OnUpdate();

    void OnResize(wxSizeEvent&);
    void OnMouseClick(wxMouseEvent& evt);

private:
    validator::TBarcodeResults *m_barcode;
    set<int> *m_selected;
    int m_RowHeight;
    vector<pair<size_t, wxRect> > m_checkboxes;
    vector<pair<size_t, wxRect> > m_links;
};


class CApplyFBOL
{
public:
    static CRef<CCmdComposite> apply(objects::CSeq_entry_Handle seh);
};

END_NCBI_SCOPE

#endif
    // _BARCODE_TOOL_H_
