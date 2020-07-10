/*  $Id: molinfo_panel.hpp 38676 2017-06-08 14:41:06Z filippov $
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
#ifndef _MOLINFO_PANEL_H_
#define _MOLINFO_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/Bioseq.hpp>
#include <gui/widgets/edit/utilities.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMOLINFOPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMOLINFOPANEL_TITLE _("MolInfoPanel")
#define SYMBOL_CMOLINFOPANEL_IDNAME ID_CMOLINFOPANEL
#define SYMBOL_CMOLINFOPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMOLINFOPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMolInfoPanel class declaration
 */

class CMolInfoPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CMolInfoPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMolInfoPanel();
    CMolInfoPanel( wxWindow* parent, CRef<objects::CMolInfo> molinfo, CRef<objects::CBioseq> bioseq, wxWindowID id = SYMBOL_CMOLINFOPANEL_IDNAME, const wxString& caption = SYMBOL_CMOLINFOPANEL_TITLE, const wxPoint& pos = SYMBOL_CMOLINFOPANEL_POSITION, const wxSize& size = SYMBOL_CMOLINFOPANEL_SIZE, long style = SYMBOL_CMOLINFOPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMOLINFOPANEL_IDNAME, const wxString& caption = SYMBOL_CMOLINFOPANEL_TITLE, const wxPoint& pos = SYMBOL_CMOLINFOPANEL_POSITION, const wxSize& size = SYMBOL_CMOLINFOPANEL_SIZE, long style = SYMBOL_CMOLINFOPANEL_STYLE );

    /// Destructor
    ~CMolInfoPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CRef<objects::CMolInfo> GetMolinfo() const;
    void SetBioseqValues(CRef<objects::CBioseq> bioseq);
    void ChangeBioseq(CConstRef<objects::CBioseq> bioseq);

////@begin CMolInfoPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE8
    void OnChoice8Selected( wxCommandEvent& event );

////@end CMolInfoPanel event handler declarations

////@begin CMolInfoPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMolInfoPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMolInfoPanel member variables
    wxChoice* m_MoleculeCtrl;
    wxChoice* m_CompletednessCtrl;
    wxChoice* m_TechniqueCtrl;
    wxTextCtrl* m_TechExpCtrl;
    wxChoice* m_TopologyCtrl;
    wxChoice* m_StrandednessCtrl;
    /// Control identifiers
    enum {
        ID_CMOLINFOPANEL = 10066,
        ID_CHOICE6 = 10067,
        ID_CHOICE7 = 10068,
        ID_CHOICE8 = 10069,
        ID_TEXTCTRL24 = 10072,
        ID_CHOICE9 = 10070,
        ID_CHOICE10 = 10071
    };
////@end CMolInfoPanel member variables

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

private:
    CRef<objects::CMolInfo> m_Molinfo;
    CRef<objects::CBioseq> m_Bioseq;
    int m_TechOther;
    void x_SetMolecule();
    void x_GetMolecule();
    void x_SetCompletedness();
    void x_GetCompletedness();
    void x_SetTechnique();
    void x_GetTechnique();
    void x_SetTopology();
    void x_GetTopology();
    void x_SetStrandedness();
    void x_GetStrandedness();

};

END_NCBI_SCOPE

#endif
    // _MOLINFO_PANEL_H_
