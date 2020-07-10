#ifndef GUI_WIDGETS_EDIT___MACRO_PANEL_LOADER__HPP
#define GUI_WIDGETS_EDIT___MACRO_PANEL_LOADER__HPP

/*  $Id: macro_panel_loader.hpp 44731 2020-03-03 15:53:48Z asztalos $
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
* Authors:  Vladislav Evgeniev
*/

#include <string>
#include <vector>
#include <map>
#include <wx/window.h>
#include <wx/panel.h>
#include <wx/event.h>

#include <gui/widgets/edit/macro_argument.hpp>
#include <gui/widgets/edit/publisher_subscriber.hpp>

class wxControl;
class wxListBox;
class wxRadioBox;
class wxRadioButton;
class wxTextCtrl;
class wxCheckBox;
class wxComboBox;
class wxChoice;
class wxCheckListBox;
class wxChoice;
class wxButton;
class wxToggleButton;
class wxStaticText;

BEGIN_NCBI_SCOPE

enum class ECustomPanelType
{
    eAddAuthorsPanel,
    eAddStructCommPanel,
    eAddAssmGapsByNPanel,
    eApplySrcQualTable,
    eApplyStrCommTable,
    eApplyPubQualTable,
    eApplyDBLinkTable,
    eApplyMolinfoQualTable,
    eApplyFeatQualTable,
    eApplyCDSGeneProtTable,
    eApplyRNAQualTable,
    eApplyMiscQualTable
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CMacroPanelLoader
    : public wxEvtHandler
    , public ISubscriberT<bool>
{
public:
    ~CMacroPanelLoader();

    static void Initialize();

    CMacroPanelLoader(wxWindow *parent, const string &panel_name, const TArgumentsVector &arguments);
    
    void SetControlValues(const string &ctrl_name, const vector<string> &values);
    void SetSelection(const string &ctrl_name, int n);

    wxPanel* GetPanel() { return m_Panel; }
    const CArgumentList& GetArgumentList() const { return m_Arguments; }
    bool TransferDataFromPanel();

    wxPanel* LoadCustomPanel(ECustomPanelType type);
protected:
    void OnComboBoxSelectionChanged(wxCommandEvent& event);
    void OnChoiceSelectionChanged(wxCommandEvent& event);
    void OnTextCtrlTextChanged(wxCommandEvent& event);
    void OnListBoxSelectionChanged(wxCommandEvent& event);
    void OnRadioBoxClicked(wxCommandEvent& event);
    void OnRadiobuttonClicked(wxCommandEvent& event);
    void OnButtonClicked(wxCommandEvent& event);
    void OnToggleButtonClicked(wxCommandEvent& event);
    void OnCheckBoxClicked(wxCommandEvent& event);
    void OnCheckListBoxSelectionChanged(wxCommandEvent& event);

    void Update(const bool& value);

protected:
    void BindEvents(wxControl& ctrl);
    wxControl& FindControl(const string &name);
    void Initialize(wxListBox &list_box, const vector<string> &values);
    void Initialize(wxRadioBox &radio_box, const vector<string> &values);
    void Initialize(wxTextCtrl &txt_ctrl, const vector<string> &values);
    void Initialize(wxComboBox &combo_box, const vector<string> &values);
    void Initialize(wxCheckListBox& chlist_box, const vector<string>& values);
    void Initialize(wxChoice& choice, const vector<string>& values);
    void Initialize(wxStaticText& st_text, const vector<string>& values);

    bool GetCtrlValue(TStdStringPublisher &value, const wxTextCtrl &txt_ctrl);
    bool GetCtrlValue(TStdStringPublisher &value, const wxListBox &lst_box);
    bool GetCtrlValue(TStdStringPublisher &value, const wxCheckListBox &chklist_box);
    bool GetCtrlValue(TStdStringPublisher &value, const wxRadioBox &radio_box);
    bool GetCtrlValue(TStdStringPublisher &value, const wxRadioButton &radio_btn);
    bool GetCtrlValue(TStdStringPublisher &value, const wxCheckBox &chk_box);
    bool GetCtrlValue(TStdStringPublisher &value, const wxComboBox &combo_box);
    bool GetCtrlValue(TStdStringPublisher &value, const wxChoice &choice);
    
    void ToArrayString(const vector<string>& in, wxArrayString& out);

private:
    string x_GetRadioboxSelection(const wxRadioBox& radio_box);
    string x_GetRadiobuttonSelection(const wxRadioButton& radio_btn);
    string x_GetCheckboxSelection(const wxCheckBox& chk_box);
    string x_GetCheckListBoxSelection(const wxCheckListBox& chklist_box);

    void x_UpdateBsrcPanelControls();

    map<string,wxControl*>  m_Controls;
    CArgumentList           m_Arguments;
    wxPanel*                m_Panel{ nullptr };
    static bool             m_Initialized;
    DECLARE_EVENT_TABLE();
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_EDIT___MACRO_PANEL_LOADER__HPP
