#ifndef GUI_WIDGETS_EDIT___LOCATION_LIST_CTRL__HPP
#define GUI_WIDGETS_EDIT___LOCATION_LIST_CTRL__HPP

/*  $Id: locationl_list_ctrl.hpp 42869 2019-04-23 12:24:11Z filippov $
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
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objmgr/scope.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>

#include "wx/statline.h"
#include "wx/valgen.h"


class wxHyperlinkEvent;
class wxScrolledWindow;
class wxFlexGridSizer;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CLOCATIONLISTCTRL_STYLE wxSUNKEN_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_CLOCATIONLISTCTRL_TITLE _("Location List Control")
#define SYMBOL_CLOCATIONLISTCTRL_IDNAME ID_CLOCATIONLISTCTRL
#define SYMBOL_CLOCATIONLISTCTRL_SIZE wxDefaultSize
#define SYMBOL_CLOCATIONLISTCTRL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLocationListCtrl class declaration
 */

class CLocationListCtrl: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CLocationListCtrl )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLocationListCtrl();
    CLocationListCtrl( wxWindow* parent, CRef<objects::CScope> scope, 
                       bool allow_nuc, bool allow_prot, bool is_aa,
                       wxWindowID id = SYMBOL_CLOCATIONLISTCTRL_IDNAME, 
                       const wxPoint& pos = SYMBOL_CLOCATIONLISTCTRL_POSITION, 
                       const wxSize& size = SYMBOL_CLOCATIONLISTCTRL_SIZE, 
                       long style = SYMBOL_CLOCATIONLISTCTRL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CLOCATIONLISTCTRL_IDNAME, 
        const wxPoint& pos = SYMBOL_CLOCATIONLISTCTRL_POSITION, 
        const wxSize& size = SYMBOL_CLOCATIONLISTCTRL_SIZE, 
        long style = SYMBOL_CLOCATIONLISTCTRL_STYLE );

    /// Destructor
    ~CLocationListCtrl();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CLocationListCtrl event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnSortClick( wxCommandEvent& event );

////@end CLocationListCtrl event handler declarations

////@begin CLocationListCtrl member function declarations

    int GetLocType() const { return m_LocType ; }
    void SetLocType(int value) { m_LocType = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocationListCtrl member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnDelete(wxHyperlinkEvent& event);
    void OnSpinCtrl(wxCommandEvent& evt);
    void OnStrandChange(wxCommandEvent& event);
    void OnNewText(wxCommandEvent& event);

    void AddRow(int from, int to, int strand, const  wxString& seqID, const bool fuzz);
    void AddEmptyRow();
    void ClearRows();
    void SetPartials(bool partial5, bool partial3);

    CRef<objects::CSeq_loc> GetSeq_loc();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void PopulateIdListFromScope();
    bool IsInIdList(wxString str);
    wxString GetListIdStrForSeqId(const objects::CSeq_id& seq_id);

    wxScrolledWindow* m_ScrolledWindow;
    wxCheckBox* m_Partial5;
    wxCheckBox* m_Partial3;

private:
    int m_LocType;

    enum {
        ID_CLOCATIONLISTCTRL = 10032,
        ID_PANEL1 = 10039,
        ID_SCROLLEDWINDOW1 = 10035,
        ID_CHECKBOX1 = 10025,
        ID_CHECKBOX2 = 10109,
        ID_RADIOBOX6 = 10040,
        ID_BUTTON1 = 10038
    };

    wxFlexGridSizer* m_Sizer;

    struct SIntData
    {
        int      m_From;
        int      m_To;
        int      m_Strand;
        wxString m_SeqId;
        bool     m_Empty;
        bool     m_Fuzz;

        bool operator==(const SIntData& intData) const
            { return m_SeqId== intData.m_SeqId && m_From == intData.m_From && m_To == intData.m_To && m_Fuzz == intData.m_Fuzz;  }
        bool operator<(const SIntData& intData) const
            {
              if (m_SeqId == intData.m_SeqId) {
                  if (m_From < intData.m_From) return true;
                  if (m_From == intData.m_From)
                      return m_To < intData.m_To; 
                  return false;
              }
              else
                  return (m_SeqId < intData.m_SeqId);
            }
    };

    void x_AddRow(int from, int to, int strand, const wxString& seqID, const bool fuzz, size_t rowPos = -1);
    void x_AddEmptyRow(size_t rowPos = -1);
    objects::ENa_strand GetStrandFromForm(size_t index);
    objects::ENa_strand x_GetStrandFromForm(size_t index);
    CRef<objects::CSeq_loc> x_GetInterval(size_t index, bool int_only);
    CRef<objects::CSeq_id> GetIdFromForm(size_t index);
    void DeleteRow(wxSizerItemList::iterator row, wxSizerItemList& itemList);
    void x_GetRowData(wxSizerItemList::iterator row, wxSizerItemList& itemList,
                      int& from, int& to, int& strand, wxString& seqID, bool &fuzz);
    wxSizerItemList::iterator x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
    wxSizerItemList::iterator x_NextRow(wxSizerItemList::iterator row, wxSizerItemList& itemList);
    wxSizerItemList::iterator x_PrevRow(wxSizerItemList::iterator row, wxSizerItemList& itemList);

    vector<SIntData> m_Data;
    wxArrayString m_SeqIds;

    bool m_AllowNuc;
    bool m_AllowProt;
    CRef<objects::CScope> m_Scope;
    bool m_is_aa;
    map<string, CConstRef<objects::CSeq_id> > m_str_to_id;
    wxTextCtrl* m_LastFrom;
    wxTextCtrl* m_LastTo;
    CNoTabChoice* m_LastStrand;
    int m_LastStrandSelection;
    CNoTabComboBox* m_LastId;
    int m_LastIdSelection;
    wxString m_LastIdValue;
    int m_CtrlColNum;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___LOCATION_LIST_CTRL__HPP
