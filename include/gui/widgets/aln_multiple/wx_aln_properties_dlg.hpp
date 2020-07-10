#ifndef GUI_WIDGETS_ALN_MULTIPLE___WX_PROPERTIES_DLG__HPP
#define GUI_WIDGETS_ALN_MULTIPLE___WX_PROPERTIES_DLG__HPP

/*  $Id: wx_aln_properties_dlg.hpp 39636 2017-10-20 19:32:22Z falkrb $
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

#include <gui/widgets/wx/dialog.hpp>

#include <gui/widgets/aln_multiple/align_row.hpp>


////@begin includes
#include "wx/valgen.h"
#include "wx/statline.h"
#include "wx/clrpicker.h"
////@end includes

////@begin forward declarations
////@end forward declarations

class wxCheckListBox;
class wxComboBox;
class wxCheckBox;

////@begin control identifiers
#define SYMBOL_CALNPROPERTIESDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CALNPROPERTIESDLG_TITLE _("Properties")
#define SYMBOL_CALNPROPERTIESDLG_IDNAME ID_CWXALNPROPERTIESDLG
#define SYMBOL_CALNPROPERTIESDLG_SIZE wxSize(400, 300)
#define SYMBOL_CALNPROPERTIESDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CwxAlnPropertiesDlg
class CAlnPropertiesDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CAlnPropertiesDlg )
    DECLARE_EVENT_TABLE()

public:
    CAlnPropertiesDlg();
    CAlnPropertiesDlg( wxWindow* parent, wxWindowID id = SYMBOL_CALNPROPERTIESDLG_IDNAME, const wxString& caption = SYMBOL_CALNPROPERTIESDLG_TITLE, const wxPoint& pos = SYMBOL_CALNPROPERTIESDLG_POSITION, const wxSize& size = SYMBOL_CALNPROPERTIESDLG_SIZE, long style = SYMBOL_CALNPROPERTIESDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALNPROPERTIESDLG_IDNAME, const wxString& caption = SYMBOL_CALNPROPERTIESDLG_TITLE, const wxPoint& pos = SYMBOL_CALNPROPERTIESDLG_POSITION, const wxSize& size = SYMBOL_CALNPROPERTIESDLG_SIZE, long style = SYMBOL_CALNPROPERTIESDLG_STYLE );

    ~CAlnPropertiesDlg();

    void Init();

    void CreateControls();

    void SetParams(CWidgetDisplayStyle& style);

////@begin CAlnPropertiesDlg event handler declarations

    void OnOkClick( wxCommandEvent& event );

////@end CAlnPropertiesDlg event handler declarations

////@begin CAlnPropertiesDlg member function declarations

    CRgbaColor GetSeqColor() const { return m_SeqColor ; }
    void SetSeqColor(CRgbaColor value) { m_SeqColor = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAlnPropertiesDlg member function declarations

    static bool ShowToolTips();

////@begin CAlnPropertiesDlg member variables
    wxCheckListBox* m_VisColumnsList;
    wxComboBox* m_TextFaceCombo;
    wxComboBox* m_TextSizeCombo;
    wxComboBox* m_SeqFaceCombo;
    wxComboBox* m_SeqSizeCombo;
    wxCheckBox* m_ShowIdenticalBases;
    wxCheckBox* m_ShowConsensus;
protected:
    wxString m_TextFace;
    wxString m_TextSize;
    wxString m_SeqFace;
    wxString m_SeqSize;
    wxArrayInt m_VisColumns;
    CRgbaColor m_TextColor;
    CRgbaColor m_BackColor;
    CRgbaColor m_SelTextColor;
    CRgbaColor m_SelBackColor;
    CRgbaColor m_FrameColor;
    CRgbaColor m_FocusedColor;
    CRgbaColor m_SegmentColor;
    CRgbaColor m_SeqColor;
    enum {
        ID_CWXALNPROPERTIESDLG = 10000,
        ID_PANEL = 10013,
        ID_VIS_COLUMNS = 10011,
        ID_TEXT_FACE_COMBO = 10001,
        ID_TEXT_SIZE_COMBO = 10022,
        ID_SEQ_FACE_COMBO = 10024,
        ID_FACE_SIZE_COMBO = 10023,
        ID_IDENTICAL_BASES_CHECKBOX = 10019,
        ID_CONSENSUS_CHECKBOX = 10029,
        ID_TEXT_COLOR = 10012,
        ID_BACK_COLOR = 10014,
        ID_SEQ_COLOR = 10015,
        ID_FRAME_COLOR = 10016,
        ID_SEG_COLOR = 10025,
        ID_SEL_TEXT_COLOR = 10026,
        ID_SEL_BACK_COLOR = 10027,
        ID_FOCUSED_BACK_COLOR = 10028
    };
////@end CAlnPropertiesDlg member variables
    CWidgetDisplayStyle* m_Style;
    vector<string>  m_AllColumns;
};


END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_ALN_MULTIPLE___WX_PROPERTIES_DLG__HPP

