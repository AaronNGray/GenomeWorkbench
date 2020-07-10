/*  $Id: create_gene_model_params_panel.hpp 42821 2019-04-18 19:32:56Z joukovv $
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

#ifndef _CREATE_GENE_MODEL_PARAMS_PANEL_H_
#define _CREATE_GENE_MODEL_PARAMS_PANEL_H_


/*!
 * Includes
 */

#include <gui/objutils/reg_settings.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/core/algo_tool_manager_base.hpp>

////@begin includes
#include "wx/listctrl.h"
////@end includes

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class CObjectListWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

#define GENEMODELPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define GENEMODELPARAMSPANEL_TITLE _("Create Gene Model")
#define GENEMODELPARAMSPANEL_IDNAME ID_GENEMODELPARAMSPANEL
#define GENEMODELPARAMSPANEL_SIZE wxSize(400, 300)
#define GENEMODELPARAMSPANEL_POSITION wxDefaultPosition

/*!
 * CCreateGeneModelParamsPanel class declaration
 */

class SCreateGeneModelParams : public IRegSettings
{
public:
    SCreateGeneModelParams();

    void Init();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath( const string& reg_path );
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

public:
    string m_RegPath;

    TConstScopedObjects m_Alignments;

    bool m_CreateGene;
    bool m_CreateMrna;
    bool m_CreateCds;
    bool m_PropagateNcrnaFeats;

    bool m_GroupByGeneId;
    bool m_TranscribeMrna;
    bool m_TranslateCds;

    bool m_MergeResults;
    bool m_PropagateLocalIds;
};

class CCreateGeneModelParamsPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CCreateGeneModelParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    CCreateGeneModelParamsPanel();
    CCreateGeneModelParamsPanel( wxWindow* parent, wxWindowID id = GENEMODELPARAMSPANEL_IDNAME, const wxPoint& pos = GENEMODELPARAMSPANEL_POSITION, const wxSize& size = GENEMODELPARAMSPANEL_SIZE, long style = GENEMODELPARAMSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = GENEMODELPARAMSPANEL_IDNAME, const wxPoint& pos = GENEMODELPARAMSPANEL_POSITION, const wxSize& size = GENEMODELPARAMSPANEL_SIZE, long style = GENEMODELPARAMSPANEL_STYLE );

    ~CCreateGeneModelParamsPanel();

    void Init();
    void CreateControls();

    void SetParams(SCreateGeneModelParams* params, TConstScopedObjects* objects);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath( const string& path );
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CCreateGeneModelParamsPanel event handler declarations

    void OnCreateGeneClick( wxCommandEvent& event );

    void OnCreateMrnaClick( wxCommandEvent& event );

    void OnCreateCdsClick( wxCommandEvent& event );

////@end CCreateGeneModelParamsPanel event handler declarations

////@begin CCreateGeneModelParamsPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CCreateGeneModelParamsPanel member function declarations

private:
////@begin CCreateGeneModelParamsPanel member variables
    CObjectListWidget* m_ObjectList;
    wxCheckBox* m_CreateGene;
    wxCheckBox* m_PropagateNcrnaFeats;
    wxCheckBox* m_CreateMrna;
    wxCheckBox* m_TranscribeMrna;
    wxCheckBox* m_CreateCds;
    wxCheckBox* m_TranslateCds;
    wxCheckBox* m_GroupByGeneId;
    wxCheckBox* m_MergeToGenome;
    wxCheckBox* m_PropagateLocalIds;
private:
    SCreateGeneModelParams* m_Params;
    enum {
        ID_GENEMODELPARAMSPANEL = 10031,
        ID_OBJLISTCTRL = 10014,
        ID_CHECK_GENE = 10032,
        ID_PROPAGATE_NCRNA = 10046,
        ID_CHECK_MRNA = 10041,
        ID_CHECK_TRANSCRIPT = 10000,
        ID_CHECK_CDS = 10042,
        ID_CHECK_TRANSLATE = 10044,
        ID_CHECK_GROUP = 10045,
        ID_CHECK_MERGE = 10059,
        ID_PROPAGATE_LOCAL_IDS = 10060
    };
////@end CCreateGeneModelParamsPanel member variables

private:
    TConstScopedObjects*    m_InputObjects;

    bool    m_InputListDirty;

    string  m_RegPath;
};

END_NCBI_SCOPE

#endif
    // _CREATE_GENE_MODEL_PARAMS_PANEL_H_
