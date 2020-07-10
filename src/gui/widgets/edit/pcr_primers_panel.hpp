#ifndef GUI_WIDGETS_EDIT___PCR_PRIMERS_PANEL__HPP
#define GUI_WIDGETS_EDIT___PCR_PRIMERS_PANEL__HPP

/*  $Id: pcr_primers_panel.hpp 43014 2019-05-07 14:51:18Z katargir $
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
 * Authors:  Igor Filippov, adapted from Vasuki Palanigobu
 */

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/hyperlink.h>

#include <objects/seqfeat/BioSource.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE



#define ID_PCRPRIMERENTRY 10034
#define ID_PCRPRIMER_NAME 10035
#define ID_PCRPRIMER_SEQ 10036
#define ID_PCRPRIMER_DIR 10037
#define ID_PCRPRIMER_SET 10038
#define SYMBOL_PCRPRIMERENTRY_STYLE wxTAB_TRAVERSAL
#define SYMBOL_PCRPRIMERENTRY_TITLE _("PCR Primer Entry")
#define SYMBOL_PCRPRIMERENTRY_IDNAME ID_PCRPRIMERENTRY
#define SYMBOL_PCRPRIMERENTRY_SIZE wxDefaultSize
#define SYMBOL_PCRPRIMERENTRY_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSrcModPanel class declaration
 */

class CPCRPrimerEntry: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CPCRPRimerEntry )
    DECLARE_EVENT_TABLE()

public:
    struct SPrimerData {
        wxString name;
        wxString seq;
        bool forward_dir;
        int set;
    };
    
    /// Constructors
    CPCRPrimerEntry();
    CPCRPrimerEntry( wxWindow* parent, SPrimerData primer_data, wxWindowID id = SYMBOL_PCRPRIMERENTRY_IDNAME, const wxPoint& pos = SYMBOL_PCRPRIMERENTRY_POSITION, const wxSize& size = SYMBOL_PCRPRIMERENTRY_SIZE, 
                     long style = SYMBOL_PCRPRIMERENTRY_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_PCRPRIMERENTRY_IDNAME, const wxPoint& pos = SYMBOL_PCRPRIMERENTRY_POSITION, const wxSize& size = SYMBOL_PCRPRIMERENTRY_SIZE, long style = SYMBOL_PCRPRIMERENTRY_STYLE );

    /// Destructor
    ~CPCRPrimerEntry();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    SPrimerData GetData();
    void SetData(SPrimerData entry);

    void OnSeqUpdated( wxCommandEvent& event );

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxChoice* m_PrimerDir;
    wxTextCtrl* m_PrimerName;
    wxTextCtrl* m_PrimerSeq;
    wxTextCtrl* m_PrimerSet;
private:
    SPrimerData	m_PrimerData;
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_PCRPRIMERSPANEL 10032
#define ID_PCRSCROLLEDWINDOW1 10033
#define ID_ADD_PCR_PRIMER 10034
#define SYMBOL_PCRPRIMERSPANEL_STYLE  wxWS_EX_VALIDATE_RECURSIVELY
#define SYMBOL_PCRPRIMERSPANEL_TITLE _("PCR Primers")
#define SYMBOL_PCRPRIMERSPANEL_IDNAME ID_PCRPRIMERSPANEL
#define SYMBOL_PCRPRIMERSPANEL_SIZE wxDefaultSize
#define SYMBOL_PCRPRIMERSPANEL_POSITION wxDefaultPosition
////@end control identifiers


class CPCRPrimersPanel : public wxScrolledWindow
{    
    DECLARE_DYNAMIC_CLASS( CPCRPrimersPanel )
    DECLARE_EVENT_TABLE()
    
public:
    
    /// Constructors
    CPCRPrimersPanel();
    CPCRPrimersPanel( wxWindow* parent, objects::CBioSource* source, wxWindowID id = SYMBOL_PCRPRIMERSPANEL_IDNAME, const wxPoint& pos = SYMBOL_PCRPRIMERSPANEL_POSITION, const wxSize& size = SYMBOL_PCRPRIMERSPANEL_SIZE, 
                      long style = SYMBOL_PCRPRIMERSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_PCRPRIMERSPANEL_IDNAME, const wxPoint& pos = SYMBOL_PCRPRIMERSPANEL_POSITION, const wxSize& size = SYMBOL_PCRPRIMERSPANEL_SIZE, long style = SYMBOL_PCRPRIMERSPANEL_STYLE );

    /// Destructor
    ~CPCRPrimersPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxFlexGridSizer* m_Sizer;

    void OnDelete (wxHyperlinkEvent& event);
    void UpdateBioSourcePrimers(objects::CBioSource& source);
    void x_AddEmptyRow(int nset = 0, bool fwd_dir = true);
    void x_AdjustScrollWindow();

private:
    objects::CBioSource*	    m_Source;
    vector<CPCRPrimerEntry::SPrimerData>		m_Data;
    int							m_TotalHeight;
    int							m_TotalWidth;
    int							m_ScrollRate;

    void x_FillList();
    void x_AddAllRows ();
    void x_AddRow(const CPCRPrimerEntry::SPrimerData &data);
    int  x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
};



END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___PCR_PRIMERS_PANEL__HPP

