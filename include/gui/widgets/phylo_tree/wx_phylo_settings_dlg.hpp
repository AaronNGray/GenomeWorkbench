/*  $Id: wx_phylo_settings_dlg.hpp 38346 2017-04-27 13:24:57Z falkrb $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */
#ifndef _WX_PHYLO_SETTINGS_DLG_H_
#define _WX_PHYLO_SETTINGS_DLG_H_


#include <corelib/ncbistl.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/valgen.h>
#include <wx/statline.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/notebook.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxColourPickerCtrl;
class wxHyperlinkCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define ID_CWXPHYLOSETTINGSDLG 10000
#define ID_LISTBOOK1 10013
#define ID_PANEL1 10019
#define ID_COLORATION_RADIOBOX 10021
#define ID_TEXTCTRL2 10015
#define ID_TEXTCTRL3 10016
#define ID_TEXTCTRL4 10022
#define ID_TEXTCTRL5 10023
#define ID_TEXTCTRL6 10024
#define ID_TEXTCTRL7 10025
#define ID_TEXTCTRL8 10026
#define ID_TEXTCTRL9 10027
#define ID_TEXTCTRL10 10028
#define ID_TEXTCTRL11 10030
#define ID_TEXTCTRL12 10031
#define ID_COLOURPICKERCTRL 10014
#define ID_COLOURPICKERCTRL1 10032
#define ID_COLOURPICKERCTRL2 10033
#define ID_PANEL 10043
#define ID_SELECTION_ALPHA_SLIDER 10017
#define ID_PANEL2 10029
#define ID_RADIOBUTTON1 10034
#define ID_CHOICE1 10035
#define ID_RADIOBUTTON2 10036
#define ID_CHOICE2 10037
#define ID_BUTTON1 10038
#define ID_TEXTCTRL1 10039
#define ID_MAX_LABEL_LEN_CHECK 10047
#define ID_MAX_LABEL_LEN_VALUE 10048
#define ID_TEXTCTRL13 10040
#define ID_BUTTON2 10041
#define ID_PANEL3 10001
#define ID_TEXTCTRL19 10006
#define ID_TEXTCTRL20 10007
#define ID_TEXTCTRL21 10008
#define ID_TEXTCTRL22 10009
#define ID_TEXTCTRL17 10004
#define ID_TEXTCTRL23 10049
#define m_TEXTCTRL24 10050
#define ID_TEXTCTRL18 10005
#define ID_CHECKBOX 10051
#define ID_CHOICE 10044
#define m_CHOICE 10045
#define ID_HYPERLINKCTRL 10052
#define SYMBOL_CWXPHYLOSETTINGSDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXPHYLOSETTINGSDLG_TITLE _("Properties")
#define SYMBOL_CWXPHYLOSETTINGSDLG_IDNAME ID_CWXPHYLOSETTINGSDLG
#define SYMBOL_CWXPHYLOSETTINGSDLG_SIZE wxDefaultSize
#define SYMBOL_CWXPHYLOSETTINGSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxPhyloSettingsDlg class declaration
 */

class CwxPhyloSettingsDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CwxPhyloSettingsDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxPhyloSettingsDlg();
    CwxPhyloSettingsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXPHYLOSETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_CWXPHYLOSETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_CWXPHYLOSETTINGSDLG_POSITION, const wxSize& size = SYMBOL_CWXPHYLOSETTINGSDLG_SIZE, long style = SYMBOL_CWXPHYLOSETTINGSDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXPHYLOSETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_CWXPHYLOSETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_CWXPHYLOSETTINGSDLG_POSITION, const wxSize& size = SYMBOL_CWXPHYLOSETTINGSDLG_SIZE, long style = SYMBOL_CWXPHYLOSETTINGSDLG_STYLE );

    ~CwxPhyloSettingsDlg();

    void Init();

    void CreateControls();

    void SetParams(CPhyloTreeDataSource* ds, CPhyloTreeScheme* sl);

    void SwitchMode(bool bmode);

    void UpdateSample();

    void GetRandomNode();

////@begin CwxPhyloSettingsDlg event handler declarations

    void OnListbook1PageChanged( wxNotebookEvent& event );

    void OnColorationRadioboxSelected( wxCommandEvent& event );

    void OnSelectionAlphaSliderUpdated( wxCommandEvent& event );

    void OnRadiobutton1Selected( wxCommandEvent& event );

    void OnChoice1Selected( wxCommandEvent& event );

    void OnRadiobutton2Selected( wxCommandEvent& event );

    void OnButton1Click( wxCommandEvent& event );

    void OnTextctrl1Updated( wxCommandEvent& event );

    void OnMaxLabelLenCheckClick( wxCommandEvent& event );

    void OnRandomizeButtonClick( wxCommandEvent& event );

    void OnFontFaceChoiceSelected( wxCommandEvent& event );

    void OnFontSizeChoiceSelected( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CwxPhyloSettingsDlg event handler declarations

////@begin CwxPhyloSettingsDlg member function declarations

    GLdouble GetLineWidth() const { return m_LineWidth ; }
    void SetLineWidth(GLdouble value) { m_LineWidth = value ; }

    GLdouble GetMarginTop() const { return m_MarginTop ; }
    void SetMarginTop(GLdouble value) { m_MarginTop = value ; }

    GLdouble GetMarginBottom() const { return m_MarginBottom ; }
    void SetMarginBottom(GLdouble value) { m_MarginBottom = value ; }

    GLdouble GetMarginLeft() const { return m_MarginLeft ; }
    void SetMarginLeft(GLdouble value) { m_MarginLeft = value ; }

    GLdouble GetMarginRight() const { return m_MarginRight ; }
    void SetMarginRight(GLdouble value) { m_MarginRight = value ; }

    GLdouble GetNodeSize() const { return m_NodeSize ; }
    void SetNodeSize(GLdouble value) { m_NodeSize = value ; }

    GLdouble GetLeafNodeSize() const { return m_LeafNodeSize ; }
    void SetLeafNodeSize(GLdouble value) { m_LeafNodeSize = value ; }

    GLdouble GetMaxNodeSize() const { return m_MaxNodeSize ; }
    void SetMaxNodeSize(GLdouble value) { m_MaxNodeSize = value ; }

    bool GetResizableCollapsed() const { return m_ResizableCollapsed ; }
    void SetResizableCollapsed(bool value) { m_ResizableCollapsed = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxPhyloSettingsDlg member function declarations

    static bool ShowToolTips();


////@begin CwxPhyloSettingsDlg member variables
    wxPanel* m_ColorPanel;
    wxRadioBox* m_Coloration;
    wxColourPickerCtrl* m_NodeColorDefault;
    wxColourPickerCtrl* m_EdgeColorDefault;
    wxColourPickerCtrl* m_LabelColorDefault;
    wxColourPickerCtrl* m_NodeColorSelection;
    wxColourPickerCtrl* m_EdgeColorSelection;
    wxColourPickerCtrl* m_LabelColorSelection;
    wxColourPickerCtrl* m_NodeColorTrace;
    wxColourPickerCtrl* m_EdgeColorTrace;
    wxColourPickerCtrl* m_LabelColorTrace;
    wxColourPickerCtrl* m_NodeColorCommon;
    wxColourPickerCtrl* m_EdgeColorCommon;
    wxColourPickerCtrl* m_LabelColorCommon;
    wxSlider* m_SelectionAlpha;
    wxStaticText* m_SelectionAlphaText;
    wxRadioButton* m_rbSimple;
    wxChoice* m_Feature1;
    wxRadioButton* m_rbBuilder;
    wxChoice* m_Feature2;
    wxButton* m_btnInsert;
    wxTextCtrl* m_FormatString;
    wxCheckBox* m_SetMaxLabelLenCheck;
    wxTextCtrl* m_MaxLabelLenValue;
    wxTextCtrl* m_SampleLabel;
    wxChoice* m_FontFace;
    wxChoice* m_FontSize;
    wxHyperlinkCtrl* m_HelpHyperLink;
    GLdouble m_LineWidth;
    GLdouble m_MarginTop;
    GLdouble m_MarginBottom;
    GLdouble m_MarginLeft;
    GLdouble m_MarginRight;
    GLdouble m_NodeSize;
    GLdouble m_LeafNodeSize;
    GLdouble m_MaxNodeSize;
protected:
    CRgbaColor m_CommonEdge;
    CRgbaColor m_CommonLabel;
    wxString m_TextSize;
    CRgbaColor m_TraceEdge;
    CRgbaColor m_CommonNode;
    CRgbaColor m_TraceLabel;
    CRgbaColor m_TraceNode;
    wxArrayInt m_VisColumns;
    CRgbaColor m_DefEdge;
    CRgbaColor m_DefLabel;
    CRgbaColor m_DefNode;
    CRgbaColor m_SelEdge;
    CRgbaColor m_SelLabel;
    CRgbaColor m_SelNode;
    wxString m_SeqFace;
    wxString m_SeqSize;
    wxString m_TextFace;
    bool m_ResizableCollapsed;
////@end CwxPhyloSettingsDlg member variables

protected:
    CPhyloTreeDataSource* m_pDS;
    CPhyloTreeScheme*      m_pSL;
    CPhyloTree::TTreeIdx  m_SampleNode;

    // Font face/size combinations tht do not work well (these combinations are
    // not shown to the user).
    std::map<string, vector<string> > m_InvalidFaceSizes;

};

END_NCBI_SCOPE

#endif
    // _WX_PHYLO_SETTINGS_DLG_H_
