#ifndef PKG_ALIGNMENT___SPLIGN_PANEL__HPP
#define PKG_ALIGNMENT___SPLIGN_PANEL__HPP

/*  $Id: splign_panel.hpp 35070 2016-03-18 20:11:06Z asztalos $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbiobj.hpp>

//#include <gui/objutils/objects.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/splign_params.hpp>
#include <gui/widgets/object_list/object_list_widget_sel.hpp>

#include "wx/valgen.h"

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSPLIGNPANEL_STYLE wxCLIP_CHILDREN|wxTAB_TRAVERSAL
#define SYMBOL_CSPLIGNPANEL_TITLE _("SPLIGN Tool Panel")
#define SYMBOL_CSPLIGNPANEL_IDNAME ID_CSPLIGNPANEL
#define SYMBOL_CSPLIGNPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSPLIGNPANEL_POSITION wxDefaultPosition
////@end control identifiers

class wxChoice;

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CMolTypeValidator : public IObjectListAccValidator
{
public:
    enum MolType
    {
        kGenomic,
        kCDNA,
        kProtein,
        kNucleotide
    };

    CMolTypeValidator(MolType molType) : m_MolType(molType) {}

    virtual CConstRef<CObject> Validate(const objects::CSeq_id& id,
                                        objects::CScope& scope, wxString& errMessage);
private:
    MolType m_MolType;
};

/*!
 * CSplignPanel class declaration
 */

class CSplignPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CSplignPanel )
    DECLARE_EVENT_TABLE()

public:
    CSplignPanel();
    CSplignPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSPLIGNPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSPLIGNPANEL_POSITION, const wxSize& size = SYMBOL_CSPLIGNPANEL_SIZE, long style = SYMBOL_CSPLIGNPANEL_STYLE, bool visibale = true );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSPLIGNPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSPLIGNPANEL_POSITION, const wxSize& size = SYMBOL_CSPLIGNPANEL_SIZE, long style = SYMBOL_CSPLIGNPANEL_STYLE, bool visibale = true );

    ~CSplignPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataFromWindow();

    /// @name IRegSettings interface implementation
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}
        
////@begin CSplignPanel event handler declarations

////@end CSplignPanel event handler declarations

////@begin CSplignPanel member function declarations

    SConstScopedObject GetGenomicSeq() const { return m_GenomicSeq ; }
    void SetGenomicSeq(SConstScopedObject value) { m_GenomicSeq = value ; }

    TConstScopedObjects GetCDNASeqs() const { return m_cDNASeqs ; }
    void SetCDNASeqs(TConstScopedObjects value) { m_cDNASeqs = value ; }

    CSplignParams& GetData() { return m_data; }
    const CSplignParams& GetData() const { return m_data; }
    void SetData(const CSplignParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CSplignPanel member function declarations

    void SetObjects(map<string, TConstScopedObjects>* genomic,
                    map<string, TConstScopedObjects>* na);

    static bool ShowToolTips();

////@begin CSplignPanel member variables
    CObjectListWidgetSel* m_GenomicList;
    CObjectListWidgetSel* m_cDNAList;
private:
    SConstScopedObject m_GenomicSeq;
    TConstScopedObjects m_cDNASeqs;
    CSplignParams m_data;
    enum {
        ID_CSPLIGNPANEL = 10028,
        ID_PANEL1 = 10029,
        ID_PANEL2 = 10030,
        ID_CHOICE3 = 10040,
        ID_CHOICE4 = 10076,
        ID_CHECKBOX3 = 10034,
        ID_CHECKBOX4 = 10035,
        ID_TEXTCTRL7 = 10037,
        ID_TEXTCTRL6 = 10036,
        ID_TEXTCTRL8 = 10038,
        ID_TEXTCTRL9 = 10039,
        ID_TEXTCTRL13 = 10043
    };
////@end CSplignPanel member variables

private:
    string m_RegPath;

    CMolTypeValidator m_GenomicListAccValidator;
    CMolTypeValidator m_CDNAListAccValidator;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___SPLIGN_PANEL__HPP
