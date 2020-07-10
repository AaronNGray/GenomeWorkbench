#ifndef PKG_ALIGNMENT___PROSPLIGN_PANEL__HPP
#define PKG_ALIGNMENT___PROSPLIGN_PANEL__HPP

/*  $Id: prosplign_panel.hpp 35635 2016-06-03 19:05:47Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/prosplign_params.hpp>
#include <gui/packages/pkg_alignment/splign_panel.hpp>

BEGIN_NCBI_SCOPE

////@begin forward declarations
class CObjectListWidgetSel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPROSPLIGNPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROSPLIGNPANEL_TITLE _("ProSplign Tool Panel")
#define SYMBOL_CPROSPLIGNPANEL_IDNAME ID_CPROSPLIGNPANEL
#define SYMBOL_CPROSPLIGNPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPROSPLIGNPANEL_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * CProSplignPanel class declaration
 */

class CProSplignPanel : public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CProSplignPanel )
    DECLARE_EVENT_TABLE()

public:
    CProSplignPanel();
    CProSplignPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPROSPLIGNPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPROSPLIGNPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPROSPLIGNPANEL_SIZE, 
        long style = SYMBOL_CPROSPLIGNPANEL_STYLE,
        bool visible = true);

    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPROSPLIGNPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPROSPLIGNPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPROSPLIGNPANEL_SIZE, 
        long style = SYMBOL_CPROSPLIGNPANEL_STYLE,
        bool visible = true);

    ~CProSplignPanel();

    void Init();

    void CreateControls();

    /// @name IRegSettings interface implementation
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CProSplignPanel event handler declarations
    void OnRefineAlignmentClick(wxCommandEvent& event);

////@end CProSplignPanel event handler declarations

////@begin CProSplignPanel member function declarations
    bool TransferDataFromWindow();
    bool TransferDataToWindow();

    CProSplignParams& GetNCData() { return m_Data; }
    const CProSplignParams& GetData() const { return m_Data; }
    void SetData(const CProSplignParams& data) { m_Data = data; }

    SConstScopedObject GetProteinSeq() const { return m_ProteinSeq; }
    TConstScopedObjects GetNucleotideSeqs() const { return m_NucleotideSeqs; }

    void SetObjects(map<string, TConstScopedObjects>* protein,
        map<string, TConstScopedObjects>* nucleotide);

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
    ////@end CProSplignPanel member function declarations

    static bool ShowToolTips();

private:
    void x_ListGeneticCodes();
    void x_EnableRefinementOptions(bool enable);

    CObjectListWidgetSel* m_ProteinList;
    CObjectListWidgetSel* m_NucleotideList;

    SConstScopedObject m_ProteinSeq;
    TConstScopedObjects m_NucleotideSeqs;
    CProSplignParams m_Data;

    enum {
        ID_CPROSPLIGNPANEL = 10090,
        ID_PROSPLIGN_PANEL1 = 10091,
        ID_PROSPLIGN_PANEL2 = 10092,
        ID_PROSPLIGN_NOTEBOOK = 10093,
        ID_PROSPLIGN_PANEL3 = 10094,
        ID_PROSPLIGN_CHOICE1 = 10095,
        ID_PROSPLIGN_CHCKBX1 = 10096,
        ID_PROSPLIGN_TXTCTRL1 = 10097,
        ID_PROSPLIGN_TXTCTRL2 = 10098,
        ID_PROSPLIGN_TXTCTRL3 = 10099,
        ID_PROSPLIGN_CHOICE2 = 10100,
        ID_PROSPLIGN_PANEL4 = 10101,
        ID_PROSPLIGN_CHCKBX2 = 10102,
        ID_PROSPLIGN_CHCKBX3 = 10103,
        ID_PROSPLIGN_CHCKBX4 = 10104,
        ID_PROSPLIGN_TXTCTRL4 = 10105,
        ID_PROSPLIGN_TXTCTRL5 = 10106,
        ID_PROSPLIGN_TXTCTRL6 = 10107,
        ID_PROSPLIGN_TXTCTRL7 = 10108,
        ID_PROSPLIGN_TXTCTRL8 = 10109,
        ID_PROSPLIGN_TXTCTRL9 = 10110
    };

    string m_RegPath;

    CMolTypeValidator m_ProteinListAccValidator;
    CMolTypeValidator m_NucleotideListAccValidator;

    wxString m_FrameshiftCostStr;
    wxString m_GapOpenCostStr;
    wxString m_GapExtendCostStr;

    long m_TotalPosL;
    long m_FlankPosL;
    long m_MinGoodLenL;
    long m_MinExonIdentL;
    long m_MinExonPosL;
    long m_MinFlankExonLenL;
};

END_NCBI_SCOPE

#endif
    // PKG_ALIGNMENT___PROSPLIGN_PANEL__HPP

